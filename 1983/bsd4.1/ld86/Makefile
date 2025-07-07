FILES = ld86.c ../lib86/include/ranlib.h
OBJECTS = ld86.o

CFLAGS = -O0 -Iinclude
CFLAGS += -g -ggdb -ansi
CFLAGS += -Werror -Wfatal-errors -Wno-implicit-int -Wno-deprecated-non-prototype

LDFLAGS = -g -ggdb

.SUFFIXES: .o.c
.c.o:
	$(CC) $(CFLAGS) -c $<


ld86: $(OBJECTS)
	$(CC) $(LDFLAGS) -o ld86 $(OBJECTS)

$(OBJECTS): ../lib86/include/ranlib.h

clean:
	rm -f $(OBJECTS) ld86
