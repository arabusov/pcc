/* The <a.out> header file describes the format of executable files. */

#ifndef _AOUT_H
#define _AOUT_H

#include <stdint.h>

struct exec
{				/* a.out header */
  uint8_t	a_magic[2];	/* magic number */
  uint8_t	a_flags;	/* flags, see below */
  uint8_t	a_cpu;		/* cpu id */
  uint8_t	a_hdrlen;	/* length of header */
  uint8_t	a_unused;	/* reserved for future use */
  uint16_t a_version;	/* version stamp (not used at present) */
  uint32_t	a_text;		/* size of text segement in bytes */
  uint32_t	a_data;		/* size of data segment in bytes */
  uint32_t	a_bss;		/* size of bss segment in bytes */
  uint32_t	a_entry;	/* entry point */
  uint32_t	a_total;	/* total memory allocated */
  uint32_t	a_syms;		/* size of symbol table */

  /* SHORT FORM ENDS HERE */
  uint32_t	a_trsize;	/* text relocation size */
  uint32_t	a_drsize;	/* data relocation size */
  uint32_t	a_tbase;	/* text relocation base */
  uint32_t	a_dbase;	/* data relocation base */
};

#define A_MAGIC0      (uint8_t) 0x01
#define A_MAGIC1      (uint8_t) 0x03
#define O_MAGIC0	(uint8_t) 0x0a;
#define O_MAGIC1	(uint8_t) 0x01;
#define BADMAG(X)     ((X).a_magic[0] != A_MAGIC0 ||(X).a_magic[1] != A_MAGIC1)

/* CPU Id of TARGET machine (byte order coded in low order two bits) */
#define A_NONE	0x00	/* unknown */
#define A_I8086	0x04	/* intel i8086/8088 */
#define A_M68K	0x0B	/* motorola m68000 */
#define A_NS16K	0x0C	/* national semiconductor 16032 */
#define A_I80386 0x10	/* intel i80386 */
#define A_SPARC	0x17	/* Sun SPARC */

#define A_BLR(cputype)	((cputype&0x01)!=0) /* TRUE if bytes left-to-right */
#define A_WLR(cputype)	((cputype&0x02)!=0) /* TRUE if words left-to-right */

/* Flags. */
#define A_UZP	0x01	/* unmapped zero page (pages) */
#define A_PAL	0x02	/* page aligned executable */
#define A_NSYM	0x04	/* new style symbol table */
#define A_EXEC	0x10	/* executable */
#define A_SEP	0x20	/* separate I/D */
#define A_PURE	0x40	/* pure text */		/* not used */
#define A_TOVLY	0x80	/* text overlay */	/* not used */

/* Offsets of various things. */
#define A_MINHDR	32
#define	A_TEXTPOS(X)	((long)(X).a_hdrlen)
#define A_DATAPOS(X)	(A_TEXTPOS(X) + (X).a_text)
#define	A_HASRELS(X)	((X).a_hdrlen > (uint8_t) A_MINHDR)
#define A_HASEXT(X)	((X).a_hdrlen > (uint8_t) (A_MINHDR +  8))
#define A_HASLNS(X)	((X).a_hdrlen > (uint8_t) (A_MINHDR + 16))
#define A_HASTOFF(X)	((X).a_hdrlen > (uint8_t) (A_MINHDR + 24))
#define A_TRELPOS(X)	(A_DATAPOS(X) + (X).a_data)
#define A_DRELPOS(X)	(A_TRELPOS(X) + (X).a_trsize)
#define A_SYMPOS(X)	(A_TRELPOS(X) + (A_HASRELS(X) ? \
  			((X).a_trsize + (X).a_drsize) : 0))

struct reloc {
  long r_vaddr;			/* virtual address of reference */
  uint16_t r_symndx;	/* internal segnum or extern symbol num */
  uint16_t r_type;	/* relocation type */
};

/* r_tyep values: */
#define R_ABBS		0
#define R_RELLBYTE	2
#define R_PCRBYTE	3
#define R_RELWORD	4
#define R_PCRWORD	5
#define R_RELLONG	6
#define R_PCRLONG	7
#define R_REL3BYTE	8
#define R_KBRANCHE	9

/* r_symndx for internal segments */
#define S_ABS		((uint16_t)-1)
#define S_TEXT		((uint16_t)-2)
#define S_DATA		((uint16_t)-3)
#define S_BSS		((uint16_t)-4)

struct nlist {			/* symbol table entry */
	union {
		char      *n_name; /* for use when in-core */
		long      n_strx;  /* index into file string table */
	} n_un;
	uint8_t n_type;  /* type flag, i.e. N_TEXT etc; see below */
	char          n_other;
	short         n_desc;  /* see <stab.h> */
	unsigned      n_value; /* value of this symbol (or sdb offset) */
};

/* Low bits of storage class (section). */
#define	N_SECT		  07	/* section mask */
#define N_UNDF		  00	/* undefined */
#define N_ABS		  01	/* absolute */
#define N_TEXT		  02	/* text */
#define N_DATA		  03	/* data */
#define	N_BSS		  04	/* bss */
#define N_COMM		  05	/* (common) */

/* High bits of storage class. */
#define N_CLASS		0370	/* storage class mask */
#define C_NULL
#define C_EXT		0020	/* external symbol */
#define C_STAT		0030	/* static */
#define  N_EXT         01       /* external bit, or'ed in */

/* Function prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int nlist, (char *_file, struct nlist *_nl)			);

/* This structure describes a single relocation to be performed.
   The text-relocation section of the file is a vector of these structures,
   all of which apply to the text section.
   Likewise, the data-relocation section applies to the data section.  */

struct relocation_info
{
  int r_address;
  unsigned int r_symbolnum:24;
  unsigned int r_pcrel:1;
  unsigned int r_length:2;
  unsigned int r_extern:1;
  unsigned int r_pad:4;
};

#endif /* _AOUT_H */
