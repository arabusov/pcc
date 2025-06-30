	.data
	.text
	.globl	_isupper
_isupper:
	push	bp
	mov	bp,sp
	sub	sp,#LF1
	cmpb	*4(bp),*65
	blt	L10000
	cmpb	*4(bp),*90
	bgt	L10000
	mov	ax,*1
	br	L10001
L10000:
	mov	ax,*0
L10001:
	br	L12
L12:
	lea	sp,*0(bp)
	pop	bp
	ret
	LF1 = 0
	.data
	.text
	.globl	_islower
_islower:
	push	bp
	mov	bp,sp
	sub	sp,#LF2
	cmpb	*4(bp),*97
	blt	L10002
	cmpb	*4(bp),*122
	bgt	L10002
	mov	ax,*1
	br	L10003
L10002:
	mov	ax,*0
L10003:
	br	L14
L14:
	lea	sp,*0(bp)
	pop	bp
	ret
	LF2 = 0
	.data
	.text
	.globl	_islette
_islette:
	push	bp
	mov	bp,sp
	sub	sp,#LF3
	movb	ax,*4(bp)
	cbw
	push	ax
	call	_isupper
	pop	cx
	or	ax,ax
	bne	L10005
	movb	ax,*4(bp)
	cbw
	push	ax
	call	_islower
	pop	cx
	or	ax,ax
	beq	L10004
L10005:
	mov	ax,*1
	br	L10006
L10004:
	mov	ax,*0
L10006:
	br	L16
L16:
	lea	sp,*0(bp)
	pop	bp
	ret
	LF3 = 0
	.data
