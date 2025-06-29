# include "mfile2"
#include "local2.h"
#include "common.h"
#include "allo.h"
#include "match.h"

NODE resc[3];

int busy[REGSZ];

int maxa, mina, maxb, minb;

int freereg();
int freetemp();
int usable();
int shareit();
int ushare();
void rfree();

void
allo0(){ /* free everything */

	register i;

	maxa = maxb = -1;
	mina = minb = 0;

	REGLOOP(i){
		busy[i] = 0;
		if( rstatus[i] & STAREG ){
			if( maxa<0 ) mina = i;
			maxa = i;
			}
		if( rstatus[i] & STBREG ){
			if( maxb<0 ) minb = i;
			maxb = i;
			}
		}
	}

# define TBUSY 01000

int
allo( p, q ) NODE *p; struct optab *q; {

	register n, i, j;

	n = q->needs;
	i = 0;

	while( n & NACOUNT ){
		resc[i].in.op = REG;
		resc[i].tn.rval = freereg( p, n&NAMASK );
		resc[i].tn.lval = 0;
		resc[i].in.name[0] = '\0';
		n -= NAREG;
		++i;
		}

	while( n & NBCOUNT ){
		resc[i].in.op = REG;
		resc[i].tn.rval = freereg( p, n&NBMASK );
		resc[i].tn.lval = 0;
		resc[i].in.name[0] = '\0';
		n -= NBREG;
		++i;
		}

	if( n & NTMASK ){
		resc[i].in.op = OREG;
		resc[i].tn.rval = TMPREG;
		if( p->in.op == STCALL || p->in.op == STARG || p->in.op == UNARY STCALL || p->in.op == STASG ){
			resc[i].tn.lval = freetemp( (SZCHAR*p->stn.stsize + (SZINT-1))/SZINT );
			}
		else {
			resc[i].tn.lval = freetemp( (n&NTMASK)/NTEMP );
			}
		resc[i].in.name[0] = '\0';
		resc[i].tn.lval = BITOOR(resc[i].tn.lval);
		++i;
		}

	/* turn off "temporarily busy" bit */

	REGLOOP(j){
		busy[j] &= ~TBUSY;
		}

	for( j=0; j<i; ++j ) if( resc[j].tn.rval < 0 ) return(0);
	return(1);

	}

int
freetemp( k ){ /* allocate k integers worth of temp space */
/* we also make the convention that, if the number of words is more than 1,
	/* it must be aligned for storing doubles... */

# ifndef BACKTEMP
	int t;

	if( k>1 ){
		SETOFF( tmpoff, ALDOUBLE );
		}

	t = tmpoff;
	tmpoff += k*SZINT;
	if( tmpoff > maxoff ) maxoff = tmpoff;
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	return(t);

# else
	tmpoff += k*SZINT;
	if( k>1 ) {
		SETOFF( tmpoff, ALDOUBLE );
		}
	if( tmpoff > maxoff ) maxoff = tmpoff;
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	return( -tmpoff );
# endif
	}

int
freereg( p, n ) NODE *p; {
	/* allocate a register of type n */
	/* p gives the type, if floating */

	register j;

	/* not general; means that only one register (the result) OK for call */
	if( callop(p->in.op) ){
		j = callreg(p);
		if( usable( p, n, j ) ) return( j );
		/* have allocated callreg first */
		}
	j = p->in.rall & ~MUSTDO;
	if( j!=NOPREF && usable(p,n,j) ){ /* needed and not allocated */
		return( j );
		}
	if( n&NAMASK ){
		for( j=mina; j<=maxa; ++j ) if( rstatus[j]&STAREG ){
			if( usable(p,n,j) ){
				return( j );
				}
			}
		}
	else if( n &NBMASK ){
		for( j=minb; j<=maxb; ++j ) if( rstatus[j]&STBREG ){
			if( usable(p,n,j) ){
				return(j);
				}
			}
		}

	return( -1 );
	}

int
usable( p, n, r ) NODE *p; {
	/* decide if register r is usable in tree p to satisfy need n */

	/* checks, for the moment */
	if( !istreg(r) ) cerror( "usable asked about nontemp register" );

	if( busy[r] > 1 ) return(0);

	if( (n & NBMASK) && !isbreg(r)) return(0);
/*	if( isbreg(r) ){
		if( n&NAMASK ) return(0);
		}
	else {
		if( n & NBMASK ) return(0);
		}
*/
	if( (n&NAMASK) && (szty(p->in.type) == 2) ){ /* only do the pairing for real regs */
		if( r&01 ) return(0);
		if( !istreg(r+1) ) return( 0 );
		if( busy[r+1] > 1 ) return( 0 );
		if( busy[r] == 0 && busy[r+1] == 0  ||
		    busy[r+1] == 0 && shareit( p, r, n ) ||
		    busy[r] == 0 && shareit( p, r+1, n ) ){
			busy[r] |= TBUSY;
			busy[r+1] |= TBUSY;
			return(1);
			}
		else return(0);
		}
	if( busy[r] == 0 ) {
		busy[r] |= TBUSY;
		return(1);
		}

	/* busy[r] is 1: is there chance for sharing */
	return( shareit( p, r, n ) );

	}

int
shareit( p, r, n ) NODE *p; {
	/* can we make register r available by sharing from p
	   given that the need is n */
	if( (n&(NASL|NBSL)) && ushare( p, 'L', r ) ) return(1);
	if( (n&(NASR|NBSR)) && ushare( p, 'R', r ) ) return(1);
	return(0);
	}

int
ushare( p, f, r ) NODE *p; {
	/* can we find a register r to share on the left or right
		(as f=='L' or 'R', respectively) of p */
	p = getlr( p, f );
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == OREG ){
		if( R2TEST(p->tn.rval) ){
			return( r==R2UPK1(p->tn.rval) || r==R2UPK2(p->tn.rval) );
			}
		else return( r == p->tn.rval );
		}
	if( p->in.op == REG ){
		return( r == p->tn.rval || ( szty(p->in.type) == 2 && r==p->tn.rval+1 ) );
		}
	return(0);
	}

recl2( p ) register NODE *p; {
	register r = p->tn.rval;
	if( p->in.op == REG ) rfree( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST( r ) ) {
			rfree( R2UPK1( r ), PTR+INT );
			rfree( R2UPK2( r ), INT );
			}
		else {
			rfree( r, PTR+INT );
			}
		}
	}

int rdebug = 0;

void
rfree( r, t ) TWORD t; {
	/* mark register r free, if it is legal to do so */
	/* t is the type */

	if( rdebug ){
		printf( "rfree( %s ), size %d\n", rnames[r], szty(t) );
		}

	if( istreg(r) ){
		if( --busy[r] < 0 ) cerror( "register overfreed");
		if( szty(t) == 2 ){
			if( (r&01) || (istreg(r)^istreg(r+1)) ) cerror( "illegal free" );
			if( --busy[r+1] < 0 ) cerror( "register overfreed" );
			}
		}
	}

rbusy(r,t) TWORD t; {
	/* mark register r busy */
	/* t is the type */

	if( rdebug ){
		printf( "rbusy( %s ), size %d\n", rnames[r], szty(t) );
		}

	if( istreg(r) ) ++busy[r];
	if( szty(t) == 2 ){
		if( istreg(r+1) ) ++busy[r+1];
		if( (r&01) || (istreg(r)^istreg(r+1)) ) cerror( "illegal register pair freed" );
		}
	}

void
rwprint( rw ){ /* print rewriting rule */
	register i, flag;
	static char * rwnames[] = {

		"RLEFT",
		"RRIGHT",
		"RESC1",
		"RESC2",
		"RESC3",
		0,
		};

	if( rw == RNULL ){
		printf( "RNULL" );
		return;
		}

	if( rw == RNOP ){
		printf( "RNOP" );
		return;
		}

	flag = 0;
	for( i=0; rwnames[i]; ++i ){
		if( rw & (1<<i) ){
			if( flag ) printf( "|" );
			++flag;
			printf( rwnames[i] );
			}
		}
	}

void
reclaim( p, rw, cookie ) NODE *p; {
	register NODE **qq;
	register NODE *q;
	register i;
	NODE *recres[5];
	struct respref *r;

	/* get back stuff */

#ifndef	BUG4
	if( rdebug ){
		printf( "reclaim( %o, ", p );
		rwprint( rw );
		printf( ", " );
		prcook( cookie );
		printf( " )\n" );
		}
#endif
	if( rw == RNOP || ( p->in.op==FREE && rw==RNULL ) ) return;  /* do nothing */

	walkf( p, recl2 );

	if( callop(p->in.op) ){
		/* check that all scratch regs are free */
		callchk(p);  /* ordinarily, this is the same as allchk() */
		}

	if( rw == RNULL || (cookie&FOREFF) ){ /* totally clobber, leaving nothing */
		tfree(p);
		return;
		}

	/* handle condition codes specially */

	if( (cookie & FORCC) && (rw&RESCC)) {
		/* result is CC register */
		tfree(p);
		p->in.op = CCODES;
		p->tn.lval = 0;
		p->tn.rval = 0;
		return;
		}

	/* locate results */

	qq = recres;

	if( rw&RLEFT) *qq++ = getlr( p, 'L' );;
	if( rw&RRIGHT ) *qq++ = getlr( p, 'R' );
	if( rw&RESC1 ) *qq++ = &resc[0];
	if( rw&RESC2 ) *qq++ = &resc[1];
	if( rw&RESC3 ) *qq++ = &resc[2];

	if( qq == recres ){
		cerror( "illegal reclaim");
		}

	*qq = NIL;

	/* now, select the best result, based on the cookie */

	for( r=respref; r->cform; ++r ){
		if( cookie & r->cform ){
			for( qq=recres; (q= *qq) != NIL; ++qq ){
				if( tshape( q, r->mform ) ) goto gotit;
				}
			}
		}

	/* we can't do it; die */
	cerror( "cannot reclaim");

	gotit:

	if( p->in.op == STARG ) p = p->in.left;  /* STARGs are still STARGS */

	q->in.type = p->in.type;  /* to make multi-register allocations work */
		/* maybe there is a better way! */
	q = tcopy(q);

	tfree(p);

	p->in.op = q->in.op;
	p->tn.lval = q->tn.lval;
	p->tn.rval = q->tn.rval;
	for( i=0; i<NCHNAM; ++i )
		p->in.name[i] = q->in.name[i];

	q->in.op = FREE;

	/* if the thing is in a register, adjust the type */

	switch( p->in.op ){

	case REG:
		if( !rtyflg ){
			/* the C language requires intermediate results to change type */
			/* this is inefficient or impossible on some machines */
			/* the "T" command in match supresses this type changing */
			if( p->in.type == CHAR || p->in.type == SHORT ) p->in.type = INT;
			else if( p->in.type == UCHAR || p->in.type == USHORT ) p->in.type = UNSIGNED;
			else if( p->in.type == FLOAT ) p->in.type = DOUBLE;
			}
		if( ! (p->in.rall & MUSTDO ) ) return;  /* unless necessary, ignore it */
		i = p->in.rall & ~MUSTDO;
		if( i & NOPREF ) return;
		if( i != p->tn.rval ){
			if( busy[i] || ( szty(p->in.type)==2 && busy[i+1] ) ){
				cerror( "faulty register move" );
				}
			rbusy( i, p->in.type );
			rfree( p->tn.rval, p->in.type );
			rmove( i, p->tn.rval, p->in.type );
			p->tn.rval = i;
			}

	case OREG:
		if( R2TEST(p->tn.rval) ){
			int r1, r2;
			r1 = R2UPK1(p->tn.rval);
			r2 = R2UPK2(p->tn.rval);
			if( (busy[r1]>1 && istreg(r1)) || (busy[r2]>1 && istreg(r2)) ){
				cerror( "potential register overwrite" );
				}
			}
		else if( (busy[p->tn.rval]>1) && istreg(p->tn.rval) ) cerror( "potential register overwrite");
		}

	}

void
ncopy( q, p ) NODE *p, *q; {
	/* copy the contents of p into q, without any feeling for
	   the contents */
	/* this code assume that copying rval and lval does the job;
	   in general, it might be necessary to special case the
	   operator types */
	register i;

	q->in.op = p->in.op;
	q->in.rall = p->in.rall;
	q->in.type = p->in.type;
	q->tn.lval = p->tn.lval;
	q->tn.rval = p->tn.rval;
	for( i=0; i<NCHNAM; ++i ) q->in.name[i]  = p->in.name[i];

	}

NODE *
tcopy( p ) register NODE *p; {
	/* make a fresh copy of p */

	register NODE *q;
	register r;

	ncopy( q=talloc(), p );

	r = p->tn.rval;
	if( p->in.op == REG ) rbusy( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST(r) ){
			rbusy( R2UPK1(r), PTR+INT );
			rbusy( R2UPK2(r), INT );
			}
		else {
			rbusy( r, PTR+INT );
			}
		}

	switch( optype(q->in.op) ){

	case BITYPE:
		q->in.right = tcopy(p->in.right);
	case UTYPE:
		q->in.left = tcopy(p->in.left);
		}

	return(q);
	}

allchk(){
	/* check to ensure that all register are free */

	register i;

	REGLOOP(i){
		if( istreg(i) && busy[i] ){
			cerror( "register allocation error");
			}
		}

	}
