VERSION = 0.16.10
LIBDIR	= /usr/lib/bcc
CFLAGS	=-O -Wfatal-errors -Werror
LDFLAGS	=

# May need some of these if the auto-sense fails.
# -DV7_A_OUT			# a.out.h is like V7
# -DBSD_A_OUT			# a.out.h is like BSD
# -DSTANDARD_GNU_A_OUT		# a.out.h is like GNU normal.
# -DNO_AOUT			# a.out.h is like nothing known!
#
DEFS	=-DREL_OUTPUT -DBUGCOMPAT -DVERSION='"$(VERSION)"'

# An alternative file for a non-standard a.out.h (eg i386 linux on an Alpha)
#
# NATIVE=-DA_OUT_INCL='"a_out_local.h"'

OBJS= dumps.o io.o ld.o readobj.o table.o typeconv.o linksyms.o \
      writex86.o writebin.o writerel.o

all: ld86 objchop catimage objdump86

ld86: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

install: ld86
	install -d $(LIBDIR)
	install -m 755 $^ $(LIBDIR)

clean realclean clobber:
	rm -f *.o ld86 ld86r objchop catimage objdump86

$(OBJS): align.h ar.h bindef.h byteord.h config.h const.h globvar.h obj.h \
	 syshead.h type.h x86_aout.h

ar.h:
	[ -f ar.h ] || \
	{ rm -f ar.h ; ln -s ../libc/include/ar.h . ; } || \
	ln ../libc/include/ar.h .

writebin.o: writebin.c
	$(CC) $(CFLAGS) $(DEFS) $(NATIVE) -c $<

writerel.o: writebin.c

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
