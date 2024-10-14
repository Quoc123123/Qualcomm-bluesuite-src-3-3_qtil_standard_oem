#  Makefile for EngineFramework

TOP=../../../..

all: build_shared_lib

MODULE=engine
SHARED_LIB=libengineframework$(SO)
SOURCES_CPP=enginefw_cpp.cpp multilistparser.cpp
SHARED_LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

INCLUDE_DIRS=\
	-I. \
	-I$(TOP)/util \
	-I$(TOP)/util/misc

MODULE_DEFINES=-DEF_DISABLE_ALL_DEBUG=1
IMPORT_LIBS=-lpthread -lrt -ldl
BUILT_LIBS=thread time

include $(TOP)/make/Makefile.inc

clean : remove_shared_objects remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
