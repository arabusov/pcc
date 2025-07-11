/* type.h - types for linker */

/* Copyright (C) 1994 Bruce Evans */

typedef int bool_pt;
typedef unsigned char bool_t;

typedef unsigned short u2_t;
typedef unsigned u2_pt;
typedef unsigned long u4_t;
typedef unsigned long u4_pt;

#ifdef HOST_8BIT
typedef char fastin_t;
#else
typedef int fastin_t;
#endif
typedef int fastin_pt;

typedef unsigned flags_t;	/* unsigned makes shifts logical */

#ifdef LONG_OFFSETS
typedef unsigned long bin_off_t;
#else
typedef unsigned bin_off_t;
#endif

#ifdef OBJ_H			/* obj.h is included */

struct entrylist		/* list of entry symbols */
{
    struct entrylist *elnext;	/* next on list */
    struct symstruct *elsymptr;	/* entry on list */
};

struct modstruct		/* module table entry format */
{
    char *filename;		/* file containing this module */
    char *archentry;		/* name of component file for archives */
    char *modname;		/* name of module */
    unsigned long textoffset;	/* offset to module text in file */
    char class;			/* class of module */
    char loadflag;		/* set if module to be loaded */
    char segmaxsize[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for seg max size */
				/* 00 = 1, 01 = 2, 10 = 3, 11 = 4 */
    char segsizedesc[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for #bytes for size */
				/* 00 = 0, 01 = 1, 10 = 2, 11 = 4 */
    struct symstruct **symparray;	/* ^array of ptrs to referenced syms */
    struct modstruct *modnext;	/* next module in order of initial reading */
    char segsize[1];		/* up to 64 size bytes begin here */
};				/* careful with sizeof( struct modstruct )!! */

struct redlist			/* list of redefined (exported) symbols */
{
    struct redlist *rlnext;	/* next on list */
    struct symstruct *rlsymptr;	/* to symbol with same name, flags */
    struct modstruct *rlmodptr;	/* module for this redefinition */
    bin_off_t rlvalue;		/* value for this redefinition */
};

struct symstruct		/* symbol table entry format */
{
    struct modstruct *modptr;	/* module where symbol is defined */
    bin_off_t value;		/* value of symbol */
    flags_t flags;		/* see below (unsigned makes shifts logical) */
    struct symstruct *next;	/* next symbol with same hash value */
    char name[1];		/* name is any string beginning here */
};				/* don't use sizeof( struct symstruct )!! */

#endif				/* obj.h is included */

/* prototypes */

#ifndef P
#ifdef __STDC__
#define P(x)	x
#else
#define P(x)	()
#endif
#endif

/* dump.c */
void dumpmods P((void));
void dumpsyms P((void));

/* io.c */
void ioinit P((char *progname));
void closein P((void));
void closeout P((void));
void errtrace P((char *name, int level));
void executable P((void));
void flusherr P((void));
void openin P((char *filename));
void openout P((char *filename));
void putstr P((char *message));
void put08x P((bin_off_t num));
void put08lx P((bin_off_t num));
void putbstr P((unsigned width, char *str));
void putbyte P((int ch));
int readchar P((void));
void readin P((char *buf, unsigned count));
bool_pt readineofok P((char *buf, unsigned count));
void seekin P((unsigned long offset));
void seekout P((unsigned long offset));
void seektrel P((unsigned long offset));
void writechar P((int c));
void writedrel P((char *buf, unsigned count));
void writeout P((char *buf, unsigned count));
void writetrel P((char *buf, unsigned count));
void fatalerror P((char *message));
void inputerror P((char *message));
void input1error P((char *message));
void outofmemory P((void));
void prematureeof P((void));
void redefined P((char *name, char *message, char *archentry,
		  char *deffilename, char *defarchentry));
void interseg P((char *fname, char *aname, char *name));
void reserved P((char *name));
void size_error P((int seg, bin_off_t count, bin_off_t size));
void undefined P((char *name));
void usage P((void));
void version_msg P((void));
void use_error P((char *message));

/* ld.c */
int main P((int argc, char **argv));

/* readobj.c */
void objinit P((void));
void readsyms P((char *filename, bool_pt trace));
#ifdef OBJ_H
void entrysym P((struct symstruct *symptr));
unsigned segsizecount P((unsigned seg, struct modstruct *modptr));
#endif
bin_off_t readconvsize P((unsigned countindex));
bin_off_t readsize P((unsigned count));

/* table.c */
void syminit P((void));
struct symstruct *addsym P((char *name));
struct symstruct *findsym P((char *name));
char *moveup P((unsigned nbytes));
char *ourmalloc P((unsigned nbytes));
void ourfree P((char *cptr));
char *readstring P((void));
void release P((char *cptr));
char *stralloc P((char *s));

/* typeconvert.c */
u2_pt c2u2 P((char *buf));
u4_t c4u4 P((char *buf));
u2_pt cnu2 P((char *buf, unsigned count));
u4_t cnu4 P((char *buf, unsigned count));
void u2c2 P((char *buf, u2_pt offset));
void u4c4 P((char *buf, u4_t offset));
void u2cn P((char *buf, u2_pt offset, unsigned count));
void u4cn P((char *buf, u4_t offset, unsigned count));
bool_pt typeconv_init P((bool_pt big_endian, bool_pt long_big_endian));

/* writebin.c */
void writebin P((char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp));

void write_rel P((char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp));

/* write_elks.c */
void write_elks P((char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp, bool_pt nsym));

/* linksym.c */
void linksyms P((bool_pt argreloc_output));
