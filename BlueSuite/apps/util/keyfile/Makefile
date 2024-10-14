#  Makefile for util/keyfile

TOP=../..

all: build_lib

MODULE=keyfile
LIBRARY=keyfile
SOURCES_CPP=keyfile.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
