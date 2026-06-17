
include .config

PROJECT_ROOT := $(shell pwd)

ESPTOOL_PORT ?= /dev/ttyUSB0
ESPTOOL_BAUD ?= 115200
ESPTOOL_BINDIR ?= $(shell pwd)/boot/

ifeq ($(CONFIG_ESP32_SPIFLASH),y)
ifeq ($(ONFIG_FS_SPIFFS),y)
SPIFFSGEN ?= $(shell pwd)/../3rdparty/esp-idf/components/spiffs/spiffsgen.py
SPIFFSROOT ?= $(shell pwd)/spiffs
SPIFFSBIN ?= $(shell pwd)/spiffs.bin
SPIFFSOBJNAMELEN ?=  $(shell expr $(CONFIG_SPIFFS_NAME_MAX) + 1 )
SPIFFSGENFLAGS := --follow-symlinks --meta-len 0  --obj-name-len ${SPIFFSOBJNAMELEN} ${CONFIG_ESP32_STORAGE_MTD_SIZE} "${SPIFFSROOT}"  "${SPIFFSBIN}"
endif
endif

.PHONY:  all
all:checkconfig
	@${MAKE} -C nuttx


.PHONY:  putty
putty : checkconfig
ifeq (${DISPLAY},)
	@echo 当前不处于桌面环境，不可使用putty.
	@exit 2
endif
ifeq ($(shell which putty),)
	@echo 未安装putty，请安装putty.
	@exit 2
endif
	@putty -serial -sercfg ${ESPTOOL_BAUD},8,n,1 ${ESPTOOL_PORT} 

.PHONY:  flash
flash:checkconfig
ifeq ($(CONFIG_ESP32_SPIFLASH),y)
ifeq ($(ONFIG_FS_SPIFFS),y)
#生成spiffs.bin
	${SPIFFSGEN} ${SPIFFSGENFLAGS}
#烧录spiffs.bin
	@esptool.py -c esp32 -p ${ESPTOOL_PORT}  -b ${ESPTOOL_BAUD}  write_flash -fs detect ${CONFIG_ESP32_STORAGE_MTD_OFFSET} ${SPIFFSBIN}
endif
endif
#执行烧录
	@${MAKE} -C nuttx flash ESPTOOL_PORT=${ESPTOOL_PORT} ESPTOOL_BAUD=${ESPTOOL_BAUD} ESPTOOL_BINDIR=${ESPTOOL_BINDIR}

.PHONY:  clean
clean:checkconfig
	@${MAKE} -C nuttx clean

.PHONY:  distclean
distclean:checkconfig
	@${MAKE} -C nuttx distclean

.PHONY:  menuconfig
menuconfig:checkconfig
	@${MAKE} -C nuttx menuconfig
	@cp -rf nuttx/.config  ./.config

.PHONY:  checkconfig
checkconfig:sourcecheck nuttx/.config


.PHONY:  sourcecheck
sourcecheck:
	@[ -f nuttx/Makefile ] || (git submodule update --init && echo 请使用git下载子模块)
	@[ -f nuttx/Makefile ] || exit
	@ln -sfT ${PROJECT_ROOT}/external ${PROJECT_ROOT}/apps/external

nuttx/.config: sourcecheck
	@./nuttx/tools/configure.sh   esp32-devkitc:cxx
	@cp .config nuttx/.config




