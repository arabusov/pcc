#ifndef AOUT_H
#define AOUT_H

#include <stdint.h>

/*
 * Header prepended to each a.out file.
 */
struct exec {
    int32_t a_magic;	/* magic number */
uint32_t	a_text;		/* size of text segment */
uint32_t	a_data;		/* size of initialized data */
uint32_t	a_bss;		/* size of uninitialized data */
uint32_t	a_syms;		/* size of symbol table */
uint32_t	a_entry;	/* entry point */
uint32_t	a_trsize;	/* size of text relocation */
uint32_t	a_drsize;	/* size of data relocation */
};

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */

/*
 * Macros which take exec structures as arguments and tell whether
 * the file has a reasonable magic number or offsets to text|symbols|strings.
 */
#define	N_BADMAG(x) \
    (((x).a_magic)!=OMAGIC && ((x).a_magic)!=NMAGIC && ((x).a_magic)!=ZMAGIC)

#define	N_TXTOFF(x) \
	((x).a_magic==ZMAGIC ? 1024 : sizeof (struct exec))
#define N_SYMOFF(x) \
	(N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize+(x).a_drsize)
#define	N_STROFF(x) \
	(N_SYMOFF(x) + (x).a_syms)

/*
 * Format of a relocation datum.
 */
struct relocation_info {
	int32_t r_address;	/* address which is relocated */
    uint32_t r_symbolnum:16,	/* local symbol ordinal */
		r_crap:8,	/* high symbolnum, set to zero (SAW)	*/
		r_pcrel:1, 	/* was relocated pc relative already */
		r_length:2,	/* 0=byte, 1=word, 2=long */
		r_extern:1,	/* does not include value of sym referenced */
		:4;		/* nothing, yet */
};

/*
 * Format of a symbol table entry; this file is included by <a.out.h>
 * and should be used if you aren't interested the a.out header
 * or relocation information.
 */
struct	nlist {
	union {
        char    *n_name;    /* used for LD, for 64bit determines size */
		int32_t n_strx;		/* index into file string table */
	} n_un;
    uint8_t n_type;		/* type flag, i.e. N_TEXT etc; see below */
	int8_t  n_other;	/* unused */
	int16_t n_desc;		/* see <stab.h> */
    uint32_t n_value;	/* value of this symbol (or sdb offset) */
};
#define	n_hash	n_desc		/* used internally by ld */

/*
 * Simple values for n_type.
 */
#define	N_UNDF	0x0		/* undefined */
#define	N_ABS	0x2		/* absolute */
#define	N_TEXT	0x4		/* text */
#define	N_DATA	0x6		/* data */
#define	N_BSS	0x8		/* bss */
#define	N_COMM	0x12		/* common (internal to ld) */
#define	N_FN	0x1f		/* file name symbol */

#define	N_EXT	01		/* external bit, or'ed in */
#define	N_TYPE	0x1e		/* mask for all the type bits */

/*
 * Sdb entries have some of the N_STAB bits set.
 * These are given in <stab.h>
 */
#define	N_STAB	0xe0		/* if any of these bits set, a SDB entry */

/*
 * Format for namelist values.
 */
#define	N_FORMAT	"%08x"

#endif
