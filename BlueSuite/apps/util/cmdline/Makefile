#  Makefile for util/cmdline

TOP=../..

all: build_lib

MODULE=cmdline
LIBRARY=cmdline
SOURCES_CPP=cmdline.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
