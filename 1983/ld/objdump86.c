/*
 * This is a combination of three tools for decoding information from
 * Dev86/ELKS object files and executables.
 *
 * This executable can be given the names:
 *
 *  objdump86: Dumps detailed information about a binary file.
 *  size86:    Summary sizes of the data in a binary file.
 *  nm86:      The symbol table of the binary file.
 *
 * None of these programs have any options.
 * This may be a minor problem with nm86.
 *
 * Copyright (c) 1999 by Greg Haerr <greg@censoft.com>
 * Added archive file reading capabilties
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include "ar.h"
#include "obj.h"

FILE * ifd;
char * ifname;

#ifdef __STDC__
#define _(x) x
#else 
#define _(x) ()
#endif 

long get_long _((void));
long get_sized _((int sz));
unsigned int get_word _((void));
int main _((int argc, char**argv));
void do_file _((char * fname));
long read_arheader _((char *archentry));
void do_module _((char * fname, char *archive));
int error _((char * str));
int read_objheader _((void));
int read_sectheader _((void));
int read_syms _((void));
void read_databytes _((void));
void hex_output _((int ch));
void fetch_aout_hdr _((void));
void dump_aout _((void));
void size_aout _((void));
void nm_aout _((void));

int  sections;
long segsizes[16];
long textoff, textlen;
long str_off, str_len;
long filepos;

char ** symnames;
char *  symtab;

int display_mode = 0;
int multiple_files = 0;
int byte_order = 0;

long size_text, size_data, size_bss;

int
main(argc, argv)
int argc;
char ** argv;
{
   int ar;
   char * p;

   ifd = stdin;	/* In Libc6 stdin is weird */

   p = strrchr(argv[0], '/');
   if(p) p++; else p=argv[0];

   if( p[0] == 's' ) display_mode = 1;
   if( p[0] == 'n' ) display_mode = 2;

   multiple_files = 0;
   for(ar=1; ar<argc; ar++)
   {
      if( argv[ar][0] == '-' ) switch(argv[ar][1])
      {
      case 's': display_mode = 1; break;
      case 'n': display_mode = 2; break;
      }
      else
	 multiple_files++;
   }

   if( !multiple_files ) exit(1);

   multiple_files = (multiple_files>1);

   if( display_mode == 1 )
      printf("text\tdata\tbss\tdec\thex\tfilename\n");

   for(ar=1; ar<argc; ar++) if(argv[ar][0] != '-')
      do_file(argv[ar]);

   return 0;
}

void
do_file(fname)
char * fname;
{
   unsigned int magic;
   long 	filelength;
   char 	archentry[sizeof(struct ar_hdr)]; /* sizeof ar_hdr.ar_name*/
   char 	filemagic[SARMAG];

   ifname = fname;
   if( (ifd=fopen(fname, "rb")) == 0 )
   {
      error("Cannot open file");
      return;
   }
   filepos = 0L;

   /* check if file is an archive*/
   if(fread(filemagic, sizeof(filemagic), 1, ifd) == 1
     && strncmp(filemagic, ARMAG, sizeof filemagic) == 0)
   {
      filepos = SARMAG;
      while((filelength = read_arheader(archentry)) > 0) 
      {
	 filepos += sizeof(struct ar_hdr);
	 magic = get_word();
	 if(magic == 0x86A3)
	 {	/* OMAGIC*/
	    fseek(ifd, filepos, 0);
	    do_module(archentry, fname);
	 }
	 else if(magic == 0x3C21 )	/* "!<" */
	       filelength = SARMAG;
	 filepos += ld_roundup(filelength, 2, long);
	 fseek(ifd, filepos, 0);
      }
   }
   else
   {
      fseek(ifd, 0L, 0);
      do_module(fname, NULL);
   }
   fclose(ifd);
}

/* read archive header and return length */
long
read_arheader(archentry)
char *archentry;
{
   char *	  endptr;
   struct ar_hdr  arheader;

   if(fread((char *)&arheader, sizeof(arheader), 1, ifd) != 1)
      return 0;
   strncpy(archentry, arheader.ar_name, sizeof(arheader.ar_name));
   archentry[sizeof(arheader.ar_name)] = 0;
   endptr = archentry + sizeof(arheader.ar_name);
   do
   {
      *endptr-- = 0;
   } while(endptr > archentry && (*endptr == ' ' || *endptr == '/'));
   return strtoul(arheader.ar_size, (char **)NULL, 0);
}

void
do_module(fname, archive)
char * fname;
char * archive;
{
   int  modno, i, ch;

   size_text = size_data = size_bss = 0;
   byte_order = 0;

   if( !display_mode )
   {
      if(archive)
	 printf("ARCHIVEFILE '%s'\n", archive);
      printf("OBJECTFILE '%s'\n", fname);
   }

   switch( read_objheader() )
   {
   case 0: /* as86 object file */

      for(modno=1; modno<=sections; modno++)
      {
	 if( modno != 1 && !display_mode )
            printf("OBJECTSECTION %d\n", modno);
	 if( read_sectheader() < 0 ) break;

	 /* segments 0, 4-E are text, 1-3 are data*/
	 for(i=0; i<16; i++)
	 {
	    if(i < 1 || i > 4)
		 size_text += segsizes[i];
	    else size_data += segsizes[i];
	 }

	 if( read_syms() < 0 ) break;

         if( display_mode == 0 )
            printf("text\tdata\tbss\tdec\thex\tfilename\n");
         if( display_mode != 2 )
	 {
            printf("%ld\t%ld\t%ld\t%ld\t%lx\t",
               size_text, size_data, size_bss,
               size_text+ size_data+ size_bss,
               size_text+ size_data+ size_bss);

	    if(archive) printf("%s(%s)\n", archive, fname);
	    else        printf("%s\n", fname);
	 }

	 if( sections == 1 && display_mode != 0 )
	    break;

	 read_databytes();
      }
      if( !archive && !display_mode )
      {
         i=0;
         while( (ch=getc(ifd)) != EOF )
         {
            if( i == 0 )
	    {
	       printf("TRAILER\n");
	       i=1;
	    }
            hex_output(ch);
         }
         hex_output(EOF);
      }
      break;

   case 1: /* ELKS executable */
      fseek(ifd, 0L, 0);
      fetch_aout_hdr();

      switch(display_mode)
      {
      case 0: dump_aout(); break;
      case 1: size_aout(); break;
      case 2: nm_aout(); break;
      }
      break;
   }

   if( symtab ) free(symtab);
   if( symnames ) free(symnames);
   symtab = 0;
   symnames = 0;
}

int
error(str)
char * str;
{
   switch( display_mode )
   {
   case 1: fprintf(stderr, "size: %s: %s\n", ifname, str); break;
   case 2: fprintf(stderr, "nm: %s: %s\n", ifname, str); break;
   default:
      printf("Error: %s\n", str);
      break;
   }
   return -1;
}

int
read_objheader()
{
   unsigned char buf[5];

   if( fread(buf, 1, 5, ifd) != 5 )
      return error("Cannot read object header");

   if( buf[0] != 0xA3 || buf[1] != 0x86 )
   {
      if( buf[0] == 1 && buf[1] == 3 )
      {
         sections = 1;
	 return 1;
      }
      return error("Bad magic number");
   }

   if( (unsigned char)(buf[0] + buf[1] + buf[2] + buf[3]) != buf[4] )
      return error("Bad header checksum");

   sections= buf[2]+256*buf[3];

   return 0;
}

int
read_sectheader()
{
   long ssenc, cpos;
   int i, ver;

   textoff = get_long(); textlen = get_long();
   str_len=get_word(); str_off=textoff-str_len;
   ver = get_word();

   symtab = malloc((unsigned int)str_len+1);
   if( symtab == 0 ) return error("Out of memory");

   cpos = ftell(ifd);
   fseek(ifd, filepos+str_off, 0);
   fread(symtab, 1, (unsigned int)str_len, ifd);
   fseek(ifd, cpos, 0);

   if( !display_mode )
   {
      printf("MODULE  '%s'\n", symtab);
      printf("BYTEPOS %08lx\n", textoff);
      printf("BINLEN  %08lx\n", textlen);
      printf("STRINGS %04lx +%04lx\n", str_off, str_len);
      printf("VERSION %d.%d\n", ver/256, ver%256);
   }

   (void)get_long(); /* Ignore fives */
   ssenc=get_long();

   for(i=0; i<16; i++)
   {
      int ss;
      ss = (i^3);
      ss = ((ssenc>>(2*(15-ss)))&3);
      segsizes[i] = get_sized(ss);
      if( segsizes[i] && !display_mode )
	 printf("SEG%x %08lx\n", i, segsizes[i]);
   }
   if( !display_mode )
      printf("\n");

   return 0;
}

int
read_syms()
{
   int syms, i;

   syms=get_word();

   if( !display_mode ) printf("SYMS %u\n", syms);
   if( syms < 0 ) return error("Bad symbol table");

   symnames = malloc(syms*sizeof(char*)+1);
   if( symnames == 0 ) return error("Out of memory");

   if(display_mode == 2 && multiple_files)
     printf("\n%s:\n", ifname);

   for(i=0; i<syms; i++)
   {
      long offset=0;
      unsigned int nameoff, symtype;

      nameoff = get_word();
      symtype = get_word();
      offset = get_sized((symtype>>14)&3);
      symtype &= 0x3FFF;
      symnames[i] = symtab+nameoff;

      if( !display_mode )
      {
         printf("SYM %-4d %08lx ", i, offset);

         printf("%s", (symtype&0x2000)?"C":".");
         printf("%s", (symtype&0x0100)?"N":".");
         printf("%s", (symtype&0x0080)?"E":".");
         printf("%s", (symtype&0x0040)?"I":".");
         printf("%c", "T12D456789abcdeUAhijklmnopqrstuv"[symtype&0x1F]); 
         if( symtype &0x1E20 )
            printf(" %04x", symtype); 
         printf(" %s\n", symnames[i]);
      }
      if( display_mode == 2 )
      {
	 if( symtype == 0x004f || symtype == 0x0040 )
            printf("         ");
	 else
            printf("%08lx ", offset);
	 switch(symtype)
	 {
	 case 0x004F: putchar('U'); break;
	 case 0x0000: putchar('t'); break;
	 case 0x0003: putchar('d'); break;
	 case 0x2003: putchar('b'); break;
	 case 0x0043: putchar('C'); break;
	 case 0x0083: putchar('D'); break;
	 case 0x0080: putchar('T'); break;
	 case 0x0040: putchar('T'); break;
	 case 0x0180: putchar('N'); break;
	 case 0x0010: putchar('a'); break;
	 case 0x0090: putchar('A'); break;
	 default:     
	              if((symtype & ~0xF) == 0x40 )
		          putchar('u');
	              else if((symtype & ~0xF) == 0x80 )
                          printf("%c", "T12D456789abcdeU"[symtype&0xF]); 
		      else
		          putchar('?');
		      break;
	 }
         printf(" %s\n", symnames[i]);
      }

      if( symtype == 0x43 || symtype == 0x2003 )
         size_bss += offset;
   }
   if( !display_mode )
      printf("\n");

   return 0;
}

void
read_databytes()
{
static char * relstr[] = {"ERR", "DB", "DW", "DD"};
   long l;
   int ch, i;
   int curseg = 0;
   int relsize = 0;
   fseek(ifd, filepos+textoff, 0);

   printf("\nBYTECODE\n");
   for(;;)
   {
      if( (ch=getc(ifd)) == EOF ) break;

      if( ch == 0 ) break;

      switch( ch & 0xC0 )
      {
      case 0x00:  switch(ch & 0xF0)
                  {
                  case 0x00: /* Relocator size */
	                     printf("RELSZ %d\n", relsize= (ch&0xF));
			     if(relsize>3) relsize=3;
		             break;
                  case 0x10: /* Skip bytes */
	                     printf("SKP %ld\n", get_sized(ch&0xF));
	                     break;
                  case 0x20: /* Segment */
	                     printf("SEG %x\n", curseg= (ch&0xF));
		             break;
                  default:   printf("CODE %02x - unknown\n", ch);
                             return ;
                  }
		  break;
      case 0x40:  /* Raw bytes */
                  {
                     int abscnt = (ch&0x3F);
		     if( abscnt == 0 ) abscnt = 64;
	             for( i=0; i<abscnt; i++ )
	             {
                        if( (ch=getc(ifd)) == EOF ) break;
	                hex_output(ch);
	             }
	             hex_output(EOF);
            
	             if( ch == EOF ) return;
                  }
                  break;
      case 0x80:  /* Relocator - simple */
                  l = get_sized(relsize);
		  printf("%s SEG%x%s%s", relstr[relsize],
		         (ch&0xF),
		         (ch&0x20)?"-PC":"",
		         (ch&0x10)?"+?":"");
		  if(l)
		     printf("+0x%04lx", l);
		  putchar('\n');
                  break;
      case 0xC0:  /* Relocator - symbol relative */
                  if( ch & 0x18 )
		  {
		     printf("CODE %02x - unknown\n", ch);
		     return;
		  }
                  if( ch & 4 ) i = get_word();
                  else         i = getc(ifd);
		  l = get_sized(ch&3);

		  printf("%s %s%s%s", relstr[relsize],
		         symnames[i],
		         (ch&0x20)?"-PC":"",
		         (ch&0x18)?"+?":"");
		  if(l)
		     printf("+0x%04lx", l);
		  putchar('\n');
                  break;
      }
   }
   printf("\n");
}

long
get_sized(sz)
int sz;
{
   switch(sz)
   {
   case 0: return 0;
   case 1: return getc(ifd);
   case 2: return get_word();
   case 3: return get_long();
   }
   return -1;
}

long
get_long()
{
   long retv = 0;
   int i;
   for(i=0; i<32; i+=16)
   {
      unsigned int v = get_word();
      if( byte_order & 2 )
         retv += ((long)v<<(16-i));
      else
         retv += ((long)v<<i);
   }
   return retv;
}

unsigned int
get_word()
{
   long retv = 0;
   int i;
   for(i=0; i<16; i+=8)
   {
      int v = getc(ifd);
      if( v == EOF ) return -1;
      if( byte_order & 1 )
         retv += (v<<(8-i));
      else
         retv += (v<<i);
   }
   return retv;
}

void
hex_output(ch)
int ch;
{
static char linebuf[80];
static char buf[20];
static int pos = 0;

   if( ch == EOF )
   {
      if(pos)
	 printf(": %.66s\n", linebuf);
      pos = 0;
   }
   else
   {
      if(!pos)
         memset(linebuf, ' ', sizeof(linebuf));
      sprintf(buf, "%02x", ch&0xFF);
      memcpy(linebuf+pos*3+(pos>7), buf, 2);
      if( ch > ' ' && ch <= '~' ) linebuf[50+pos] = ch;
      else  linebuf[50+pos] = '.';
      pos = ((pos+1) & 0xF);
      if( pos == 0 )
      {
         printf(": %.66s\n", linebuf);
         memset(linebuf, ' ', sizeof(linebuf));
      }
   }
}

/************************************************************************/
/* ELKS a.out versions
 */

long header[12];
int  h_len, h_flgs, h_cpu;

void
fetch_aout_hdr()
{
   int i;

   header[0] = get_long();
   header[1] = get_long();
   byte_order = ((header[0]>>24) & 3);

   h_len  = (header[1] & 0xFF);
   h_flgs = ((header[0]>>16) & 0xFF);
   h_cpu  = ((header[0]>>24) & 0xFF);

   for(i=2; i<8; i++)
   {
      if( i*4 <= h_len )
         header[i] = get_long();
      else
         header[i] = 0;
   }
}

void
dump_aout()
{
static char * cpu[] = { "unknown", "8086", "m68k", "ns16k", "i386", "sparc" };
static char * byteord[] = { "LITTLE_ENDIAN", "(2143)","(3412)","BIG_ENDIAN" };
   int i;
   long l;

   if( h_cpu > 0x17 ) h_cpu &= 3;

   printf("HLEN %d\n", h_len);
   printf("CPU  %s %s\n", cpu[h_cpu>>2], byteord[h_cpu&3]);

   printf("FLAGS:");
   if( h_flgs & 0x01 ) printf(" A_UZP");
   if( h_flgs & 0x02 ) printf(" A_PAL");
   if( h_flgs & 0x04 ) printf(" A_NSYM");
   if( h_flgs & 0x08 ) printf(" FLG-08");
   if( h_flgs & 0x10 ) printf(" A_EXEC");
   if( h_flgs & 0x20 ) printf(" A_SEP");
   if( h_flgs & 0x40 ) printf(" A_PURE");
   if( h_flgs & 0x80 ) printf(" A_TOVLY");
   printf("\n");

   if( header[5] )
      printf("a_entry  = 0x%08lx\n", header[5]);
   printf("a_total  = 0x%08lx\n", header[6]);
   if( header[7] )
      printf("a_syms   = 0x%08lx\n", header[7]);

   if( h_len >= 36 )
      printf("a_trsize = 0x%08lx\n", header[8]);
   if( h_len >= 40 )
      printf("a_drsize = 0x%08lx\n", header[9]);
   if( h_len >= 44 )
      printf("a_tbase  = 0x%08lx\n", header[10]);
   if( h_len >= 48 )
      printf("a_dbase  = 0x%08lx\n", header[11]);
   printf("\n");

   size_aout();
   printf("\n");

   if( header[7] )
   {
      printf("SYMBOLS\n");
      nm_aout();
   }
   else
      printf("NO SYMBOLS\n");
   printf("\n");

   printf("TEXTSEG\n");
   fseek(ifd, (long)h_len, 0);
   for(l=0; l<header[2]; l++)
   {
      if( (i=getc(ifd)) == EOF ) break;
      hex_output(i);
   }
   hex_output(EOF);

   printf("DATASEG\n");
   fseek(ifd, (long)h_len+header[2], 0);
   for(l=0; l<header[3]; l++)
   {
      if( (i=getc(ifd)) == EOF ) break;
      hex_output(i);
   }
   hex_output(EOF);
}

void
size_aout()
{
   if( display_mode == 0 )
      printf("text\tdata\tbss\tdec\thex\tfilename\n");

   printf("%ld\t%ld\t%ld\t%ld\t%lx\t%s\n",
      header[2], header[3], header[4],
      header[2]+ header[3]+ header[4],
      header[2]+ header[3]+ header[4],
      ifname);
}

void
nm_aout()
{
   char n_name[10];
   long n_value;
   int n_sclass, n_numaux, n_type;
   long bytes_left;
   int  pending_nl = 0;

   fseek(ifd, h_len+header[2]+header[3]+header[8]+header[9], 0);

   if( h_flgs & 4 ) { error("Executable has new format symtab\n"); return; }

   bytes_left = header[7];

   if( bytes_left == 0 )
      printf("No symbols in '%s'\n", ifname);
   else if(multiple_files)
     printf("\n%s:\n", ifname);

   while(bytes_left > 16)
   {
      if( fread(n_name, 1, 8, ifd) != 8 ) break;
      n_name[8] = 0;
      n_value  = get_long();
      if( (n_sclass = getc(ifd)) == EOF ) break;
      if( (n_numaux = getc(ifd)) == EOF ) break;
      n_type = get_word();

      if( pending_nl && n_sclass == 0 )
      {
         printf("%s", n_name);
         continue;
      }

      if( pending_nl ) putchar('\n');
      if( n_sclass == 0x10 )
         printf("         ");
      else
         printf("%08lx ", n_value);
      switch(n_sclass)
      {
      case 0x01: printf("a "); break;      
      case 0x12: printf("T "); break;
      case 0x13: printf("D "); break;
      case 0x14: printf("C "); break;
      case 0x1a: printf("t "); break;
      case 0x1b: printf("d "); break;
      case 0x1c: printf("b "); break;
      case 0x10: printf("U "); break;
      default:   if( display_mode )
                 {
		    printf("? "); break;
                 }

                 printf("n_sclass=");
                 switch(n_sclass>>3)
		 {
		 case 0: printf("C_NULL,"); break;
		 case 2: printf("C_EXT,"); break;
		 case 3: printf("C_STAT,"); break;
		 default: printf("%04o,", n_sclass&0xF8);
		 }
                 switch(n_sclass&7)
		 {
		 case 0: printf("N_UNDF "); break;
		 case 1: printf("N_ABS "); break;
		 case 2: printf("N_TEXT "); break;
		 case 3: printf("N_DATA "); break;
		 case 4: printf("N_BSS "); break;
		 case 5: printf("N_COMM "); break;
		 default: printf("%o ", n_sclass&7); break;
		 }
		 break;
      }

      if( display_mode == 0 )
      {
	 if( n_numaux )
            printf("n_numaux=%02x ", n_numaux);
	 if( n_type )
            printf("n_type=%04x ", n_type);
      }

      printf("%s", n_name);

      pending_nl=1;
   }

   if( pending_nl ) putchar('\n');
}
