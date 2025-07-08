#include "mical.h"
#include "a.out.h"
#include "init.h"
#include <assert.h>

/*  Handle output file processing for a.out files */

FILE *tout;		/* text portion of output file */
FILE *dout;		/* data portion of output file */
FILE *rtout;		/* text relocation commands */
FILE *rdout;		/* data relocation commands */

long rtsize;		/* size of text relocation area */
long rdsize;		/* size of data relocation area */
long Sym_Write();

struct exec filhdr;	/* header for a.out files, contains sizes */

/* Initialize files for output and write out the header */

char text_file[] = "textXXXXXX";
char data_file[] = "dataXXXXXX";

Rel_Header()
{
	char rname[STR_MAX];	/* name of file for relocation commands */

	mkstemp(text_file);
	mkstemp(data_file);
	if ((tout = fopen(text_file, "w")) == NULL)
		Sys_Error("(text) open on tmp file %s failed", Rel_name);
	if ((dout = fopen(data_file, "w")) == NULL)
		Sys_Error("(data) open on tmp file %s failed", Rel_name);

	Concat(rname, Source_name, ".textr");
	rtout = fopen(rname, "w");
	Concat(rname, Source_name, ".datar");
	rdout = fopen(rname, "w");
	if (rtout == NULL || rdout == NULL)
	  Sys_Error("open on output file %s failed", rname);

	filhdr.a_magic = NMAGIC;

	fwrite(&filhdr, sizeof(filhdr), 1, tout);

	rtsize = 0;
	rdsize = 0;
}

static void
Cat(FILE *);

/* Fix_Rel -	Fix up the object file */
void
Fix_Rel()
  {	register FILE *fin, *fout;
	register int i;
	char rname[STR_MAX];	/* name of file for relocation commands */

	fclose(rtout);
	fclose(rdout);

	filhdr.a_text = tsize;
	filhdr.a_data = dsize;
	filhdr.a_bss = bsize;
	filhdr.a_entry = 0;
	/* TODO: test if a non-min header is supported in MINIX */
	filhdr.a_trsize = rtsize;
	filhdr.a_drsize = rdsize;

	Concat(rname, Source_name, ".textr");
	if ((fin = fopen(rname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rname);
	while ((i = getc(fin)) != EOF)
		putc(i,dout);
	fclose(fin);
	unlink(rname);

	Concat(rname, Source_name, ".datar");
	if ((fin = fopen(rname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rname);
	while ((i = getc(fin)) != EOF)
		putc(i,dout);
	fclose(fin);
	unlink(rname);

	filhdr.a_syms = Sym_Write(dout);

	fclose(dout);

	rewind(tout);
	fwrite(&filhdr, sizeof(filhdr), 1, tout);
	fclose(tout);

	Cat(fout);
}

void
Cat(fout)
	FILE *fout;
{
	int i;
	if ((tout = fopen(text_file, "r")) == NULL)
		Sys_Error("(text) open on input file %s failed", Rel_name);
	if ((dout = fopen(data_file, "r")) == NULL)
		Sys_Error("(text) open on input file %s failed", Rel_name);
	if ((fout = fopen(Rel_name, "w")) == NULL)
		Sys_Error("(a.out) open on output file %s failed", Rel_name);
	while ((i = getc(tout)) != EOF)
		putc(i, fout);
	while ((i = getc(dout)) != EOF)
		putc(i, fout);
	fclose(tout);
	unlink(text_file);
	fclose(dout);
	unlink(data_file);
	fclose(fout);
}
/* Put_Text -	Write out text to proper portion of file */
Put_Text(code,length)
 register char *code;
 {	if (Pass != 2) return;
	if (Cur_csect == Text_csect) fwrite(code, length, 1, tout);
	else if (Cur_csect == Data_csect) fwrite(code, length, 1, dout);
	else return;	/* ignore if bss segment */
 }

/* Pad_Text -	Write zero bytes to pad out proper portion of file */

Pad_Text(length)
 register length;
 {	register FILE *f;

	if (Pass != 2) return;
	if (Cur_csect == Text_csect) f = tout;
	else if (Cur_csect == Data_csect) f = dout;
	else return;	/* ignore if bss segment */
	while (length-- > 0)
		putc(0, f);
 }

/* set up relocation word for operand:
 *  opnd	pointer to operand structure
 *  size	0 = byte, 1 = word, 2 = long/address
 *  offset	offset into current segment
 */
Put_Rel(opnd,size,offset,pcrel)
struct oper *opnd;
int size;
long offset;
{	struct relocation_info r;

	if (opnd->sym_o == 0) return;	/* no relocation */
	if (Cur_csect == Text_csect)
		rtsize += rel_cmd(&r, opnd, size, offset, pcrel, rtout);
	else if (Cur_csect == Data_csect)
		rdsize += rel_cmd(&r, opnd, size, offset - tsize, pcrel, rdout);
	else return;	/* just ignore if bss segment */
}


/* rel_cmd -	Generate a relocation command and output */

rel_cmd(rp, opnd, size, offset, pcrel, file)
  register struct relocation_info *rp;
  struct oper *opnd;
  int size;
  long offset;
  FILE *file;
  {	int csid;			/* reloc csect identifier */
	register struct csect *csp;	/* pointer to csect of sym */
	register struct sym_bkt *sp;	/* pointer to symbol */

	sp = opnd->sym_o;
	csp = sp->csect_s;
	if (Pass == 2) {
		rp->r_address = offset;
		rp->r_pcrel = pcrel;
		rp->r_length = size;
		rp->r_extern = 0;
		if (!(sp->attr_s & S_DEF) && (sp->attr_s & S_EXT)) {
		  rp->r_extern = 1;
		  rp->r_symbolnum = sp->id_s;
		} else if (csp == Text_csect) rp->r_symbolnum = N_TEXT;
		else if (csp == Data_csect) rp->r_symbolnum = N_DATA;
		else if (csp == Bss_csect) rp->r_symbolnum = N_BSS;
		else Prog_Error(E_RELOCATE);
		fwrite(rp, sizeof *rp, 1, file);
	}
	return(sizeof *rp);
}
