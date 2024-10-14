#  Makefile for rsa_library

TOP = ../../../../..

all: make_int build_lib

MODULE=rsalibrary
LIBRARY=rsacommon
SOURCES_C=keygeneration/create_key.c cryption/crypt_decrypt.c cryption/crypt_exponentiation.c cryption/crypt_sign.c \
          keygeneration/mp_arithmatic.c keygeneration/mp_exponentiation.c keygeneration/prime_filter.c \
          keygeneration/random_number.c short_long_conversions.c keygeneration/strong_prime.c \
          keygeneration/test.c
SOURCES_CPP=keyfile.cpp rsa_library.cpp
LIB_OBJECTS=$(SOURCES_CPP:.cpp=$(OBJ)) $(SOURCES_C:.c=$(OBJ))
INCLUDE_DIRS=\
    -I. \
    -I ../engine

include $(TOP)/make/Makefile.inc

make_int:
	mkdir -p $(OUTPUT_INT)/cryption
	mkdir -p $(OUTPUT_INT)/keygeneration

clean : remove_libraries remove_objects remove_autodep_makefiles

-include $(SOURCES_C:.c=.d)
-include $(SOURCES_CPP:.cpp=.d)
