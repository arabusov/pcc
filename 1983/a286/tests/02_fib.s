	.data
	.text
	.globl	_main
_main:
	push	bp
	mov	bp,sp
	sub	sp,#LF1
	mov	*-6(bp),*1
	mov	*-8(bp),*0
L13:
	cmp	*-8(bp),#1024
	bge	L14
	mov	ax,*-6(bp)
	mov	*-10(bp),ax
	push	*-8(bp)
	push	#L16
	call	_printf
	add	sp,*4
	mov	ax,*-6(bp)
	add	ax,*-8(bp)
	mov	*-6(bp),ax
	mov	ax,*-10(bp)
	mov	*-8(bp),ax
	br	L13
L14:
	mov	ax,*0
	br	L12
L12:
	lea	sp,*0(bp)
	pop	bp
	ret
	LF1 = 6
	.data
L16:
	.byte	37,100,10,0
