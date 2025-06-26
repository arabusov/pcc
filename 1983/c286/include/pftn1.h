#ifndef PFTN1_H
#define PFTN1_H
int defid();
void yyerror();
void ftnend();
int bstruct();
void moedef();
void ftnarg();
void dclargs();

struct instk {
	int in_sz;   /* size of array element */
	int in_x;    /* current index for structure member in structure initializations */
	int in_n;    /* number of initializations seen */
	int in_s;    /* sizoff */
	int in_d;    /* dimoff */
	TWORD in_t;    /* type */
	int in_id;   /* stab index */
	int in_fl;   /* flag which says if this level is controlled by {} */
	OFFSZ in_off;  /* offset of the beginning of this level */
};

#endif /* PFTN1_H */
