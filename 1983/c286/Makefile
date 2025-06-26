CFLAGS=-Iinclude
CFLAGS +=-O0 -g -ggdb
CFLAGS +=-Werror -Wno-implicit-int -Wno-deprecated-non-prototype
I=include
COMP=gcc

.SUFFIXES: .o .c

.c.o:
	$(COMP) $(CFLAGS) -c -DBUG4 $<

OBJ0=	trees1.o trees2.o optim.o pftn1.o pftn2.o code.o local.o scan.o \
	xdefs.o cgram.o
OBJ1=	reader.o order.o match.o allo.o comm1.o local2.o table.o

c286:	$(OBJ0) $(OBJ1)
	cc $(LDFLAGS) -o c286 $(OBJ0) $(OBJ1)

$(OBJ0) $(OBJ0b): $(I)/manifest.h macdefs mfile1
$(OBJ1) $(OBJ1b): $(I)/manifest.h mfile2 mac2defs macdefs

clean:
	rm -f $(OBJ0) $(OBJ1)
	rm -f c286
