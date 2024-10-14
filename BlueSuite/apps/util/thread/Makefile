#  Makefile for util/thread

TOP=../..

all: build_lib

MODULE=thread
LIBRARY=thread
SOURCES_CPP=atomic_counter.cpp critical_section_c.cpp critical_section.cpp signal_box.cpp thread.cpp system_wide_mutex.cpp shared_memory.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ))

include $(TOP)/make/Makefile.inc

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_CPP:.cpp=.d)
