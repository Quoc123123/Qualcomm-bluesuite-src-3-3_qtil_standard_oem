#  Makefile for securlib

TOP = ../..

all: build_lib

DFUBDIR = $(TOP)/devHost/dfu/BCFW/DFUBuilder

MODULE=securlib
LIBRARY=securlib
SOURCES_CPP=securlib.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

include ../openssl.path

INCLUDE_DIRS=\
    -I. \
    -I $(DFUBDIR) \
    -I $(DFUBDIR)/engine \
    -I $(OPENSSL_TOP)/include/
    
include $(TOP)/make/Makefile.inc

LINK_PRE_STUFF += --version-script=qtilsecurlib.version

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
