#  Makefile for util/unicode

TOP=../..

all: build_lib

MODULE=unicode
LIBRARY=ichar
SOURCES_CPP=inarrow.cpp
SOURCES_C=ichar.c main.c
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ)) $(SOURCES_C:.c=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
-include $(SOURCES_C:.c=.d)
