/* HC5861 */

#define BOARD_PID		"HC5861"
#define BOARD_NAME		"HC5861"
#define BOARD_DESC		"HiWiFi HC5861 Wireless Router"
#define BOARD_VENDOR_NAME	"HiWiFi"
#define BOARD_VENDOR_URL	"http://www.hiwifi.com/"
#define BOARD_MODEL_URL		"http://www.hiwifi.com/"
#define BOARD_BOOT_TIME		25
#define BOARD_FLASH_TIME	120
#define BOARD_GPIO_BTN_RESET	18
#define BOARD_GPIO_LED_INVERTED
#undef  BOARD_GPIO_BTN_WPS
#undef  BOARD_GPIO_LED_ALL
#undef  BOARD_GPIO_LED_WIFI
#undef  BOARD_GPIO_LED_SW2G
#undef  BOARD_GPIO_LED_SW5G
#define BOARD_GPIO_LED_POWER	31 // NA
#define BOARD_GPIO_LED_LAN 7 //SYS
#define BOARD_GPIO_LED_WAN	8 //WORK
#undef  BOARD_GPIO_LED_USB
#undef  BOARD_GPIO_LED_ROUTER
#define BOARD_HAS_5G_11AC	0
#define BOARD_NUM_ANT_5G_TX	2
#define BOARD_NUM_ANT_5G_RX	2
#define BOARD_NUM_ANT_2G_TX	2
#define BOARD_NUM_ANT_2G_RX	2
#define BOARD_NUM_ETH_LEDS	1
#define BOARD_NUM_ETH_EPHY	3
#define BOARD_GPIO_PWR_USB	13
#define BOARD_GPIO_PWR_USB_ON	0
#define BOARD_HAS_EPHY_L1000	0
#define BOARD_HAS_EPHY_W1000	1
