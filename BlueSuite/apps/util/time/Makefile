#  Makefile for util/time

TOP=../..

all: build_lib

MODULE=time
LIBRARY=time
SOURCES_CPP=passive_timer.cpp stop_watch.cpp time_stamp.cpp
SOURCES_C=hi_res_clock.c
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ)) $(SOURCES_C:.c=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
-include $(SOURCES_C:.c=.d)
