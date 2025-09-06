	.module	crt0
	.globl	_main
	.globl	l__DATA
	.globl	s__DATA
	.globl	l__INITIALIZER
	.globl	s__INITIALIZER
	.globl	s__INITIALIZED

	.area	_HEADER (ABS)
	.org	0
	ld	sp,#0
	call	gsinit
	jr	0$
	ret
0$:
	ei
	call	_main
1$:
	halt
	jr	1$
	.org	0x38
	ei
	reti
	.org	0x66
	retn
	;
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area	_GSINIT
	.area	_GSFINAL
	.area	_DATA
	.area	_INITIALIZED
	.area	_HEAP
	.area	_HEAP_END
	;
	.area	_GSINIT
gsinit:
	ld	bc,#l__DATA
	ld	hl,#s__DATA
3$:
	ld	(hl),#0
	inc	hl
	dec	bc
	ld	a,c
	or	b
	jr	nz,3$
	ld	bc,#l__INITIALIZER
	ld	a,b
	or	a,c
	jr	z,2$
	ld	de,#s__INITIALIZED
	ld	hl,#s__INITIALIZER
	ldir
2$:
	.area	_GSFINAL
	ret

