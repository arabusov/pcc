#
#	@(#)Makefile	4.2.1	(2.11BSD)	1996/10/24
#
DESTDIR=/usr/local
PARSERDIR=$(DESTDIR)/share/misc/
PARSER=\"$(PARSERDIR)/yaccpar\"
SEPFLAG= -i
CFLAGS=-O -DPARSER=$(PARSER) -wo
SRCS = Makefile dextern files yaccpar \
	y1.c y2.c y3.c y4.c \
	yaccdiffs yaccnews

all: yacc

yacc: y1.o y2.o y3.o y4.o
	$(CC) ${SEPFLAG} -o yacc y?.o
y1.o y2.o y3.o y4.o: dextern files
install: yacc yaccpar
	install -s yacc $(DESTDIR)/bin
	install -d $(PARSERDIR)
	install -c yaccpar $(PARSERDIR)
clean :
	-rm -f *.o yacc
