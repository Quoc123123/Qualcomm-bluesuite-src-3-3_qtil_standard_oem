# See config.COMMON for this trick
include ../../../../../../devHost/Makefile.path

BUILDDIR = $(INTERMEDIATE)

CC = gcc
CPLUS = g++
ARCH = ar
RANLIB = ranlib

GENDIR = ../../engine
PUBDIR = ..

CPPFLAGS = -I$(RESULT_INC) -I. -I$(PUBDIR) -I$(GENDIR) -O2 
DEBUGFLAGS = -Wall -ansi -pedantic -W -Wsign-compare -Wredundant-decls
CDEBUGFLAGS = $(DEBUGFLAGS) -Wmissing-prototypes -Wstrict-prototypes
CPPDEBUGFLAGS = $(DEBUGFLAGS)

LIB = $(BUILDDIR)/libcrypt.a
OBJS = $(BUILDDIR)/crypt_sign.o $(BUILDDIR)/crypt_decrypt.o \
	$(BUILDDIR)/crypt_exponentiation.o

all depend lib:: $(BUILDDIR)

depend::
	makedepend -Y *.c *.cpp $(CPPFLAGS)

all lib:: $(LIB)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LIB): $(OBJS)
	rm -f $(LIB)
	$(ARCH) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)
	# Let's copy it into the result lib directory so it can be reused
	cp -f $(LIB) $(RESULT_LIB)

clean_recursive: clean

clean:
	rm -f $(OBJS) $(LIB) *~ Makefile.bak

.SUFFIXES: .cpp

$(BUILDDIR)/%.o: %.c
	$(CC) -c -o $@ $(CDEBUGFLAGS) $(CPPFLAGS) $<

$(BUILDDIR)/%.o: %.cpp
	$(CPLUS) -c -o $@ $(CPPDEBUGFLAGS) $(CPPFLAGS) $<

# DO NOT DELETE

crypt_decrypt.o: crypt_private.h ../crypt_public.h ../../engine/dfu_private.h
crypt_exponentiation.o: crypt_private.h ../crypt_public.h
crypt_exponentiation.o: ../../engine/dfu_private.h
crypt_sign.o: crypt_private.h ../crypt_public.h ../../engine/dfu_private.h
