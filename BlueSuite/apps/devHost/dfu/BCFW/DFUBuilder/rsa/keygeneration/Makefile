# See config.COMMON for this trick
include ../../../../../../devHost/Makefile.path

BUILDDIR = $(INTERMEDIATE)

CC = gcc
CPLUS = g++
ARCH = ar
RANLIB = ranlib

COMMONDIR = ../../engine
RSADIR = $(COMMONDIR)/rsa

MPAOBJ = $(BUILDDIR)/mp_arithmatic.o
MPEOBJ = $(BUILDDIR)/mp_exponentiation.o
RANOBJ = $(BUILDDIR)/random_number.o
PRIOBJ = $(BUILDDIR)/prime_filter.o
GOROBJ = $(BUILDDIR)/strong_prime.o
KEYOBJ = $(BUILDDIR)/create_key.o

LIB = $(BUILDDIR)/libkey.a
TEST_PROG = $(BUILDDIR)/key_test
OBJS = $(RANOBJ) $(MPAOBJ) $(MPEOBJ) $(PRIOBJ) $(GOROBJ) $(KEYOBJ)

COMPILEFLAGS = -I$(RESULT_INC) -I. -I.. -I$(COMMONDIR) -I$(RSADIR) -O2
DEBUGFLAGS = -Wall -ansi -pedantic -W -Wsign-compare -Wredundant-decls 
CDEBUGFLAGS = $(DEBUGFLAGS) -Wmissing-prototypes -Wstrict-prototypes
CPPDEBUGFLAGS = $(DEBUGFLAGS)

lib all depend test:: $(BUILDDIR)

all lib:: $(LIB) $(COMMONDIR)/dfu_private.h

depend::
	makedepend -Y *.c *.cpp $(COMPILEFLAGS)

new:
	$(MAKE) clean
	$(MAKE) all

all test:: $(LIB)
	$(CC) test.c $(LIB) $(DEBUGFLAGS) $(COMPILEFLAGS) -o $(TEST_PROG)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LIB): $(OBJS) $(RSALIB)
	rm -f $(LIB)
	$(ARCH) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f $(OBJS) $(LIB) *~ $(TEST_PROG) Makefile.bak

$(GOROBJ):

$(RANOBJ):

$(MPEOBJ):

$(MPAOBJ):

$(PRIOBJ):

.SUFFIXES: .cpp

$(BUILDDIR)/%.o : %.c
	$(CC) -c -o $@ $(C$(WARN)FLAGS) $(COMPILEFLAGS) $<

$(BUILDDIR)/%.o : %.cpp
	$(CPLUS) -c -o $@ $(CPP$(WARN)FLAGS) $(COMPILEFLAGS) $<

# DO NOT DELETE

$(BUILDDIR)/create_key.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/create_key.o: ../keygen_public.h
$(BUILDDIR)/create_key.o: create_key.h random_number.h mp_arithmatic.h
$(BUILDDIR)/create_key.o: mp_exponentiation.h prime_filter.h strong_prime.h
$(BUILDDIR)/mp_arithmatic.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/mp_arithmatic.o: ../keygen_public.h create_key.h random_number.h
$(BUILDDIR)/mp_arithmatic.o: mp_arithmatic.h mp_exponentiation.h prime_filter.h
$(BUILDDIR)/mp_arithmatic.o: strong_prime.h
$(BUILDDIR)/mp_exponentiation.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/mp_exponentiation.o: ../keygen_public.h create_key.h
$(BUILDDIR)/mp_exponentiation.o: mp_arithmatic.h mp_exponentiation.h
$(BUILDDIR)/mp_exponentiation.o: prime_filter.h strong_prime.h random_number.h
$(BUILDDIR)/prime_filter.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/prime_filter.o: ../keygen_public.h create_key.h random_number.h
$(BUILDDIR)/prime_filter.o: mp_arithmatic.h mp_exponentiation.h prime_filter.h
$(BUILDDIR)/prime_filter.o: strong_prime.h
$(BUILDDIR)/random_number.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/random_number.o: ../keygen_public.h create_key.h random_number.h
$(BUILDDIR)/random_number.o: mp_arithmatic.h mp_exponentiation.h prime_filter.h
$(BUILDDIR)/random_number.o: strong_prime.h
$(BUILDDIR)/strong_prime.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/strong_prime.o: ../keygen_public.h create_key.h random_number.h
$(BUILDDIR)/strong_prime.o: mp_arithmatic.h mp_exponentiation.h prime_filter.h
$(BUILDDIR)/strong_prime.o: strong_prime.h trial_division.h
$(BUILDDIR)/test.o: keygen_private.h ../../engine/dfu_private.h
$(BUILDDIR)/test.o: ../keygen_public.h create_key.h random_number.h
$(BUILDDIR)/test.o: mp_arithmatic.h mp_exponentiation.h prime_filter.h
$(BUILDDIR)/test.o: strong_prime.h
