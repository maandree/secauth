.POSIX:

LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk



OBJ =\
	libsecauth_client_hash.o\
	libsecauth_format_spec.o\
	libsecauth_parse_spec.o\
	libsecauth_server_hash.o

HDR =\
	libsecauth.h

LOBJ = $(OBJ:.o=.lo)


all: libsecauth.a libsecauth.$(LIBEXT)
$(OBJ): $($@:.o=.c) $(HDR)

libsecauth.a: $(OBJ)
	-rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

libsecauth.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) $(LDFLAGS_METHODS) -o $@ $(LOBJ) $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

install: libsecauth.a libsecauth.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libsecauth.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libsecauth.h "$(DESTDIR)$(PREFIX)/include/"
	cp -- libsecauth.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBMINOREXT)"
	ln -sf -- libsecauth.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBMAJOREXT)"
	ln -sf -- libsecauth.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBEXT)"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libsecauth.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libsecauth.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libsecauth.h"

clean:
	-rm -rf -- *.o *.a *.so *.lo *.su *.dll *.dylib

.SUFFIXES:
.SUFFIXES: .c .o .lo

.PHONY: all install uninstall clean
