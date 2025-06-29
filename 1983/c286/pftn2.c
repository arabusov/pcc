# include "mfile1"
#include "code.h"
#include "common.h"
#include "local.h"
#include "scan.h"
#include "trees2.h"
#include "pftn1.h"
#include "gram.h"

int instk();
int ilbrace();
int irbrace();
int gotscal();
int tyreduce();
int unhide();
int movestab();

extern unsigned int offsz;

extern struct instk instack[10], *pstk;

	/* defines used for getting things off of the initialization stack */


struct symtab *relook();

extern int ddebug;

struct symtab * mknonuniq();
talign( ty, s) register unsigned ty; register s; {
	/* compute the alignment of an object with type ty, sizeoff index s */

	register i;
	if( s<0 && ty!=INT && ty!=CHAR && ty!=SHORT && ty!=UNSIGNED && ty!=UCHAR && ty!=USHORT 
#ifdef LONGFIELDS
		&& ty!=LONG && ty!=ULONG
#endif
					){
		return( fldal( ty ) );
		}

	for( i=0; i<=(SZINT-BTSHIFT-1); i+=TSHIFT ){
		switch( (ty>>i)&TMASK ){

		case FTN:
			cerror( "compiler takes alignment of function");
		case PTR:
			return( ALPOINT );
		case ARY:
			continue;
		case 0:
			break;
			}
		}

	switch( BTYPE(ty) ){

	case UNIONTY:
	case ENUMTY:
	case STRTY:
		return( (unsigned int) dimtab[ s+2 ] );
	case CHAR:
	case UCHAR:
		return( ALCHAR );
	case FLOAT:
		return( ALFLOAT );
	case DOUBLE:
		return( ALDOUBLE );
	case LONG:
	case ULONG:
		return( ALLONG );
	case SHORT:
	case USHORT:
		return( ALSHORT );
	default:
		return( ALINT );
		}
	}

OFFSZ
tsize( ty, d, s )  TWORD ty; {
	/* compute the size associated with type ty,
	    dimoff d, and sizoff s */
	/* BETTER NOT BE CALLED WHEN t, d, and s REFER TO A BIT FIELD... */

	int i;
	OFFSZ mult;

	mult = 1;

	for( i=0; i<=(SZINT-BTSHIFT-1); i+=TSHIFT ){
		switch( (ty>>i)&TMASK ){

		case FTN:
			cerror( "compiler takes size of function");
		case PTR:
			return( SZPOINT * mult );
		case ARY:
			mult *= (unsigned int) dimtab[ d++ ];
			continue;
		case 0:
			break;

			}
		}

	if( dimtab[s]==0 ) {
		uerror( "unknown size");
		return( SZINT );
		}
	return( (unsigned int) dimtab[ s ] * mult );
	}

inforce( n ) OFFSZ n; {  /* force inoff to have the value n */
	/* inoff is updated to have the value n */
	OFFSZ wb;
	register rest;
	/* rest is used to do a lot of conversion to ints... */

	if( inoff == n ) return(0);
	if( inoff > n ) {
		cerror( "initialization alignment error");
		}

	wb = inoff;
	SETOFF( wb, SZINT );

	/* wb now has the next higher word boundary */

	if( wb >= n ){ /* in the same word */
		rest = n - inoff;
		vfdzero( rest );
		return(0);
		}

	/* otherwise, extend inoff to be word aligned */

	rest = wb - inoff;
	vfdzero( rest );

	/* now, skip full words until near to n */

	rest = (n-inoff)/SZINT;
	zecode( rest );

	/* now, the remainder of the last word */

	rest = n-inoff;
	vfdzero( rest );
	if( inoff != n ) cerror( "inoff error");

	}

vfdalign( n ){ /* make inoff have the offset the next alignment of n */
	OFFSZ m;

	m = inoff;
	SETOFF( m, n );
	inforce( m );
	}


int idebug = 0;

int ibseen = 0;  /* the number of } constructions which have been filled */

int iclass;  /* storage class of thing being initialized */

int ilocctr = 0;  /* location counter for current initialization */

beginit(curid){
	/* beginning of initilization; set location ctr and set type */
	register struct symtab *p;

# ifndef BUG1
	if( idebug >= 3 ) printf( "beginit(), curid = %d\n", curid );
# endif

	p = &stab[curid];

	iclass = p->sclass;
	if( curclass == EXTERN || curclass == FORTRAN ) iclass = EXTERN;
	switch( iclass ){

	case UNAME:
	case EXTERN:
		return(0);
	case AUTO:
	case REGISTER:
		break;
	case EXTDEF:
	case STATIC:
		ilocctr = ISARY(p->stype)?ADATA:DATA;
		locctr( ilocctr );
		defalign( talign( p->stype, p->sizoff ) );
		defnam( p );

		}

	inoff = 0;
	ibseen = 0;

	pstk = 0;

	instk( curid, p->stype, p->dimoff, p->sizoff, inoff );

	}

instk( id, t, d, s, off ) OFFSZ off; TWORD t; {
	/* make a new entry on the parameter stack to initialize id */

	register struct symtab *p;

	for(;;){
# ifndef BUG1
		if( idebug ) printf( "instk((%d, %o,%d,%d, %d)\n", id, t, d, s, off );
# endif

		/* save information on the stack */

		if( !pstk ) pstk = instack;
		else ++pstk;

		pstk->in_fl = 0;	/* { flag */
		pstk->in_id =  id ;
		pstk->in_t =  t ;
		pstk->in_d =  d ;
		pstk->in_s =  s ;
		pstk->in_n = 0;  /* number seen */
		pstk->in_x =  t==STRTY ?dimtab[s+1] : 0 ;
		pstk->in_off =  off;   /* offset at the beginning of this element */
		/* if t is an array, DECREF(t) can't be a field */
		/* INS_sz has size of array elements, and -size for fields */
		if( ISARY(t) ){
			pstk->in_sz = tsize( DECREF(t), d+1, s );
			}
		else if( stab[id].sclass & FIELD ){
			pstk->in_sz = - ( stab[id].sclass & FLDSIZ );
			}
		else {
			pstk->in_sz = 0;
			}

		if( (iclass==AUTO || iclass == REGISTER ) &&
			(ISARY(t) || t==STRTY) ) uerror( "no automatic aggregate initialization" );

		/* now, if this is not a scalar, put on another element */

		if( ISARY(t) ){
			t = DECREF(t);
			++d;
			continue;
			}
		else if( t == STRTY ){
			id = dimtab[pstk->in_x];
			p = &stab[id];
			if( p->sclass != MOS && !(p->sclass&FIELD) ) cerror( "insane structure member list" );
			t = p->stype;
			d = p->dimoff;
			s = p->sizoff;
			off += p->offset;
			continue;
			}
		else return(0);
		}
	}

NODE *
getstr(){ /* decide if the string is external or an initializer, and get the contents accordingly */

	register l, temp;
	register NODE *p;

	if( (iclass==EXTDEF||iclass==STATIC) && (pstk->in_t == CHAR || pstk->in_t == UCHAR) &&
			pstk!=instack && ISARY( pstk[-1].in_t ) ){
		/* treat "abc" as { 'a', 'b', 'c', 0 } */
		strflg = 1;
		ilbrace();  /* simulate { */
		inforce( pstk->in_off );
		/* if the array is inflexible (not top level), pass in the size and
			be prepared to throw away unwanted initializers */
		lxstr((pstk-1)!=instack?dimtab[(pstk-1)->in_d]:0);  /* get the contents */
		irbrace();  /* simulate } */
		return( NIL );
		}
	else { /* make a label, and get the contents and stash them away */
		if( iclass != SNULL ){ /* initializing */
			/* fill out previous word, to permit pointer */
			vfdalign( ALPOINT );
			}
		temp = locctr( blevel==0?ISTRNG:STRNG ); /* set up location counter */
		deflab( l = getlab() );
		strflg = 0;
		lxstr(0); /* get the contents */
		locctr( blevel==0?ilocctr:temp );
		p = buildtree( STRING, NIL, NIL );
		p->tn.rval = -l;
		return(p);
		}
	}

putbyte( v ){ /* simulate byte v appearing in a list of integer values */
	register NODE *p;
	p = bcon(v);
	incode( p, SZCHAR );
	tfree( p );
	gotscal();
	}

endinit(){
	register TWORD t;
	register d, s, n, d1;

# ifndef BUG1
	if( idebug ) printf( "endinit(), inoff = %d\n", inoff );
# endif

	switch( iclass ){

	case EXTERN:
	case AUTO:
	case REGISTER:
		return(0);
		}

	pstk = instack;

	t = pstk->in_t;
	d = pstk->in_d;
	s = pstk->in_s;
	n = pstk->in_n;

	if( ISARY(t) ){
		d1 = dimtab[d];

		vfdalign( pstk->in_sz );  /* fill out part of the last element, if needed */
		n = inoff/pstk->in_sz;  /* real number of initializers */
		if( d1 >= n ){
			/* once again, t is an array, so no fields */
			inforce( tsize( t, d, s ) );
			n = d1;
			}
		if( d1!=0 && d1!=n ) uerror( "too many initializers");
		if( n==0 ) werror( "empty array declaration");
		dimtab[d] = n;
		}

	else if( t == STRTY || t == UNIONTY ){
		/* clearly not fields either */
		inforce( tsize( t, d, s ) );
		}
	else if( n > 1 ) uerror( "bad scalar initialization");
	/* this will never be called with a field element... */
	else inforce( tsize(t,d,s) );

	paramno = 0;
	vfdalign( AL_INIT );
	inoff = 0;
	iclass = SNULL;

	}

doinit( p ) register NODE *p; {

	/* take care of generating a value for the initializer p */
	/* inoff has the current offset (last bit written)
		in the current word being generated */

	register sz, d, s;
	register TWORD t;

	/* note: size of an individual initializer is assumed to fit into an int */

	if( iclass < 0 ) goto leave;
	if( iclass == EXTERN || iclass == UNAME ){
		uerror( "cannot initialize extern or union" );
		iclass = -1;
		goto leave;
		}

	if( iclass == AUTO || iclass == REGISTER ){
		/* do the initialization and get out, without regard 
		    for filing out the variable with zeros, etc. */
		bccode();
		idname = pstk->in_id;
		p = buildtree( ASSIGN, buildtree( NAME, NIL, NIL ), p );
		ecomp(p);
		return(0);
		}

	if( p == NIL ) return(0);  /* for throwing away strings that have been turned into lists */

	if( ibseen ){
		uerror( "} expected");
		goto leave;
		}

# ifndef BUG1
	if( idebug > 1 ) printf( "doinit(%o)\n", p );
# endif

	t = pstk->in_t;  /* type required */
	d = pstk->in_d;
	s = pstk->in_s;
	if( pstk->in_sz < 0 ){  /* bit field */
		sz = -pstk->in_sz;
		}
	else {
		sz = tsize( t, d, s );
		}

	inforce( pstk->in_off );

	p = buildtree( ASSIGN, block( NAME, NIL,NIL, t, d, s ), p );
	p->in.left->in.op = FREE;
	p->in.left = p->in.right;
	p->in.right = NIL;
	p->in.left = optim( p->in.left );
	if( p->in.left->in.op == UNARY AND ){
		p->in.left->in.op = FREE;
		p->in.left = p->in.left->in.left;
		}
	p->in.op = INIT;

	if( sz < SZINT ){ /* special case: bit fields, etc. */
		if( p->in.left->in.op != ICON ) uerror( "illegal initialization" );
		else incode( p->in.left, sz );
		}
	else if( p->in.left->in.op == FCON ){
		fincode( p->in.left->fpn.dval, sz );
		}
	else {
		cinit( optim(p), sz );
		}

	gotscal();

	leave:
	tfree(p);
	}

gotscal(){
	register t, ix;
	register n, id;
	struct symtab *p;
	OFFSZ temp;

	for( ; pstk > instack; ) {

		if( pstk->in_fl ) ++ibseen;

		--pstk;
		
		t = pstk->in_t;

		if( t == STRTY ){
			ix = ++pstk->in_x;
			if( (id=dimtab[ix]) < 0 ) continue;

			/* otherwise, put next element on the stack */

			p = &stab[id];
			instk( id, p->stype, p->dimoff, p->sizoff, p->offset+pstk->in_off );
			return(0);
			}
		else if( ISARY(t) ){
			n = ++pstk->in_n;
			if( n >= dimtab[pstk->in_d] && pstk > instack ) continue;

			/* put the new element onto the stack */

			temp = pstk->in_sz;
			instk( pstk->in_id, (TWORD)DECREF(pstk->in_t), pstk->in_d+1, pstk->in_s,
				pstk->in_off+n*temp );
			return(0);
			}

		}

	}

ilbrace(){ /* process an initializer's left brace */
	register t;
	struct instk *temp;

	temp = pstk;

	for( ; pstk > instack; --pstk ){

		t = pstk->in_t;
		if( t != STRTY && !ISARY(t) ) continue; /* not an aggregate */
		if( pstk->in_fl ){ /* already associated with a { */
			if( pstk->in_n ) uerror( "illegal {");
			continue;
			}

		/* we have one ... */
		pstk->in_fl = 1;
		break;
		}

	/* cannot find one */
	/* ignore such right braces */

	pstk = temp;
	}

irbrace(){
	/* called when a '}' is seen */

# ifndef BUG1
	if( idebug ) printf( "irbrace(): paramno = %d on entry\n", paramno );
# endif

	if( ibseen ) {
		--ibseen;
		return(0);
		}

	for( ; pstk > instack; --pstk ){
		if( !pstk->in_fl ) continue;

		/* we have one now */

		pstk->in_fl = 0;  /* cancel { */
		gotscal();  /* take it away... */
		return(0);
		}

	/* these right braces match ignored left braces: throw out */

	}

upoff( size, alignment, poff ) register alignment, *poff; {
	/* update the offset pointed to by poff; return the
	/* offset of a value of size `size', alignment `alignment',
	/* given that off is increasing */

	register off;

	off = *poff;
	SETOFF( off, alignment );
	if( (offsz-off) <=  size ){
		if( instruct!=INSTRUCT )cerror("too many local variables");
		else cerror("Structure too large");
		}
	*poff = off+size;
	return( off );
	}

oalloc( p, poff ) register struct symtab *p; register *poff; {
	/* allocate p with offset *poff, and update *poff */
	register al, off, tsz;
	int noff;

	al = talign( p->stype, p->sizoff );
	noff = off = *poff;
	tsz = tsize( p->stype, p->dimoff, p->sizoff );
#ifdef BACKAUTO
	if( p->sclass == AUTO ){
		if( (offsz-off) <= tsz ) cerror("too many local variables");
		noff = off + tsz;
		SETOFF( noff, al );
		off = -noff;
		}
	else
#endif
		if( p->sclass == PARAM && ( tsz < SZINT ) ){
			off = upoff( SZINT, ALINT, &noff );
# ifndef RTOLBYTES
			off = noff - tsz;
#endif
			}
		else
		{
		off = upoff( tsz, al, &noff );
		}

	if( p->sclass != REGISTER ){ /* in case we are allocating stack space for register arguments */
		if( p->offset == NOOFFSET ) p->offset = off;
		else if( off != p->offset ) return(1);
		}

	*poff = noff;
	return(0);
	}

falloc( p, w, new, pty )  register struct symtab *p; NODE *pty; {
	/* allocate a field of width w */
	/* new is 0 if new entry, 1 if redefinition, -1 if alignment */

	register al,sz,type;

	type = (new<0)? pty->in.type : p->stype;

	/* this must be fixed to use the current type in alignments */
	switch( new<0?pty->in.type:p->stype ){

	case ENUMTY:
		{
			int s;
			s = new<0 ? pty->fn.csiz : p->sizoff;
			al = dimtab[s+2];
			sz = dimtab[s];
			break;
			}

	case CHAR:
	case UCHAR:
		al = ALCHAR;
		sz = SZCHAR;
		break;

	case SHORT:
	case USHORT:
		al = ALSHORT;
		sz = SZSHORT;
		break;

	case INT:
	case UNSIGNED:
		al = ALINT;
		sz = SZINT;
		break;
#ifdef LONGFIELDS

	case LONG:
	case ULONG:
		al = ALLONG;
		sz = SZLONG;
		break;
#endif

	default:
		if( new < 0 ) {
			uerror( "illegal field type" );
			al = ALINT;
			}
		else {
			al = fldal( p->stype );
			sz =SZINT;
			}
		}

	if( w > sz ) {
		uerror( "field too big");
		w = sz;
		}

	if( w == 0 ){ /* align only */
		SETOFF( strucoff, al );
		if( new >= 0 ) uerror( "zero size field");
		return(0);
		}

	if( strucoff%al + w > sz ) SETOFF( strucoff, al );
	if( new < 0 ) {
		if( (offsz-strucoff) <= w )
			cerror("structure too large");
		strucoff += w;  /* we know it will fit */
		return(0);
		}

	/* establish the field */

	if( new == 1 ) { /* previous definition */
		if( p->offset != strucoff || p->sclass != (FIELD|w) ) return(1);
		}
	p->offset = strucoff;
	if( (offsz-strucoff) <= w ) cerror("structure too large");
	strucoff += w;
	p->stype = type;
	fldty( p );
	return(0);
	}

nidcl( p ) NODE *p; { /* handle unitialized declarations */
	/* assumed to be not functions */
	register class;
	register commflag;  /* flag for labelled common declarations */

	commflag = 0;

	/* compute class */
	if( (class=curclass) == SNULL ){
		if( blevel > 1 ) class = AUTO;
		else if( blevel != 0 || instruct ) cerror( "nidcl error" );
		else { /* blevel = 0 */
			class = noinit();
			if( class == EXTERN ) commflag = 1;
			}
		}

	defid( p, class );

	if( class==EXTDEF || class==STATIC ){
		/* simulate initialization by 0 */
		beginit(p->tn.rval);
		endinit();
		}
	if( commflag ) commdec( p->tn.rval );
	}

TWORD
types( t1, t2, t3 ) TWORD t1, t2, t3; {
	/* return a basic type from basic types t1, t2, and t3 */

	TWORD t[3], noun, adj, unsg;
	register i;

	t[0] = t1;
	t[1] = t2;
	t[2] = t3;

	unsg = INT;  /* INT or UNSIGNED */
	noun = UNDEF;  /* INT, CHAR, or FLOAT */
	adj = INT;  /* INT, LONG, or SHORT */

	for( i=0; i<3; ++i ){
		switch( t[i] ){

		default:
		bad:
			uerror( "illegal type combination" );
			return( INT );

		case UNDEF:
			continue;

		case UNSIGNED:
			if( unsg != INT ) goto bad;
			unsg = UNSIGNED;
			continue;

		case LONG:
		case SHORT:
			if( adj != INT ) goto bad;
			adj = t[i];
			continue;

		case INT:
		case CHAR:
		case FLOAT:
			if( noun != UNDEF ) goto bad;
			noun = t[i];
			continue;
			}
		}

	/* now, construct final type */
	if( noun == UNDEF ) noun = INT;
	else if( noun == FLOAT ){
		if( unsg != INT || adj == SHORT ) goto bad;
		return( adj==LONG ? DOUBLE : FLOAT );
		}
	else if( noun == CHAR && adj != INT ) goto bad;

	/* now, noun is INT or CHAR */
	if( adj != INT ) noun = adj;
	if( unsg == UNSIGNED ) return( noun + (UNSIGNED-INT) );
	else return( noun );
	}

NODE *
tymerge( typ, idp ) NODE *typ, *idp; {
	/* merge type typ with identifier idp  */

	register unsigned t;
	register i;
	extern int eprint();

	if( typ->in.op != TYPE ) cerror( "tymerge: arg 1" );
	if(idp == NIL ) return( NIL );

# ifndef BUG1
	if( ddebug > 2 ) fwalk( idp, eprint, 0 );
# endif

	idp->in.type = typ->in.type;
	idp->fn.cdim = curdim;
	tyreduce( idp );
	idp->fn.csiz = typ->fn.csiz;

	for( t=typ->in.type, i=typ->fn.cdim; t&TMASK; t = DECREF(t) ){
		if( ISARY(t) ) dstash( dimtab[i++] );
		}

	/* now idp is a single node: fix up type */

	idp->in.type = ctype( idp->in.type );

	if( (t = BTYPE(idp->in.type)) != STRTY && t != UNIONTY && t != ENUMTY ){
		idp->fn.csiz = t;  /* in case ctype has rewritten things */
		}

	return( idp );
	}

tyreduce( p ) register NODE *p; {

	/* build a type, and stash away dimensions, from a parse tree of the declaration */
	/* the type is build top down, the dimensions bottom up */
	register o, temp;
	register unsigned t;

	o = p->in.op;
	p->in.op = FREE;

	if( o == NAME ) return(0);

	t = INCREF( p->in.type );
	if( o == UNARY CALL ) t += (FTN-PTR);
	else if( o == LB ){
		t += (ARY-PTR);
		temp = p->in.right->tn.lval;
		p->in.right->in.op = FREE;
		}

	p->in.left->in.type = t;
	tyreduce( p->in.left );

	if( o == LB ) dstash( temp );

	p->tn.rval = p->in.left->tn.rval;
	p->in.type = p->in.left->in.type;

	}

fixtype( p, class ) register NODE *p; {
	register unsigned t, type;
	register mod1, mod2;
	/* fix up the types, and check for legality */

	if( (type = p->in.type) == UNDEF ) return(0);
	if( mod2 = (type&TMASK) ){
		t = DECREF(type);
		while( mod1=mod2, mod2 = (t&TMASK) ){
			if( mod1 == ARY && mod2 == FTN ){
				uerror( "array of functions is illegal" );
				type = 0;
				}
			else if( mod1 == FTN && ( mod2 == ARY || mod2 == FTN ) ){
				uerror( "function returns illegal type" );
				type = 0;
				}
			t = DECREF(t);
			}
		}

	/* detect function arguments, watching out for structure declarations */
	/* for example, beware of f(x) struct [ int a[10]; } *x; { ... } */
	/* the danger is that "a" will be converted to a pointer */

	if( class==SNULL && blevel==1 && !(instruct&(INSTRUCT|INUNION)) ) class = PARAM;
	if( class == PARAM || ( class==REGISTER && blevel==1 ) ){
		if( type == FLOAT ) type = DOUBLE;
		else if( ISARY(type) ){
			++p->fn.cdim;
			type += (PTR-ARY);
			}
		else if( ISFTN(type) ){
			werror( "a function is declared as an argument" );
			type = INCREF(type);
			}

		}

	if( instruct && ISFTN(type) ){
		uerror( "function illegal in structure or union" );
		type = INCREF(type);
		}
	p->in.type = type;
	}

uclass( class ) register class; {
	/* give undefined version of class */
	if( class == SNULL ) return( EXTERN );
	else if( class == STATIC ) return( USTATIC );
	else if( class == FORTRAN ) return( UFORTRAN );
	else return( class );
	}

fixclass( class, type ) TWORD type; {

	/* first, fix null class */

	if( class == SNULL ){
		if( instruct&INSTRUCT ) class = MOS;
		else if( instruct&INUNION ) class = MOU;
		else if( blevel == 0 ) class = EXTDEF;
		else if( blevel == 1 ) class = PARAM;
		else class = AUTO;

		}

	/* now, do general checking */

	if( ISFTN( type ) ){
		switch( class ) {
		default:
			uerror( "function has illegal storage class" );
		case AUTO:
			class = EXTERN;
		case EXTERN:
		case EXTDEF:
		case FORTRAN:
		case TYPEDEF:
		case STATIC:
		case UFORTRAN:
		case USTATIC:
			;
			}
		}

	if( class&FIELD ){
		if( !(instruct&INSTRUCT) ) uerror( "illegal use of field" );
		return( class );
		}

	switch( class ){

	case MOU:
		if( !(instruct&INUNION) ) uerror( "illegal class" );
		return( class );

	case MOS:
		if( !(instruct&INSTRUCT) ) uerror( "illegal class" );
		return( class );

	case MOE:
		if( instruct & (INSTRUCT|INUNION) ) uerror( "illegal class" );
		return( class );

	case REGISTER:
		if( blevel == 0 ) uerror( "illegal register declaration" );
		else if( regvar >= MINRVAR && cisreg( type ) ) return( class );
		if( blevel == 1 ) return( PARAM );
		else return( AUTO );

	case AUTO:
	case LABEL:
	case ULABEL:
		if( blevel < 2 ) uerror( "illegal class" );
		return( class );

	case PARAM:
		if( blevel != 1 ) uerror( "illegal class" );
		return( class );

	case UFORTRAN:
	case FORTRAN:
# ifdef NOFORTRAN
			NOFORTRAN;    /* a condition which can regulate the FORTRAN usage */
# endif
		if( !ISFTN(type) ) uerror( "fortran declaration must apply to function" );
		else {
			type = DECREF(type);
			if( ISFTN(type) || ISARY(type) || ISPTR(type) ) {
				uerror( "fortran function has wrong type" );
				}
			}
	case STNAME:
	case UNAME:
	case ENAME:
	case EXTERN:
	case STATIC:
	case EXTDEF:
	case TYPEDEF:
	case USTATIC:
		return( class );

	default:
		cerror( "illegal class: %d", class );
		/* NOTREACHED */

		}
	}

struct symtab *
mknonuniq(idindex) int *idindex; {/* locate a symbol table entry for */
	/* an occurrence of a nonunique structure member name */
	/* or field */
	register i;
	register struct symtab * sp;
	char *p,*q;

	sp = & stab[ i= *idindex ]; /* position search at old entry */
	while( sp->stype != TNULL ){ /* locate unused entry */
		if( ++i >= SYMTSZ ){/* wrap around symbol table */
			i = 0;
			sp = stab;
			}
		else ++sp;
		if( i == *idindex ) cerror("Symbol table full");
		}
	sp->sflags = SNONUNIQ | SMOS;
	p = sp->sname;
	q = stab[*idindex].sname; /* old entry name */
# ifndef BUG1
	if( ddebug ){
		printf("\tnonunique entry for %s from %d to %d\n",
			q, *idindex, i );
		}
# endif
	*idindex = i;
	for( i=1; i<=NCHNAM; ++i ){ /* copy name */
		if( *p++ = *q /* assign */ ) ++q;
		}
	return ( sp );
	}

lookup( name, s) char *name; { 
	/* look up name: must agree with s w.r.t. STAG, SMOS and SHIDDEN */

	register char *p, *q;
	int i, j, ii;
	register struct symtab *sp;

	/* compute initial hash index */
# ifndef BUG1
	if( ddebug > 2 ){
		printf( "lookup( %.8s, %d ), stwart=%d, instruct=%d\n", name, s, stwart, instruct );
		}
# endif

	i = 0;
	for( p=name, j=0; *p != '\0'; ++p ){
		i = (i<<1)+*p;
		if( ++j >= NCHNAM ) break;
		}
	/* this will not have overflowed, since NCHNAM is small relative to
	/* the wordsize... */
	i = i%SYMTSZ;
	sp = &stab[ii=i];

	for(;;){ /* look for name */

		if( sp->stype == TNULL ){ /* empty slot */
			p = sp->sname;
			sp->sflags = s;  /* set STAG, SMOS if needed, turn off all others */
			for( j=0; j<NCHNAM; ++j ) if( *p++ = *name ) ++name;
			sp->stype = UNDEF;
			sp->sclass = SNULL;
			return( i );
			}
		if( (sp->sflags & (STAG|SMOS|SHIDDEN)) != s ) goto next;
		p = sp->sname;
		q = name;
		for( j=0; j<NCHNAM;++j ){
			if( *p++ != *q ) goto next;
			if( !*q++ ) break;
			}
		return( i );
	next:
		if( ++i >= SYMTSZ ){
			i = 0;
			sp = stab;
			}
		else ++sp;
		if( i == ii ) cerror( "symbol table full 1" );
		}
	}

#ifndef checkst
/* if not debugging, make checkst a macro */
checkst(lev){
	register int s, i, j;
	register struct symtab *p, *q;

	for( i=0, p=stab; i<SYMTSZ; ++i, ++p ){
		if( p->stype == TNULL ) continue;
		j = lookup( p->sname, p->sflags&(SMOS|STAG) );
		if( j != i ){
			q = &stab[j];
			if( q->stype == UNDEF ||
			    q->slevel <= p->slevel ){
				cerror( "check error: %.8s", q->sname );
				}
			}
		else if( p->slevel > lev ) cerror( "%.8s check at level %d", p->sname, lev );
		}
	}
#endif

struct symtab *
relook(p) register struct symtab *p; {  /* look up p again, and see where it lies */

	register struct symtab *q;

	/* I'm not sure that this handles towers of several hidden definitions in all cases */
	q = &stab[lookup( p->sname, p->sflags&(STAG|SMOS|SHIDDEN) )];
	/* make relook always point to either p or an empty cell */
	if( q->stype == UNDEF ){
		q->stype = TNULL;
		return(q);
		}
	while( q != p ){
		if( q->stype == TNULL ) break;
		if( ++q >= &stab[SYMTSZ] ) q=stab;
		}
	return(q);
	}

clearst( lev ){ /* clear entries of internal scope  from the symbol table */
	register struct symtab *p, *q, *r;
	register int temp, rehash;

	temp = lineno;
	aobeg();

	/* first, find an empty slot to prevent newly hashed entries from
	   being slopped into... */

	for( q=stab; q< &stab[SYMTSZ]; ++q ){
		if( q->stype == TNULL )goto search;
		}

	cerror( "symbol table full 2");

	search:
	p = q;

	for(;;){
		if( p->stype == TNULL ) {
			rehash = 0;
			goto next;
			}
		lineno = p->suse;
		if( lineno < 0 ) lineno = - lineno;
		if( p->slevel>lev ){ /* must clobber */
			if( p->stype == UNDEF || ( p->sclass == ULABEL && lev < 2 ) ){
				lineno = temp;
				uerror( "%.8s undefined", p->sname );
				}
			else aocode(p);
# ifndef BUG1
			if (ddebug) printf("removing %8s from stab[ %d], flags %o level %d\n",
				p->sname,p-stab,p->sflags,p->slevel);
# endif
			if( p->sflags & SHIDES ) unhide(p);
			p->stype = TNULL;
			rehash = 1;
			goto next;
			}
		if( rehash ){
			if( (r=relook(p)) != p ){
				movestab( r, p );
				p->stype = TNULL;
				}
			}
		next:
		if( ++p >= &stab[SYMTSZ] ) p = stab;
		if( p == q ) break;
		}
	lineno = temp;
	aoend();
	}

movestab( p, q ) register struct symtab *p, *q; {
	int k;
	/* structure assignment: *p = *q; */
	p->stype = q->stype;
	p->sclass = q->sclass;
	p->slevel = q->slevel;
	p->offset = q->offset;
	p->sflags = q->sflags;
	p->dimoff = q->dimoff;
	p->sizoff = q->sizoff;
	p->suse = q->suse;
	for( k=0; k<NCHNAM; ++k ){
		p->sname[k] = q->sname[k];
		}
	}

hide( p ) register struct symtab *p; {
	register struct symtab *q;
	for( q=p+1; ; ++q ){
		if( q >= &stab[SYMTSZ] ) q = stab;
		if( q == p ) cerror( "symbol table full 3" );
		if( q->stype == TNULL ) break;
		}
	movestab( q, p );
	p->sflags |= SHIDDEN;
	q->sflags = (p->sflags&(SMOS|STAG)) | SHIDES;
	if( hflag ) werror( "%.8s redefinition hides earlier one", p->sname );
# ifndef BUG1
	if( ddebug ) printf( "	%d hidden in %d\n", p-stab, q-stab );
# endif
	return( idname = q-stab );
	}

unhide( p ) register struct symtab *p; {
	register struct symtab *q;
	register s, j;

	s = p->sflags & (SMOS|STAG);
	q = p;

	for(;;){

		if( q == stab ) q = &stab[SYMTSZ-1];
		else --q;

		if( q == p ) break;

		if( (q->sflags&(SMOS|STAG)) == s ){
			for( j =0; j<NCHNAM; ++j ) if( p->sname[j] != q->sname[j] ) break;
			if( j == NCHNAM ){ /* found the name */
				q->sflags &= ~SHIDDEN;
# ifndef BUG1
				if( ddebug ) printf( "unhide uncovered %d from %d\n", q-stab,p-stab);
# endif
				return(0);
				}
			}

		}
	cerror( "unhide fails" );
	}
