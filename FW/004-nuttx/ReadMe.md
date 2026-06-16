# 说明

本目录为采用[nuttx](https://nuttx.apache.org/)的基础模板。


# 编译环境

本工程主要支持Linux / WSL。注意:由于nuttx编译过程中需要下载大量数据,请保持网络通畅(尤其是[github.com](https://github.com))。

本工程中采用esp-idf的安装脚本安装工具链,需要执行[../esp-idf-env.sh](../esp-idf-env.sh)进入编译环境。

nuttx采用esp-idf的工具链编译,但不使用esp-idf的idf.py而是make，由于支持的原因，不采用cmake。

## 编译环境安装

参考 [esp-idf](https://docs.espressif.com/projects/esp-idf/) 与 [nuttx](https://nuttx.apache.org/) 的说明。

对于ubuntu 22.04（及更新版本的ubuntu） 可采用以下指令安装(如缺少其它命令，请根据提示操作):

```bash
sudo apt-get install build-essential make cmake python3-pip python3-venv curl wget kconfig-frontends git patch findutils sed
```

# 编译

编译之前需要执行需要执行[../esp-idf-env.sh](../esp-idf-env.sh)进入编译环境。

## 构建

```bash
make
```

## 烧录

```bash
#直接烧录
make flash
#使用ESPTOOL_PORT指定烧录端口，不指定默认为/dev/ttyUSB0。 
#使用ESPTOOL_BAUD指定烧录端口，不指定默认为115200。
make flash ESPTOOL_PORT=串口设备 ESPTOOL_BAUD=波特率
```

## 调试

除了自行使用串口调试工具,也可使用以下命令调试。注意:烧录时需要手动关闭调试相关进程。

```bash
#putty,若安装了putty且处于桌面环境，可使用putty。
make putty
#使用ESPTOOL_PORT指定烧录端口，不指定默认为/dev/ttyUSB0。 
#使用ESPTOOL_BAUD指定烧录端口，不指定默认为115200。
make putty ESPTOOL_PORT=串口设备 ESPTOOL_BAUD=波特率

```



## Kconfig配置

```bash
make menuconfig
```

## 清理

```bash
make clean
```

## 清理所有

此清理除了清理生成的.o文件,还会清理下载的文件。对于处理莫名奇妙出现的编译错误非常有用。

```bash
make distclean
```

若此命令仍然不能解决问题（如Kconfig有误导致路径错误）,请直接删除nuttx/.config。
