/*
    mtr  --  a network diagnostic tool
    Copyright (C) 2016  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "probe.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "command.h"
#include "platform.h"
#include "protocols.h"
#include "timeval.h"

#define IP_TEXT_LENGTH 64

/*  Convert the destination address from text to sockaddr  */
int decode_address_string(
    int ip_version,
    const char *address_string,
    struct sockaddr_storage *address)
{
    struct in_addr addr4;
    struct in6_addr addr6;
    struct sockaddr_in *sockaddr4;
    struct sockaddr_in6 *sockaddr6;

    if (address == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (ip_version == 6) {
        sockaddr6 = (struct sockaddr_in6 *) address;

        if (inet_pton(AF_INET6, address_string, &addr6) != 1) {
            errno = EINVAL;
            return -1;
        }

        sockaddr6->sin6_family = AF_INET6;
        sockaddr6->sin6_port = 0;
        sockaddr6->sin6_flowinfo = 0;
        sockaddr6->sin6_addr = addr6;
        sockaddr6->sin6_scope_id = 0;
    } else if (ip_version == 4) {
        sockaddr4 = (struct sockaddr_in *) address;

        if (inet_pton(AF_INET, address_string, &addr4) != 1) {
            errno = EINVAL;
            return -1;
        }

        sockaddr4->sin_family = AF_INET;
        sockaddr4->sin_port = 0;
        sockaddr4->sin_addr = addr4;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/*
    Resolve the probe parameters into a remote and local address
    for the probe.
*/
int resolve_probe_addresses(
    const struct probe_param_t *param,
    struct sockaddr_storage *dest_sockaddr,
    struct sockaddr_storage *src_sockaddr)
{
    if (decode_address_string
        (param->ip_version, param->remote_address, dest_sockaddr)) {
        return -1;
    }

    if (param->local_address) {
        if (decode_address_string
            (param->ip_version, param->local_address, src_sockaddr)) {
            return -1;
        }
    } else {
        if (find_source_addr(src_sockaddr, dest_sockaddr)) {
            return -1;
        }
    }

    return 0;
}

/*  Allocate a structure for tracking a new probe  */
struct probe_t *alloc_probe(
    struct net_state_t *net_state,
    int token)
{
    struct probe_t *probe;

    if (net_state->outstanding_probe_count >= MAX_PROBES) {
        return NULL;
    }

    probe = malloc(sizeof(struct probe_t));
    if (probe == NULL) {
        return NULL;
    }

    memset(probe, 0, sizeof(struct probe_t));
    probe->token = token;

    platform_alloc_probe(net_state, probe);

    net_state->outstanding_probe_count++;
    LIST_INSERT_HEAD(&net_state->outstanding_probes, probe,
                     probe_list_entry);

    return probe;
}

/*  Mark a probe tracking structure as unused  */
void free_probe(
    struct net_state_t *net_state,
    struct probe_t *probe)
{
    LIST_REMOVE(probe, probe_list_entry);
    net_state->outstanding_probe_count--;

    platform_free_probe(probe);

    free(probe);
}

/*
    Find an existing probe structure by ICMP id and sequence number.
    Returns NULL if non is found.
*/
struct probe_t *find_probe(
    struct net_state_t *net_state,
    int protocol,
    int id,
    int sequence)
{
    struct probe_t *probe;

    /*
       ICMP has room for an id to check against our process, but
       UDP doesn't.
     */
    if (protocol == IPPROTO_ICMP) {
        /*
           If the ICMP id doesn't match our process ID, it wasn't a
           probe generated by this process, so ignore it.
         */
        if (id != htons(getpid())) {
            return NULL;
        }
    }

    LIST_FOREACH(probe, &net_state->outstanding_probes, probe_list_entry) {
        if (htons(probe->sequence) == sequence) {
            return probe;
        }
    }

    return NULL;
}

/*
    Format a list of MPLS labels into a string appropriate for including
    as an argument to a probe reply.
*/
static
void format_mpls_string(
    char *str,
    int buffer_size,
    int mpls_count,
    const struct mpls_label_t *mpls_list)
{
    int i;
    char *append_pos = str;
    const struct mpls_label_t *mpls;

    /*  Start with an empty string  */
    str[0] = 0;

    for (i = 0; i < mpls_count; i++) {
        mpls = &mpls_list[i];

        if (i > 0) {
            strncat(append_pos, ",", buffer_size - 1);

            buffer_size -= strlen(append_pos);
            append_pos += strlen(append_pos);
        }

        snprintf(append_pos, buffer_size, "%d,%d,%d,%d",
                 mpls->label, mpls->experimental_use,
                 mpls->bottom_of_stack, mpls->ttl);

        buffer_size -= strlen(append_pos);
        append_pos += strlen(append_pos);
    }
}

/*
    After a probe reply has arrived, respond to the command request which
    sent the probe.
*/
void respond_to_probe(
    struct net_state_t *net_state,
    struct probe_t *probe,
    int icmp_type,
    const struct sockaddr_storage *remote_addr,
    unsigned int round_trip_us,
    int mpls_count,
    const struct mpls_label_t *mpls)
{
    char ip_text[IP_TEXT_LENGTH];
    char response[COMMAND_BUFFER_SIZE];
    char mpls_str[COMMAND_BUFFER_SIZE];
    int remaining_size;
    const char *result;
    const char *ip_argument;
    struct sockaddr_in *sockaddr4;
    struct sockaddr_in6 *sockaddr6;
    void *addr;

    if (icmp_type == ICMP_TIME_EXCEEDED) {
        result = "ttl-expired";
    } else {
        assert(icmp_type == ICMP_ECHOREPLY);
        result = "reply";
    }

    if (remote_addr->ss_family == AF_INET6) {
        ip_argument = "ip-6";
        sockaddr6 = (struct sockaddr_in6 *) remote_addr;
        addr = &sockaddr6->sin6_addr;
    } else {
        ip_argument = "ip-4";
        sockaddr4 = (struct sockaddr_in *) remote_addr;
        addr = &sockaddr4->sin_addr;
    }

    if (inet_ntop(remote_addr->ss_family, addr, ip_text, IP_TEXT_LENGTH) ==
        NULL) {

        perror("inet_ntop failure");
        exit(EXIT_FAILURE);
    }

    snprintf(response, COMMAND_BUFFER_SIZE,
             "%d %s %s %s round-trip-time %d",
             probe->token, result, ip_argument, ip_text, round_trip_us);

    if (mpls_count) {
        format_mpls_string(mpls_str, COMMAND_BUFFER_SIZE, mpls_count,
                           mpls);

        remaining_size = COMMAND_BUFFER_SIZE - strlen(response) - 1;
        strncat(response, " mpls ", remaining_size);

        remaining_size = COMMAND_BUFFER_SIZE - strlen(response) - 1;
        strncat(response, mpls_str, remaining_size);
    }

    puts(response);
    free_probe(net_state, probe);
}

/*
    Find the source address for transmitting to a particular destination
    address.  Remember that hosts can have multiple addresses, for example
    a unique address for each network interface.  So we will bind a UDP
    socket to our destination and check the socket address after binding
    to get the source for that destination, which will allow the kernel
    to do the routing table work for us.

    (connecting UDP sockets, unlike TCP sockets, doesn't transmit any packets.
    It's just an association.)
*/
int find_source_addr(
    struct sockaddr_storage *srcaddr,
    const struct sockaddr_storage *destaddr)
{
    int sock;
    int len;
    struct sockaddr_in *destaddr4;
    struct sockaddr_in6 *destaddr6;
    struct sockaddr_storage dest_with_port;
    struct sockaddr_in *srcaddr4;
    struct sockaddr_in6 *srcaddr6;

    dest_with_port = *destaddr;

    /*
       MacOS requires a non-zero sin_port when used as an
       address for a UDP connect.  If we provide a zero port,
       the connect will fail.  We aren't actually sending
       anything to the port.
     */
    if (destaddr->ss_family == AF_INET6) {
        destaddr6 = (struct sockaddr_in6 *) &dest_with_port;
        destaddr6->sin6_port = htons(1);

        len = sizeof(struct sockaddr_in6);
    } else {
        destaddr4 = (struct sockaddr_in *) &dest_with_port;
        destaddr4->sin_port = htons(1);

        len = sizeof(struct sockaddr_in);
    }

    sock = socket(destaddr->ss_family, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &dest_with_port, len)) {
        close(sock);
        return -1;
    }

    if (getsockname(sock, (struct sockaddr *) srcaddr, &len)) {
        close(sock);
        return -1;
    }

    close(sock);

    /*
       Zero the port, as we may later use this address to finding, and
       we don't want to use the port from the socket we just created.
     */
    if (destaddr->ss_family == AF_INET6) {
        srcaddr6 = (struct sockaddr_in6 *) srcaddr;

        srcaddr6->sin6_port = 0;
    } else {
        srcaddr4 = (struct sockaddr_in *) srcaddr;

        srcaddr4->sin_port = 0;
    }

    return 0;
}
