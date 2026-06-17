
include $(APPDIR)/Make.defs

HRC_ROOT_PATH:= $(APPDIR)/../../3rdparty/HCppBox/master/hrc/

CSRCS +=  ${HRC_ROOT_PATH}/RC.c RC_fs.c


.PHONY: RC_fs.c fsgen

RC_fs.c:fsgen
	@PATH=$(shell pwd) fsgen $(shell pwd)/fs $(shell pwd)/RC_fs.c

fsgen:
	@g++ -static -static-libgcc -static-libstdc++  ${HRC_ROOT_PATH}/fsgen.cpp -o $(shell pwd)/fsgen


distclean::
	$(call DELFILE, RC_fs.c)
	$(call DELFILE, fsgen)

include $(APPDIR)/Application.mk
