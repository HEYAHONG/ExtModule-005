# 说明

ESP32-DevKitC是[espressif](https://www.espressif.com)官方出品的核心板开发板，可用于固件验证。

# 原理图

[ESP32-DevKitC-V4_SCH_20180607A.pdf](ESP32-DevKitC-V4_SCH_20180607A.pdf)

# 外设

核心板不包含除串口0与烧录模式选择之外任何外设，用户进行固件验证时可能视固件要求连接以下外设：

- LED灯（带限流电阻）
- 以太网PHY模块（LAN8720模块）

