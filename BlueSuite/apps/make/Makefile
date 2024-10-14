#  Master Makefile for BlueSuite 3.x Source Code Release
#
#  This directory contains the module level dependencies
#

TOP=..

HOSTBUILD_OS=linux

cmdline : enginefw ichar misc
	make -C $(TOP)/util/cmdline/ HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

enginefw : thread time
	make -C $(TOP)/util/engine/engineframework/cpp HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

ichar :
	make -C $(TOP)/util/unicode HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

keyfile : misc
	make -C $(TOP)/util/keyfile HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

misc : enginefw
	make -C $(TOP)/util/misc HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

rsa_library :
	make -C $(TOP)/devHost/dfu/BCFW/DFUBuilder/rsa HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

securitycmd : securlib cmdline
	make -C $(TOP)/security/securitycmd HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

securlib : rsa_library keyfile ichar
	make -C $(TOP)/security/securlib HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

thread : time
	make -C $(TOP)/util/thread HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

time :
	make -C $(TOP)/util/time HOSTBUILD_OS=$(HOSTBUILD_OS) $(ACTION)

