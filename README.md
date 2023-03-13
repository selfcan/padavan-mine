# Padavan-KVR #

fork于fightroad的仓库 https://github.com/fightroad/Padavan-KVR.git 

最终也是是vb1948  https://github.com/vb1980/Padavan-KVR.git

透明主题使用的是yuos-bit  https://github.com/yuos-bit/Padavan.git

想要没有改主题的可以去上面大佬主页fork

默认纯净没有添加插件，[在线云编译修改插件](.github/workflows/NEWIFI3.yml) [自定义增减插件](trunk/configs/templates/NEWIFI3.config)

[修改自己想要的背景](/trunk/user/www/n56u_ribbon_fixed/bootstrap/img/bg/wood.jpg) 替换wood.jpg文件就行

[修改自己想要的LOGO](/trunk/user/www/n56u_ribbon_fixed/bootstrap/img/asus_logo.png) 替换asus_logo.png文件就行(像素尺寸要求150×70）

[修改默认管理地址wifi名称账号密码](trunk/user/shared/defaults.h) 

### UI预览 ###
![](https://github.com/lmq8267/padavan-KVR/raw/main/.github/workflows/%E6%8D%95%E8%8E%B7(1).PNG)
![](https://github.com/lmq8267/padavan-KVR/raw/main/.github/workflows/%E6%B7%BB%E5%8A%A0%E7%8A%B6%E6%80%81%E6%98%BE%E7%A4%BA%E8%AE%BE%E5%A4%87ipv6%E5%8F%96%E6%B6%88%E9%A1%B6%E9%83%A8%E5%85%B3%E6%9C%BA%E6%8C%89%E9%92%AE%E7%94%A8ttyd%E4%BB%A3%E6%9B%BF.PNG)


基于hanwckf,chongshengB以及padavanonly的源码整合而来，支持7603/7615/7915的kvr  
编译方法同其他Padavan源码，主要特点如下：  
1.采用padavanonly源码的5.0.4.0无线驱动，支持kvr  
2.添加了chongshengB源码的所有插件  
3.其他部分等同于hanwckf的源码，有少量优化来自immortalwrt的padavan源码  
4.添加了MSG1500的7615版本config  
  
以下附上他们四位的源码地址供参考  
https://github.com/hanwckf/rt-n56u  
https://github.com/chongshengB/rt-n56u  
https://github.com/padavanonly/rt-n56u  
https://github.com/immortalwrt/padavan
  
最后编译出的固件对7612无线的支持已知是有问题的，包含7612的机型比如B70是无法正常工作的  
已测试的机型为MSG1500-7615，JCG-Q20，CR660x  
  
固件默认wifi名称
 - 2.4G：机器名_mac地址最后四位，如K2P_9981
 - 5G：机器名_5G_mac地址最后四位，如K2P_5G_9981

wifi密码
 - haierdianqi123

管理地址
 - 10.10.10.10

管理账号密码
 - admin
 - admin

**最近的更新代码都来自于hanwckf和MelsReallyBa大佬的4.4内核代码**
- https://github.com/hanwckf/padavan-4.4
- https://github.com/MeIsReallyBa/padavan-4.4
