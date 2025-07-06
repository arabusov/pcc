FILES = mical.h inst.h scan.c error.c init.c ins.c \
	ps.c rel.c sdi.c sym.c
OBJECTS = error.o init.o ins.o ps.o rel.o sdi.o sym.o scan.o

CFLAGS = -O0 -DX286 -Iinclude
CFLAGS += -g -ggdb -ansi
CFLAGS += -Werror -Wfatal-errors -Wno-implicit-int -Wno-deprecated-non-prototype

LDFLAGS = -g -ggdb

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c  $<

a286: $(OBJECTS)
	$(CC) $(LDFLAGS) -o a286 $(OBJECTS)

$(OBJECTS): mical.h

init.o ins.o: mical.h inst.h

clean:
	rm -f $(OBJECTS) a286 *.out
