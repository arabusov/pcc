#	$Id$
#
# Makefile  for ccom
#
VPATH=..
srcdir=../ccom
top_srcdir=..
CC = clang
EXEEXT =
BINPREFIX = i86-minix-
CC_FOR_BUILD = clang
CFLAGS = -Wfatal-errors -Werror -g -ggdb
CPPFLAGS = -D_ISOC99_SOURCE \
	-Dos_$(TARGOS) -Dmach_$(TARGMACH) \
	-I$(srcdir) -I$(MIPDIR) -I$(MDIR) \
	-I$(top_srcdir)/$(TARGOS) -I$(COMMONDIR)
LIBS =
LDFLAGS = -g -ggdb
LEX = flex
LFLAGS =
LEX_OUTPUT_ROOT = lex.yy
YACC = byacc
YFLAGS =
TARGOS = minix
TARGOSVER = 1.7.5
TARGMACH = i86
prefix =
exec_prefix =
libexecdir =
datarootdir =
mandir =
INSTALL =
INSTALL_PROGRAM =
INSTALL_DATA =
CCOM=$(BINPREFIX)ccom$(EXEEXT)
CC0=$(BINPREFIX)cc0$(EXEEXT)
CC1=$(BINPREFIX)cc1$(EXEEXT)
CF0=-DPASS1
CF1=-DPASS2

MDIR=$(top_srcdir)/$(TARGMACH)
MIPDIR=$(top_srcdir)/mip
COMMONDIR=$(top_srcdir)/common

DEST=$(CCOM)
MANPAGE=$(BINPREFIX)ccom
MKEXT=mkext$(EXEEXT)

OBJS=	builtins.o cgram.o code.o common.o compat.o dwarf.o external.o	\
	gcc_compat.o init.o inline.o local.o local2.o main.o		\
	match.o optim.o optim2.o order.o pftn.o reader.o		\
	regs.o scan.o stabs.o symtabs.o table.o trees.o unicode.o \
	softfloat.o

OBJS0=  builtins.o cgram.o code.o common.o compat.o dwarf.o external.o	\
	gcc_compat.o init.o inline.o local.o main.o             	\
	optim.o pftn.o 							\
	scan.o stabs.o symtabs.o trees.o unicode.o \
	softfloat.o

OBJS1=  common2.o compat.o external.o           			\
	local2.o main2.o                				\
	match.o optim2.o order.o reader.o      				\
	regs.o table.o

all: $(CC0) $(CC1)

LOBJS=	mkext.lo common.lo table.lo

HDRS=	$(srcdir)/pass1.h $(MIPDIR)/pass2.h $(MIPDIR)/manifest.h	\
	$(MDIR)/macdefs.h $(MIPDIR)/node.h $(COMMONDIR)/compat.h	\
	$(COMMONDIR)/unicode.h

#
# round 1: generate external.[ch], cgram.[ch] & scan.c
#

$(LOBJS): $(HDRS)

mkext.lo: $(MIPDIR)/mkext.c
	$(CC_FOR_BUILD) $(CFLAGS) $(CPPFLAGS) -DMKEXT -c -o $@ $(MIPDIR)/mkext.c

common.lo: $(MIPDIR)/common.c
	$(CC_FOR_BUILD) $(CFLAGS) $(CPPFLAGS) -DMKEXT -c -o $@ $(MIPDIR)/common.c

table.lo: $(MDIR)/table.c
	$(CC_FOR_BUILD) $(CFLAGS) $(CPPFLAGS) -DMKEXT -c -o $@ $(MDIR)/table.c

$(MKEXT): $(LOBJS)
	$(CC_FOR_BUILD) $(LDFLAGS) $(LOBJS) -o $@ $(LIBS)

external.c: $(MKEXT)
	./$(MKEXT)

cgram.c: $(srcdir)/cgram.y
	$(YACC) $(YFLAGS) -d $(srcdir)/cgram.y
	mv -f y.tab.c cgram.c
	mv -f y.tab.h cgram.h

scan.c: $(srcdir)/scan.l
	$(LEX) $(LFLAGS) $(srcdir)/scan.l
	mv -f $(LEX_OUTPUT_ROOT).c scan.c

#
# round 2: compile $(OBJS)
#

$(OBJS): $(HDRS) external.c cgram.c

builtins.o: $(srcdir)/builtins.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/builtins.c

cgram.o: cgram.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ cgram.c

code.o: $(MDIR)/code.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MDIR)/code.c

common.o: $(MIPDIR)/common.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/common.c

common2.o: $(MIPDIR)/common.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/common.c

compat.o: $(COMMONDIR)/compat.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(COMMONDIR)/compat.c

dwarf.o: $(srcdir)/dwarf.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/dwarf.c

external.o: external.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ external.c

gcc_compat.o: $(srcdir)/gcc_compat.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/gcc_compat.c

init.o: $(srcdir)/init.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/init.c

inline.o: $(srcdir)/inline.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/inline.c

local.o: $(MDIR)/local.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MDIR)/local.c

local2.o: $(MDIR)/local2.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MDIR)/local2.c

main.o: $(srcdir)/main.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/main.c

main2.o: $(srcdir)/main.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/main.c

match.o: $(MIPDIR)/match.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/match.c

optim.o: $(srcdir)/optim.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/optim.c

optim2.o: $(MIPDIR)/optim2.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/optim2.c

order.o: $(MDIR)/order.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MDIR)/order.c

pftn.o: $(srcdir)/pftn.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/pftn.c

reader.o: $(MIPDIR)/reader.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/reader.c

regs.o: $(MIPDIR)/regs.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MIPDIR)/regs.c

scan.o: scan.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ scan.c

softfloat.o: $(COMMONDIR)/softfloat.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(COMMONDIR)/softfloat.c

stabs.o: $(srcdir)/stabs.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/stabs.c

symtabs.o: $(srcdir)/symtabs.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/symtabs.c

table.o: $(MDIR)/table.c
	$(CC) $(CF1) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(MDIR)/table.c

trees.o: $(srcdir)/trees.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(srcdir)/trees.c

unicode.o: $(COMMONDIR)/unicode.c
	$(CC) $(CF0) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(COMMONDIR)/unicode.c

#
# round 3: build $(DEST)
#

$(CCOM): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

$(CC0):	$(OBJS0)
	$(CC) $(LDFLAGS) $(OBJS0) -o $@ $(LIBS)

$(CC1):	external.o $(OBJS1)
	$(CC) $(LDFLAGS) $(OBJS1) -o $@ $(LIBS)

install: $(DEST)
	test -z "$(DESTDIR)$(libexecdir)" || mkdir -p "$(DESTDIR)$(libexecdir)"
	$(INSTALL_PROGRAM) $(DEST) $(DESTDIR)$(libexecdir)
	test -z "$(DESTDIR)$(mandir)/man1" || mkdir -p "$(DESTDIR)$(mandir)/man1"
	$(INSTALL_DATA) $(srcdir)/ccom.1 $(DESTDIR)$(mandir)/man1/$(MANPAGE).1

clean:
	rm -f $(DEST) $(OBJS0) $(OBJS1) $(MKEXT) $(LOBJS) \
	$(LEX_OUTPUT_ROOT).c scan.c y.tab.[ch] cgram.[ch] external.[ch]
	rm -f $(CCOM) $(CC0) $(CC1)
