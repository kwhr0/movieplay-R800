	.module	sjpeg
	.globl	_imul_b1_b3
	.globl	_imul_b2
	.globl	_imul_b4
	.globl	_imul_b5

	.area	_CODE
_imul_b1_b3:
	ld	bc,#362
	jr	mul
_imul_b2:
	ld	bc,#669
	jr	mul
_imul_b4:
	ld	bc,#277
	jr	mul
_imul_b5:
	ld	bc,#196
mul:
	xor	a
	.db	0xfd,0x6f	;ld	iyl,a
	ld	a,h
	or	a
	jp	p,10$
;	ld	a,1
	.db	0xfd,0x6f	;ld	iyl,a
	ld	de,#0
	ex	de,hl
;	or	a
	sbc	hl,de
10$:
	.db	0xed,0xc3	;muluw bc
	ld	bc,#0x80
	add	hl,bc
	jr	nc,11$
	inc	e
11$:
	ld	l,h
	ld	h,e
	.db	0xfd,0x7d	;ld	a,iyl
	or	a
	jr	z,12$
	ld	de,#0
	ex	de,hl
;	or	a
	sbc	hl,de
12$:
	ex	de,hl
	ret

