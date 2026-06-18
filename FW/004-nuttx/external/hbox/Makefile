
include $(APPDIR)/Make.defs

HRC_ROOT_PATH:= $(APPDIR)/../../3rdparty/HCppBox/master/hbox/

CSRCS +=  $(wildcard ${HRC_ROOT_PATH}/*.c)  $(wildcard ${HRC_ROOT_PATH}/h3rdparty/*.c)
CXXEXT = .cpp
CXXSRCS += $(wildcard ${HRC_ROOT_PATH}/cpp/*.cpp)

CFLAGS += -Wno-shadow -Wno-undef -Wno-restrict
CXXFLAGS += -Wno-shadow -Wno-undef -Wno-restrict

MAINSRC = hbox_config.cpp

PROGNAME = hbox
PRIORITY = 100
STACKSIZE = 4096

include $(APPDIR)/Application.mk
