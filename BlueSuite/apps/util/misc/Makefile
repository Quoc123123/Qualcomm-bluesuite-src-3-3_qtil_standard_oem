#  Makefile for util/misc

TOP=../..

all: build_lib

MODULE=misc
LIBRARY=misc
SOURCES_CPP=fileutil.cpp multilistparser.cpp pluginmanager.cpp stringutil.cpp sysutil.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
