#
# QuickJS Javascript Engine
# 
# Copyright (c) 2017-2021 Fabrice Bellard
# Copyright (c) 2017-2021 Charlie Gordon
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# consider warnings as errors (for development)
#CONFIG_WERROR=y

# installation directory
prefix=/usr/local

# include the code for BigInt/BigFloat/BigDecimal and math mode
CONFIG_BIGNUM=y

OBJDIR=.obj

EXE=

HOST_CC=gcc
CC=gcc
CFLAGS=-g -Wall -MMD -MF $(OBJDIR)/$(@F).d
CFLAGS += -Wno-array-bounds -Wno-format-truncation
AR=ar

STRIP=strip
ifdef CONFIG_WERROR
CFLAGS+=-Werror
endif
DEFINES:=-D_GNU_SOURCE -DCONFIG_VERSION=\"$(shell cat VERSION)\"
ifdef CONFIG_BIGNUM
DEFINES+=-DCONFIG_BIGNUM
endif

CFLAGS+=$(DEFINES)
CFLAGS_DEBUG=$(CFLAGS) -O0
CFLAGS_SMALL=$(CFLAGS) -Os
CFLAGS_OPT=$(CFLAGS) -O2
CFLAGS_NOLTO:=$(CFLAGS_OPT)
LDFLAGS=-g
LDEXPORT=-rdynamic

PROGS=qjsc$(EXE) run-test262
QJSC_CC=$(CC)
QJSC=./qjsc$(EXE)
#PROGS+=qjscalc
PROGS+=libquickjs.a

all: $(OBJDIR) $(PROGS)

# quickjs.c
# libregexp.c
# libunicode.c
# cutils.c
# quickjs-libc.c
QJS_LIB_OBJS=$(OBJDIR)/quickjs.o $(OBJDIR)/libregexp.o $(OBJDIR)/libunicode.o $(OBJDIR)/cutils.o $(OBJDIR)/quickjs-libc.o

ifdef CONFIG_BIGNUM
QJS_LIB_OBJS+=$(OBJDIR)/libbf.o 
endif

HOST_LIBS=-lm -ldl -lpthread
LIBS=-lm
LIBS+=-ldl -lpthread
LIBS+=$(EXTRA_LIBS)

$(OBJDIR):
	mkdir -p $(OBJDIR) $(OBJDIR)/examples $(OBJDIR)/tests

qjsc$(EXE): $(OBJDIR)/qjsc.o $(QJS_LIB_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

QJSC_DEFINES:=-DCONFIG_CC=\"$(QJSC_CC)\" -DCONFIG_PREFIX=\"$(prefix)\"
#QJSC_HOST_DEFINES:=-DCONFIG_CC=\"$(HOST_CC)\" -DCONFIG_PREFIX=\"$(prefix)\"

$(OBJDIR)/qjsc.o: CFLAGS+=$(QJSC_DEFINES)
#$(OBJDIR)/qjsc.host.o: CFLAGS+=$(QJSC_HOST_DEFINES)

libquickjs.a: $(QJS_LIB_OBJS)
	$(AR) rcs $@ $^

ifneq ($(wildcard unicode/UnicodeData.txt),)
$(OBJDIR)/libunicode.o $(OBJDIR)/libunicode.m32.o $(OBJDIR)/libunicode.m32s.o \
    $(OBJDIR)/libunicode.nolto.o: libunicode-table.h

libunicode-table.h: unicode_gen
	./unicode_gen unicode $@
endif

run-test262: $(OBJDIR)/run-test262.o $(QJS_LIB_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

run-test262-debug: $(patsubst %.o, %.debug.o, $(OBJDIR)/run-test262.o $(QJS_LIB_OBJS))
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

run-test262-32: $(patsubst %.o, %.m32.o, $(OBJDIR)/run-test262.o $(QJS_LIB_OBJS))
	$(CC) -m32 $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS_OPT) -c -o $@ $<

clean:
	rm -f repl.c qjscalc.c out.c
	rm -f *.a *.o *.d *~ unicode_gen regexp_test $(PROGS)
	rm -f hello.c test_fib.c
	rm -f examples/*.so tests/*.so
	rm -rf $(OBJDIR)/ *.dSYM/ qjs-debug
	rm -rf run-test262-debug run-test262-32

install: all
	mkdir -p "$(DESTDIR)$(prefix)/bin"
	$(STRIP) qjs qjsc
	install -m755 qjs qjsc "$(DESTDIR)$(prefix)/bin"
	ln -sf qjs "$(DESTDIR)$(prefix)/bin/qjscalc"
	mkdir -p "$(DESTDIR)$(prefix)/lib/quickjs"
	install -m644 libquickjs.a "$(DESTDIR)$(prefix)/lib/quickjs"
ifdef CONFIG_LTO
	install -m644 libquickjs.lto.a "$(DESTDIR)$(prefix)/lib/quickjs"
endif
	mkdir -p "$(DESTDIR)$(prefix)/include/quickjs"
	install -m644 quickjs.h quickjs-libc.h "$(DESTDIR)$(prefix)/include/quickjs"

-include $(wildcard $(OBJDIR)/*.d)
