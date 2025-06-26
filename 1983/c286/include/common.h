# ifndef COMMON_H
# define COMMON_H

unsigned caloff();

int uerror( );
	/* the routine where is different for pass 1 and pass 2;
	 *  it tells where the error took place */

	/* VARARGS1 */
int cerror();

	/* VARARGS1 */
int werror();

int tinit();

# define TNEXT(p) (p== &node[TREESZ-1]?node:p+1)

int tcheck();
int tfree();

int tfree1();

int fwalk();

int walkf();



int mkdope();

# ifndef BUG4
int tprint();
# endif

# endif /* COMMON_H */
