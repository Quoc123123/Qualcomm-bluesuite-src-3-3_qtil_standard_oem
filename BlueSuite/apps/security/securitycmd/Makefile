#  Makefile for securitycmd

TOP=../..

all: build_exe

MODULE=securitycmd
EXECUTABLE=securitycmd$(EXE)
SOURCES_CPP=keygen.cpp main.cpp secureappstore.cpp xuvreader.cpp
SOURCES_C=uestores.c
EXE_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ)) $(SOURCES_C:.c=$(OBJ))

IMPORT_LIBS=-lpthread -lrt -ldl -lcrypto
BUILT_LIBS=securlib rsacommon thread time cmdline ichar keyfile misc

BUILT_SHARED_OBJECTS=\
    -lengineframework

DFUBDIR = $(TOP)/devHost/dfu/BCFW/DFUBuilder

include ../openssl.path

INCLUDE_DIRS=\
    -I. \
    -I .. \
    -I $(DFUBDIR) \
    -I $(DFUBDIR)/engine \
    -I $(OPENSSL_TOP)/include/
    
include $(TOP)/make/Makefile.inc

LINK_PRE_STUFF += -L$(OPENSSL_TOP)/lib/ -Wl,-rpath,$(OPENSSL_TOP)/lib/

clean : remove_objects remove_autodep_makefiles
	-$(RM) $(OUTPUT_BIN)/securitycmd$(EXE)

-include $(SOURCES_CPP:.cpp=.d)
-include $(SOURCES_C:.c=.d)
