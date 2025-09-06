// R800
// Copyright 2025 © Yasuo Kuwahara
// MIT License

#include "R800.h"
#include "main.h" // emulator specific
#include <string>

#define B			(r[1])
#define C			(r[0])
#define H			(r[5])
#define L			(r[4])
#define F			(r[7])
#define A			(r[6])
#define REG0		(r[1])
#define REG1		(r[0])
#define REG2		(r[3])
#define REG3		(r[2])
#define REG4		(r[5 + rofs])
#define REG5		(r[4 + rofs])
#define REG7		(r[6])
#define REGFIX0		(r[1])
#define REGFIX1		(r[0])
#define REGFIX2		(r[3])
#define REGFIX3		(r[2])
#define REGFIX4		(r[5])
#define REGFIX5		(r[4])
#define REGFIX7		(r[6])

#define BC			((u16 &)r[0])
#define DE			((u16 &)r[2])
#define HL			((u16 &)r[4 + rofs])
#define HLfix		((u16 &)r[4])
#define REG160		BC
#define REG162		DE
#define REG164		HL
#define REGFIX160	BC
#define REGFIX162	DE
#define REGFIX164	HLfix

#define SET2(macro)\
	macro(0)\
	macro(2)
#define SET3(macro)\
	macro(0)\
	macro(2)\
	macro(4)
#define SET7(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(7)
#define SET7BIT(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(6)
#define SET_8(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(6)\
	macro(7)
#define SET7X(macro, X)\
	macro(0, X)\
	macro(1, X)\
	macro(2, X)\
	macro(3, X)\
	macro(4, X)\
	macro(5, X)\
	macro(7, X)
#define SET77REG(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 7)
#define SET77BIT(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 6)
#define SET78(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 6)\
	SET7X(macro, 7)
#define COND8(macro)\
	macro(0, !(F & MZ))\
	macro(1, F & MZ)\
	macro(2, !(F & MC))\
	macro(3, F & MC)\
	macro(4, !(F & MP))\
	macro(5, F & MP)\
	macro(6, !(F & MS))\
	macro(7, F & MS)

#define OFS_IX 		8
#define OFS_IY 		16
#define CY			(F & MC)
#define swap(a, b)	(tmp2 = (a), (a) = (b), (b) = tmp2)

static constexpr uint8_t clk[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    1, 3, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, // 0x00
    2, 3, 2, 1, 1, 1, 2, 1, 3, 1, 2, 1, 1, 1, 2, 1, // 0x10
    2, 3, 2, 1, 1, 1, 2, 1, 2, 1, 5, 1, 1, 1, 2, 1, // 0x20
    2, 3, 4, 1, 4, 4, 3, 1, 2, 1, 4, 1, 1, 1, 2, 1, // 0x30
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x40
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x50
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x60
    2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 1, // 0x70
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x80
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x90
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0xa0
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0xb0
    1, 3, 3, 3, 3, 4, 2, 4, 1, 3, 3, 0, 3, 5, 2, 4, // 0xc0
    1, 3, 3, 3, 3, 4, 2, 4, 1, 1, 3, 3, 3, 1, 2, 4, // 0xd0
    1, 3, 3, 5, 3, 4, 2, 4, 1, 1, 3, 1, 3, 0, 2, 4, // 0xe0
    1, 3, 3, 2, 3, 4, 2, 4, 1, 1, 3, 1, 3, 1, 2, 4, // 0xf0
};

static constexpr uint8_t clk_ed[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x00
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x10
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x20
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x30
    3, 3, 2, 6, 2, 5, 3, 2, 3, 3, 2, 6, 2, 5, 2, 2, // 0x40
    3, 3, 2, 6, 2, 2, 3, 2, 3, 3, 2, 6, 2, 2, 3, 2, // 0x50
    3, 3, 2, 6, 2, 2, 2, 5, 3, 3, 2, 6, 2, 2, 2, 5, // 0x60
    3, 2, 2, 6, 2, 2, 2, 2, 3, 3, 2, 6, 2, 2, 2, 2, // 0x70
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x80
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x90
    4, 4, 4, 4, 2, 2, 2, 2, 4, 4, 4, 4, 2, 2, 2, 2, // 0xa0
    4, 5, 3, 3, 2, 2, 2, 2, 4, 5, 3, 3, 2, 2, 2, 2, // 0xb0
    2,14, 2,36, 2, 2, 2, 2, 2,14, 2, 2, 2, 2, 2, 2, // 0xc0
    2,14, 2,36, 2, 2, 2, 2, 2,14, 2, 2, 2, 2, 2, 2, // 0xd0
    2,14, 2,36, 2, 2, 2, 2, 2,14, 2, 2, 2, 2, 2, 2, // 0xe0
    2, 2, 2,36, 2, 2, 2, 2, 2,14, 2, 2, 2, 2, 2, 2, // 0xf0
};

static constexpr uint8_t clk_cb[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x00
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x10
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x20
	2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x30
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 0x40
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 0x50
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 0x60
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 0x70
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x80
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0x90
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xa0
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xb0
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xc0
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xd0
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xe0
    2, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 5, 2, // 0xf0
};

R800::R800() {
	Reset();
	m = nullptr;
#if R800_TRACE
	for (TraceBuffer *p = tracebuf; p < tracebuf + TRACEMAX; p++) {
		p->pc = p->adr1 = p->data1 = p->adr2 = p->data2 = 0;
		p->r0 = p->r1 = 0;
		p->acs1 = p->acs2 = 0;
	}
	tracep = tracebuf;
#endif
}

void R800::Reset() {
#if 0
	IntVec = a_ = f_ = 0xff;
	hl = 0;
	bcde = 0;
	for (int i = 0; i < 24; i++) r[i] = 0;
#endif
	Iff1 = Iff2 = IntMode = RefReg = RefReg7 = 0;
	pc = 0;
	rofs = 0;
	A = F = 0xff;
	sp = 0xffff;
	status = 0;
}

// customized I/O access -- start

R800::s32 R800::in(u16 adr) {
	s32 data = 0;
	switch (adr & 0xff) {
		case 0: data = file_getc();
	}
	TRACE_LOG1(acsIn);
	return data;
}

void R800::out(u16 adr, u8 data) {
	static int lba, vdata;
	static Rect rect;
	switch (adr & 0xff) {
		case 0: putchar(data); break;
		case 1: lba = data; break;
		case 2: lba |= data << 8; break;
		case 3: lba |= data << 16; file_seek(lba); break;
		case 4: rect.left = data; break;
		case 5: rect.right = data; break;
		case 6: rect.top = data; break;
		case 7: rect.bottom = data; set_area(&rect); break;
		case 8: vdata = data; break;
		case 9: vdata |= data << 8; set_data(vdata); break;
	}
	TRACE_LOG2(acsOut);
}

// customized I/O access -- end

R800::s32 R800::Execute(s32 n) {
	s32 tmp, tmp2, cy;
	clock = clock_i = 0;
	do {
		RefReg++;
		u8 op = imm8();
		clock += clk[op];
		if ((op & 0xdf) == 0xdd) {
			rofs = op & 0x20 ? OFS_IY : OFS_IX;
			RefReg++;
			op = imm8();
			clock += clk[op];
			clock_i = 2;
		}
#if R800_TRACE
		tracep->pc = pc - 1 - (rofs != 0);
		tracep->op = op;
		tracep->acs1 = tracep->acs2 = 0;
#endif
		switch (op) {
#define RST(i) case 0xc7 + 8 * (i): if (Extender(8 * i)) break; st16(sp -= 2, pc); pc = 8 * i; break;
			SET_8(RST)
			case 0xfe:
			tmp = imm8();
			fsub(A, tmp, A - tmp, 0); // cp n
			break;
			case 0xf6:
			fxor(A |= imm8()); // or n
			break;
			case 0xee:
			fxor(A ^= imm8()); // xor n
			break;
			case 0xe6:
			fand(A &= imm8()); // and n
			break;
			case 0xde: // sbc a,n
			tmp = imm8();
			cy = CY;
			fsub(A, tmp, tmp2 = A - tmp - cy, cy);
			A = tmp2;
			break;
			case 0xd6: // sub n
			tmp = imm8();
			fsub(A, tmp, tmp2 = A - tmp, 0);
			A = tmp2;
			break;
			case 0xce: // adc a,n
			tmp = imm8();
			cy = CY;
			fadd(A, tmp, tmp2 = A + tmp + cy, cy);
			A = tmp2;
			break;
			case 0xc6: // add a,n
			tmp = imm8();
			fadd(A, tmp, tmp2 = A + tmp, 0);
			A = tmp2;
			break;
			case 0xed:
			RefReg++;
			op = imm8();
			clock += clk_ed[op];
			switch (op) {
				case 0x6f: // rld
				tmp2 = ld8(HLfix) << 4 | (A & 0xf);
				frd(A = (A & 0xf0) | (tmp2 >> 8 & 0xf));
				st8(HLfix, tmp2);
				break;
				case 0x67: // rrd
				tmp2 = ld8(HLfix);
				tmp = tmp2 & 0xf;
				tmp2 = (tmp2 >> 4 & 0xf) | A << 4;
				frd(A = (A & 0xf0) | tmp);
				st8(HLfix, tmp2);
				break;
				case 0x5f:
				fmov(A = (RefReg & 0x7f) | RefReg7);
				break;
				case 0x57:
				fmov(A = IntVec);
				break;
				case 0x4f:
				RefReg = A;
				RefReg7 = A & 0x80;
				break;
				case 0x47:
				IntVec = A;
				break;
				case 0x44: case 0x4c: case 0x54: case 0x5c: 
				case 0x64: case 0x6c: case 0x74: case 0x7c: // neg
				fsub(0, -A, tmp2 = -A, 0);
				A = tmp2;
				break;
				case 0x4d: // reti
				RETI();
				// fall
				case 0x45: case 0x55: case 0x5d: case 0x65: case 0x6d: case 0x75: case 0x7d: // retn
				Iff1 = Iff2;
				pc = ld16(sp);
				sp += 2;
				break;
				case 0x7b: // ld sp,(nn)
				sp = ld16(imm16());
				break;
				// ld bc/de/hl,(nn)
#define LD_REG_NN(i) case 0x4b + 8 * (i): REGFIX16##i = ld16(imm16()); break;
				SET3(LD_REG_NN)
				case 0x73: // ld (nn),sp
				st16(imm16(), sp);
				break;
				// ld (nn),bc/de/hl
#define LD_NN_REG(i) case 0x43 + 8 * (i): st16(imm16(), REGFIX16##i); break;
				SET3(LD_NN_REG)
				case 0x7a: // adc hl,sp
				tmp = sp + (cy = CY);
				fadc16(HLfix, tmp2 = (s32)HLfix + tmp, cy);
				HLfix = tmp2;
				break;
				// adc hl,bc/de/hl
#define ADC_HL_REG(i) case 0x4a + 8 * (i): tmp = REGFIX16##i + (cy = CY); fadc16(HLfix, tmp2 = (s32)HLfix + tmp, cy); HLfix = tmp2; break;
				SET3(ADC_HL_REG)
				case 0x72: // sbc hl,sp
				tmp = sp + (cy = CY);
				fsbc16(HLfix, tmp2 = (s32)HLfix - tmp, cy);
				HLfix = tmp2;
				break;
				// sbc hl,bc/de/hl
#define SBC_HL_REG(i) case 0x42 + 8 * (i): tmp = REGFIX16##i + (cy = CY); fsbc16(HLfix, tmp2 = (s32)HLfix - tmp, cy); HLfix = tmp2; break;
				SET3(SBC_HL_REG)
				// out (c),reg
#define OUT_C_REG(i) case 0x41 + 8 * (i): out(BC, REGFIX##i); break;
				SET7(OUT_C_REG)
				// in reg,(c)
#define IN_REG_C(i) case 0x40 + 8 * (i): tmp2 = in(BC); fin(REGFIX##i, tmp2); REGFIX##i = tmp2; break;
				SET7(IN_REG_C)
				case 0x71: // out (c),0
				out(BC, 0);
				break;
				case 0x70: // in (c)
				fin(0, in(BC));
				break;
				case 0xbb: // otdr
				B--;
				out(BC, tmp = ld8(HLfix--));
				fbio(B, L + tmp, tmp);
				if (B) {
					pc -= 2;
					clock++;
				}
				break;
				case 0xab: // outd
				B--;
				out(BC, tmp = ld8(HLfix--));
				fbio(B, L + tmp, tmp);
				break;
				case 0xba: // indr
				st8(HLfix--, tmp = in(BC));
				fbio(--B, (C - 1 & 0xff) + tmp, tmp);
				if (B) {
					pc -= 2;
					clock++;
				}
				break;
				case 0xaa: // ind
				st8(HLfix--, tmp = in(BC));
				fbio(--B, (C - 1 & 0xff) + tmp, tmp);
				break;
				case 0xb9: // cpdr
				tmp2 = A - ld8(HLfix);
				HLfix--;
				fcp(--BC, A, tmp2);
				if (BC && tmp2) pc -= 2;
				break;
				case 0xa9: // cpd
				tmp2 = A - ld8(HLfix);
				HLfix--;
				fcp(--BC, A, tmp2);
				break;
				case 0xb8: // lddr
				st8(DE, ld8(HLfix));
				DE--;
				HLfix--;
				fbtr(--BC);
				if (BC) pc -= 2;
				break;
				case 0xa8: // ldd
				st8(DE, ld8(HLfix));
				DE--;
				HLfix--;
				fbtr(--BC);
				break;
				case 0xb3: // otir
				B--;
				out(BC, tmp = ld8(HLfix++));
				fbio(B, L + tmp, tmp);
				if (B) {
					pc -= 2;
					clock++;
				}
				break;
				case 0xa3: // outi
				B--;
				out(BC, tmp = ld8(HLfix++));
				fbio(B, L + tmp, tmp);
				break;
				case 0xb2: // inir
				st8(HLfix++, tmp = in(BC));
				fbio(--B, (C + 1 & 0xff) + tmp, tmp);
				if (B) {
					pc -= 2;
					clock++;
				}
				break;
				case 0xa2: // ini
				st8(HLfix++, tmp = in(BC));
				fbio(--B, (C + 1 & 0xff) + tmp, tmp);
				break;
				case 0xb1: // cpir
				tmp2 = A - ld8(HLfix);
				HLfix++;
				fcp(--BC, A, tmp2);
				if (BC && tmp2) pc -= 2;
				break;
				case 0xa1: // cpi
				tmp2 = A - ld8(HLfix);
				HLfix++;
				fcp(--BC, A, tmp2);
				break;
				case 0xb0: // ldir
				st8(DE, ld8(HLfix));
				DE++;
				HLfix++;
				fbtr(--BC);
				if (BC) pc -= 2;
				break;
				case 0xa0: // ldi
				st8(DE, ld8(HLfix));
				DE++;
				HLfix++;
				fbtr(--BC);
				break;
				case 0x46: case 0x4e: case 0x66: case 0x6e:
				IntMode = 0;
				break;
				case 0x56: case 0x76:
				IntMode = 1;
				break;
				case 0x5e: case 0x7e:
				IntMode = 2;
				break;
#define MULUB(i) case 0xc1 + 8 * (i): HLfix = A * REGFIX##i; fmulub(H, HLfix); break;
				SET7(MULUB)
#define MULUW(i) case 0xc3 + 8 * (i): tmp = (u32)HLfix * REGFIX16##i; DE = tmp >> 16; HLfix = tmp; fmuluw(DE, tmp); break;
				SET3(MULUW);
				case 0xf3: // mulw hl,sp
				tmp = (u32)HLfix * sp; DE = tmp >> 16; HLfix = tmp; fmuluw(DE, tmp);
				break;
			}
			break;
			case 0xcd: // call nn
			st16(sp -= 2, pc + 2);
			tmp2 = imm16();
			pc = tmp2;
			break;
			case 0xf5: // push af
			st16(sp -= 2, A << 8 | F);
			break;
			// push bc/de/hl
#define PUSH(i) case 0xc5 + 8 * (i): st16(sp -= 2, REG16##i); break;
			SET3(PUSH)
			// conditional call
#define CALL_COND(i, cond) case 0xc4 + 8 * (i): if (cond) { tmp2 = imm16(); st16(sp -= 2, pc); pc = tmp2; clock += 2; } else pc += 2; break;
			COND8(CALL_COND)
			case 0xf3:
			Iff1 = Iff2 = 0;
			break;
			case 0xfb:
			status |= S_IFF;
			break;
			case 0xeb: // ex de,hl
			swap(DE, HLfix);
			break;
			case 0xe3: // ex (sp),hl
			tmp2 = ld16(sp);
			st16(sp, HL);
			HL = tmp2;
			break;
			case 0xdb: // in a,(n)
			A = in(imm8() | A << 8);
			break;
			case 0xd3: // out (n),a
			out(imm8() | A << 8, A);
			break;
			case 0xcb:
			RefReg++;
			if (rofs) DDCB_FDCB();
            else switch (clock += clk_cb[op = imm8()]; op) {
				// rlc reg
#define RLC(i) case (i): tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | tmp2 & 1, tmp2); break;
				SET7(RLC)
				// rlc (hl)
				case 6:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 = tmp2 << 1 | (tmp & 1), tmp);
				st8(HL, tmp2);
				break;
				// rrc reg
#define RRC(i) case 8 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | tmp2 << 7, tmp2); break;
				SET7(RRC)
				// rrc (hl)
				case 0xe:
				tmp2 = ld8(HL);
				frs(tmp = tmp2 >> 1 | tmp2 << 7, tmp2);
				st8(HL, tmp);
				break;
				// rl reg
#define RL(i) case 0x10 + (i): tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | CY, tmp2); break;
				SET7(RL)
				// rl (hl)
				case 0x16:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 = tmp2 << 1 | CY, tmp);
				st8(HL, tmp2);
				break;
				// rr reg
#define RR(i) case 0x18 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | CY << 7, tmp2); break;
				SET7(RR)
				// rr(hl)
				case 0x1e:
				tmp2 = ld8(HL);
				tmp = tmp2;
				frs(tmp2 = tmp2 >> 1 | CY << 7, tmp);
				st8(HL, tmp2);
				break;
				// sla reg
#define SLA(i) case 0x20 + (i): tmp2 = REG##i >> 7; frs(REG##i <<= 1, tmp2); break;
				SET7(SLA)
				// sla (hl)
				case 0x26:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 <<= 1, tmp);
				st8(HL, tmp2);
				break;
				// sra reg
#define SRA(i) case 0x28 + (i): tmp2 = (s8)REG##i; frs(REG##i = tmp2 >> 1, tmp2); break;
				SET7(SRA)
				// sra (hl)
				case 0x2e:
				tmp = tmp2 = (s8)ld8(HL);
				frs(tmp2 >>= 1, tmp);
				st8(HL, tmp2);
				break;
				// sll reg (set LSB)
#define SLL(i) case 0x30 + (i): tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | 1, tmp2); break;
				SET7(SLL)
				// sll (hl) (set LSB)
				case 0x36:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 = tmp2 << 1 | 1, tmp);
				st8(HL, tmp2);
				break;
				// srl reg
#define SRL(i) case 0x38 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1, tmp2); break;
				SET7(SRL)
				// srl (hl)
				case 0x3e:
				tmp = tmp2 = ld8(HL);
				frs(tmp2 >>= 1, tmp);
				st8(HL, tmp2);
				break;
#define BIT(i, j) case 0x40 + (i) + 8 * (j): fbit(REG##i >> (j)); break;
				SET77BIT(BIT)
#define BIT7(i) case 0x78 + (i): fbits(REG##i); break;
				SET7(BIT7)
#define BIT_HL(j) case 0x46 + 8 * (j): fbit(ld8(HL) >> (j)); break;
				SET7BIT(BIT_HL)
				case 0x7e: // bit 7,(hl)
				fbits(ld8(HL));
				break;
#define RES(i, j) case 0x80 + (i) + 8 * (j): REG##i &= ~(1 << (j)); break;
				SET78(RES)
#define RES_HL(j) case 0x86 + 8 * (j): st8(HL, ld8(HL) & ~(1 << (j))); break;
				SET_8(RES_HL)
#define SET(i, j) case 0xc0 + (i) + 8 * (j): REG##i |= 1 << (j); break;
				SET78(SET)
#define SET_HL(j) case 0xc6 + 8 * (j): st8(HL, ld8(HL) | 1 << (j)); break;
				SET_8(SET_HL)
			}
			break;
			case 0xc3: // jp nn
			tmp2 = imm16();
			pc = tmp2;
			break;
			// conditional jump
#define JP_COND(i, cond) case 0xc2 + 8 * (i): tmp2 = imm16(); if (cond) pc = tmp2; break;
			COND8(JP_COND)
			case 0xf9: // ld sp,hl
			sp = HL;
			break;
			case 0xe9: // jp (hl)
			pc = HL;
			break;
			case 0xd9: // exx
			swap((u32 &)r[0], bcde);
			swap(HLfix, hl);
			break;
			case 0xc9: // ret
			pc = ld16(sp);
			sp += 2;
			break;
			case 0xf1: // pop af
			tmp = ld16(sp);
			sp += 2;
			F = tmp;
			A = tmp >> 8;
			break;
			// pop bc/de/hl
#define POP(i) case 0xc1 + 8 * (i): REG16##i = ld16(sp); sp += 2; break;
			SET3(POP)
			// conditional return
#define RET_COND(i, cond) case 0xc0 + 8 * (i): if (cond) { pc = ld16(sp); sp += 2; clock += 2; } break;
			COND8(RET_COND)
			case 0xbe: // cp (hl)
			tmp = ld8(HL);
			fsub(A, tmp, A - tmp, 0);
			if (rofs) pc++;
			break;
			// cp reg
#define CP(i) case 0xb8 + (i): tmp = REG##i; fsub(A, tmp, A - tmp, 0); break;
			SET7(CP)
			// or (hl)
			case 0xb6:
			fxor(A |= ld8(HL));
			if (rofs) pc++;
			break;
			// or reg
#define OR(i) case 0xb0 + (i): fxor(A |= REG##i); break;
			SET7(OR)
			// xor (hl)
			case 0xae:
			fxor(A ^= ld8(HL));
			if (rofs) pc++;
			break;
			// xor reg
#define XOR(i) case 0xa8 + (i): fxor(A ^= REG##i); break;
			SET7(XOR)
			// and (hl)
			case 0xa6:
			fand(A &= ld8(HL));
			if (rofs) pc++;
			break;
			// and reg
#define AND(i) case 0xa0 + (i): fand(A &= REG##i); break;
			SET7(AND)
			// sbc (hl)
			case 0x9e:
			tmp = ld8(HL);
			cy = CY;
			fsub(A, tmp, tmp2 = A - tmp - cy, cy);
			A = tmp2;
			if (rofs) pc++;
			break;
			// sbc reg
#define SBC8(i) case 0x98 + (i): tmp = REG##i; cy = CY; fsub(A, tmp, tmp2 = A - tmp - cy, cy); A = tmp2; break;
			SET7(SBC8)
			// sub (hl)
			case 0x96:
			tmp = ld8(HL);
			fsub(A, tmp, tmp2 = A - tmp, 0);
			A = tmp2;
			if (rofs) pc++;
			break;
			// sub reg
#define SUB(i) case 0x90 + (i): tmp = REG##i; fsub(A, tmp, tmp2 = A - tmp, 0); A = tmp2; break;
			SET7(SUB)
			// adc (hl)
			case 0x8e:
			tmp = ld8(HL);
			cy = CY;
			fadd(A, tmp, tmp2 = A + tmp + cy, cy);
			A = tmp2;
			if (rofs) pc++;
			break;
			// adc reg
#define ADC8(i) case 0x88 + (i): tmp = REG##i; cy = CY; fadd(A, tmp, tmp2 = A + tmp + cy, cy); A = tmp2; break;
			SET7(ADC8)
			// add (hl)
			case 0x86:
			tmp = ld8(HL);
			fadd(A, tmp, tmp2 = A + tmp, 0);
			A = tmp2;
			if (rofs) pc++;
			break;
			// add reg
#define ADD8(i) case 0x80 + (i): tmp = REG##i; fadd(A, tmp, tmp2 = A + tmp, 0); A = tmp2; break;
			SET7(ADD8)
			case 0x76:
			pc--;
			status |= S_HALT;
			break;
			// ld (hl),reg
#define LD_HL_REG(i) case 0x70 + (i): st8(HL, REGFIX##i); if (rofs) pc++; break;
			SET7(LD_HL_REG)
			// ld reg,(hl)
#define LD_REG_HL(i) case 0x46 + 8 * (i): REGFIX##i = ld8(HL); if (rofs) pc++; break;
			SET7(LD_REG_HL)
#define REG_REG(i, j) case 0x40 + (i) + 8 * (j): REG##j = REG##i; break;
			SET77REG(REG_REG)
			case 0x3f: // ccf
			tmp = CY;
			fccf(tmp << LH | !tmp);
			break;
			case 0x37: // scf
			fscf();
			break;
			case 0x2f: // cpl
			A = ~A;
			fcpl();
			break;
			case 0x27: // daa
			if (F & MN) {
				if (F & MH)
					if (CY) tmp = A - 0x66;
					else tmp = A - 6 - 0x60 * (A >= 0x9a);
				else 
					if (CY) tmp = A - 6 * ((A & 0xf) >= 10) - 0x60;
					else tmp = A - 6 * ((A & 0xf) >= 10) - 0x60 * ((A >> 4 & 0xf) >= 10);
				fdaa(((A & 0xf) < 6) << LH | CY | (A >= 0x9a), tmp);
			}
			else {
				if (F & MH)
					if (CY) tmp = A + 0x66;
					else tmp = A + 6 + 0x60 * (A >= 0x9a);
				else 
					if (CY) tmp = A + 6 * ((A & 0xf) >= 10) + 0x60;
					else tmp = A + 6 * ((A & 0xf) >= 10) + 0x60 * ((A >> 4 & 0xf) >= 10);
				fdaa(((A & 0xf) >= 10) << LH | CY | (A >= 0x9a), tmp);
			}
			A = tmp;
			break;
			case 0x1f: // rra
			tmp2 = A;
			A = A >> 1 | CY << 7;
			frot(tmp2);
			break;
			case 0x17: // rla
			tmp2 = A >> 7;
			A = A << 1 | CY;
			frot(tmp2);
			break;
			case 0xf: // rrca
			frot(A);
			A = A >> 1 | A << 7;
			break;
			case 7: // rlca
			frot(tmp2 = A >> 7);
			A = A << 1 | (tmp2 & 1);
			break;
			case 0x36: // ld (hl),n
			if (rofs) {
				st8(HL, imm8n());
				pc += 2;
			}
			else st8(HL, imm8());
			break;
			// ld reg,n
#define LD_REG_N(i) case 6 + 8 * (i): REG##i = imm8(); break;
			SET7(LD_REG_N)
			// dec (hl)
			case 0x35:
			tmp2 = ld8(HL);
			fdec(tmp2, tmp2 - 1);
			st8(HL, tmp2 - 1);
			if (rofs) pc++;
			break;
			// dec reg
#define DEC8(i) case 5 + 8 * (i): tmp2 = REG##i; fdec(tmp2, tmp2 - 1); REG##i = tmp2 - 1; break;
			SET7(DEC8)
			// inc (hl)
			case 0x34:
			tmp2 = ld8(HL);
			finc(tmp2, tmp2 + 1);
			st8(HL, tmp2 + 1);
			if (rofs) pc++;
			break;
			// inc reg
#define INC8(i) case 4 + 8 * (i): tmp2 = REG##i; finc(tmp2, tmp2 + 1); REG##i = tmp2 + 1; break;
			SET7(INC8)
			case 0x3b:
			sp--;
			break;
#define DEC16(i) case 0xb + 8 * (i): (REG16##i)--; break;
			SET3(DEC16)
			case 0x33:
			sp++;
			break;
#define INC16(i) case 3 + 8 * (i): (REG16##i)++; break;
			SET3(INC16)
			case 0x3a: // ld a,(nn)
			A = ld8(imm16());
			break;
			case 0x32: // ld (nn),a
			st8(imm16(), A);
			break;
			case 0x2a: // ld hl,(nn)
			HL = ld16(imm16());
			break;
			case 0x22: // ld (nn),hl
			st16(imm16(), HL);
			break;
			// ld a,(bc)/(de)
#define LD_A_MEM(i) case 0xa + 8 * (i): A = ld8(REG16##i); break;
			SET2(LD_A_MEM)
			// ld (bc)/(de),a
#define LD_MEM_A(i) case 2 + 8 * (i): st8(REG16##i, A); break;
			SET2(LD_MEM_A)
			case 0x39: // add hl,sp
			tmp = sp;
			fadd16(HL, tmp2 = (s32)HL + tmp, 0);
			HL = tmp2;
			break;
			// add hl,bc/de/hl
#define ADD_HL_REG(i) case 9 + 8 * (i): tmp = REG16##i; fadd16(HL, tmp2 = (s32)HL + tmp, 0); HL = tmp2; break;
			SET3(ADD_HL_REG)
			case 0x31:
			sp = imm16();
			break;
#define LD_REG_IMM(i) case 1 + 8 * (i): REG16##i = imm16(); break;
			SET3(LD_REG_IMM)
			case 0x38:
			tmp2 = (s8)imm8();
			if (CY) {
				pc += tmp2;
				clock++;
			}
			break;
			case 0x30:
			tmp2 = (s8)imm8();
			if (!CY) {
				pc += tmp2;
				clock++;
			}
			break;
			case 0x28:
			tmp2 = (s8)imm8();
			if (F & MZ) {
				pc += tmp2;
				clock++;
			}
			break;
			case 0x20:
			tmp2 = (s8)imm8();
			if (!(F & MZ)) {
				pc += tmp2;
				clock++;
			}
			break;
			case 0x18:
			tmp2 = (s8)imm8();
			pc += tmp2;
			break;
			case 0x10:
			tmp2 = (s8)imm8();
			if (--B) pc += tmp2;
			break;
			case 8: // ex af,af'
			swap(A, a_);
			swap(F, f_);
			break;
			case 0: // nop
			break;
		}
		rofs = 0;
		if (status) {
			if (status & S_NMI) {
				status &= ~(S_NMI | S_IFF);
				Iff1 = 0;
				if (status & S_HALT) {
					status &= ~S_HALT;
					pc++;
				}
				st16(sp -= 2, pc);
				pc = 0x66;
			}
			else if (status & S_INT && Iff1) {
				status &= ~(S_INT | S_IFF);
				Iff1 = Iff2 = 0;
				if (status & S_HALT) {
					status &= ~S_HALT;
					pc++;
				}
				switch (IntMode) {
					case 0: // only RST instructions supported
					if ((intdata & 0xc7) == 0xc7) {
						st16(sp -= 2, pc);
						pc = intdata & 0x38;
					}
					break;
					case 1:
					st16(sp -= 2, pc);
					pc = 0x38;
					break;
					case 2:
					st16(sp -= 2, pc);
					pc = ld16(IntVec << 8 | (intdata & 0xff));
					break;
				}
			}
			else if (status & S_IFF) {
				status &= ~S_IFF;
				Iff1 = Iff2 = 1;
			}
			if (status & S_HALT) {
				status &= ~S_HALT;
				emu_exit(); // emulator specific
				break;
			}
		}
#if R800_TRACE
		tracep->r0 = (s32 &)r[0];
		tracep->r1 = (s32 &)r[4];
#if R800_TRACE > 1
		if (++tracep >= tracebuf + TRACEMAX - 1) StopTrace();
#else
		if (++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
#endif
#endif
	}
	while (clock < n);
	return clock - n;
}

void R800::DDCB_FDCB() {
	s32 tmp, tmp2 = imm8n();
	clock += clk_cb[tmp2];
	clock_i = 1;
	switch (tmp2) {
		// rlc reg
#define RLCI(i) case (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | tmp2 & 1, tmp2); st8(HL, REG##i); break;
		SET7(RLCI)
		// rlc (hl)
		case 6:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 = tmp2 << 1 | (tmp & 1), tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
		// rrc reg
#define RRCI(i) case 8 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | tmp2 << 7, tmp2); st8(HL, REG##i); break;
		SET7(RRCI)
		// rrc (hl)
		case 0xe:
		tmp2 = ld8(HL);
		frs(tmp = tmp2 >> 1 | tmp2 << 7, tmp2);
		st8(HL, tmp);
		pc += 2;
		break;
		// rl reg
#define RLI(i) case 0x10 + (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | CY, tmp2); st8(HL, REG##i); break;
		SET7(RLI)
		// rl (hl)
		case 0x16:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 = tmp2 << 1 | CY, tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
		// rr reg
#define RRI(i) case 0x18 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | CY << 7, tmp2); st8(HL, REG##i); break;
		SET7(RRI)
		// rr(hl)
		case 0x1e:
		tmp2 = ld8(HL);
		tmp = tmp2;
		frs(tmp2 = tmp2 >> 1 | CY << 7, tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
		// sla/sll reg
#define SLAI(i) case 0x20 + (i): case 0x30 + (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i <<= 1, tmp2); st8(HL, REG##i); break;
		SET7(SLAI)
		// sla/sll (hl)
		case 0x26: case 0x36:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 <<= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
		// sra reg
#define SRAI(i) case 0x28 + (i): REG##i = ld8(HL); tmp2 = (s8)REG##i; frs(REG##i = tmp2 >> 1, tmp2); st8(HL, REG##i); break;
		SET7(SRAI)
		// sra (hl)
		case 0x2e:
		tmp = tmp2 = (s8)ld8(HL);
		frs(tmp2 >>= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
		// srl reg
#define SRLI(i) case 0x38 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1, tmp2); st8(HL, REG##i); break;
		SET7(SRLI)
		// srl (hl)
		case 0x3e:
		tmp = tmp2 = ld8(HL);
		frs(tmp2 >>= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		break;
#define BITI(i, j) case 0x40 + (i) + 8 * (j): fbit(ld8(HL) >> (j)); pc += 2; break;
		SET77BIT(BITI)
#define BITI7(i) case 0x78 + (i): fbits(ld8(HL)); pc += 2; break;
		SET7(BITI7)
#define BIT_I(j) case 0x46 + 8 * (j): fbit(ld8(HL) >> (j)); pc += 2; break;
		SET7BIT(BIT_I)
		case 0x7e:
		fbits(ld8(HL));
		pc += 2;
		break;
#define RESI(i, j) case 0x80 + (i) + 8 * (j): REG##i = ld8(HL) & ~(1 << (j)); st8(HL, REG##i); break;
		SET78(RESI)
#define RES_I(j) case 0x86 + 8 * (j): st8(HL, ld8(HL) & ~(1 << (j))); pc += 2; break;
		SET_8(RES_I)
#define SETI(i, j) case 0xc0 + (i) + 8 * (j): REG##i = ld8(HL) | 1 << (j); st8(HL, REG##i); break;
		SET78(SETI)
#define SET_I(j) case 0xc6 + 8 * (j): st8(HL, ld8(HL) | 1 << (j)); pc += 2; break;
		SET_8(SET_I)
	}
}

template<int M> void R800::fset(s32 b, s32 a, u16 s, u16 pv) {
	if constexpr ((M & 0xf) == C0) F &= ~MC;
	if constexpr ((M & 0xf) == C1) F |= MC;
	if constexpr ((M & 0xf) == CB) F = (b & MC) | (F & ~MC);
	if constexpr ((M & 0xf) == CADD8) F = b < a && (u8)b >= (u8)a ? F | MC : F & ~MC;
	if constexpr ((M & 0xf) == CSUB8) F = b > a && (u8)b <= (u8)a ? F | MC : F & ~MC;
	if constexpr ((M & 0xf) == CADD16) F = b < a && (u16)b >= (u16)a ? F | MC : F & ~MC;
	if constexpr ((M & 0xf) == CSUB16) F = b > a && (u16)b <= (u16)a ? F | MC : F & ~MC;
	if constexpr ((M & 0xf) == CIO) F = b & ~0xff ? F | MC : F & ~MC;
	if constexpr ((M & 0xf) == CMUL) F = b ? F | MC : F & ~MC;
	if constexpr ((M & 0xf0) == N0) F &= ~MN;
	if constexpr ((M & 0xf0) == N1) F |= MN;
	if constexpr ((M & 0xf0) == NV) F = pv >> 7 ? F | MN : F & ~MN;
	if constexpr ((M & 0xf00) == P0) F &= ~MP;
	if constexpr ((M & 0xf00) == PB) F = (b & MP) | (F & ~MP);
	if constexpr ((M & 0xf00) == PADD8) F = ((b & s & ~a) | (~b & ~s & a)) & 0x80 ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PSUB8) F = ((b & ~s & ~a) | (~b & s & a)) & 0x80 ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PADD16) F = ((b & s & ~a) | (~b & ~s & a)) & 0x8000 ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PSUB16) F = ((b & ~s & ~a) | (~b & s & a)) & 0x8000 ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PPARITY) {
		s32 x = a; x ^= x >> 4; x ^= x << 2; x ^= x >> 1;
		F = (~x & MP) | (F & ~MP);
	}
	if constexpr ((M & 0xf00) == PV) F = pv ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PVZ) F = !(a & 0xff) ? F | MP : F & ~MP;
	if constexpr ((M & 0xf00) == PIO) {
		s32 x = a; x ^= b & 7; x ^= x >> 4; x ^= x << 2; x ^= x >> 1;
		F = (~x & MP) | (F & ~MP);
	}
	if constexpr ((M & 0xf0000) == H0) F &= ~MH;
	if constexpr ((M & 0xf0000) == H1) F |= MH;
	if constexpr ((M & 0xf0000) == HB) F = (b & MH) | (F & ~MH);
	if constexpr ((M & 0xf0000) == HADD8) F = (b & 0xf) > (a & 0xf) || (pv && (b & 0xf) == (a & 0xf)) ? F | MH : F & ~MH;
	if constexpr ((M & 0xf0000) == HSUB8) F = (b & 0xf) < (a & 0xf) || (pv && (b & 0xf) == (a & 0xf)) ? F | MH : F & ~MH;
	if constexpr ((M & 0xf0000) == HADD16) F = (b & 0xfff) > (a & 0xfff) || (pv && (b & 0xfff) == (a & 0xfff)) ? F | MH : F & ~MH;
	if constexpr ((M & 0xf0000) == HSUB16) F = (b & 0xfff) < (a & 0xfff) || (pv && (b & 0xfff) == (a & 0xfff)) ? F | MH : F & ~MH;
	if constexpr ((M & 0xf0000) == HIO) F = b & ~0xff ? F | MH : F & ~MH;
	if constexpr ((M & 0xf000000) == Z1) F |= MZ;
	if constexpr ((M & 0xf000000) == Z8) F = !(a & 0xff) ? F | MZ : F & ~MZ;
	if constexpr ((M & 0xf000000) == Z16) F = !(a & 0xffff) ? F | MZ : F & ~MZ;
	if constexpr ((M & 0xf000000) == Z32) F = !a ? F | MZ : F & ~MZ;
	if constexpr ((M & 0xf0000000) == S0) F &= ~MS;
	if constexpr ((M & 0xf0000000) == S8) F = a & 0x80 ? F | MS : F & ~MS;
	if constexpr ((M & 0xf0000000) == S16) F = a & 0x8000 ? F | MS : F & ~MS;
}

#if R800_TRACE
void R800::StopTrace() {
	TraceBuffer *endp = tracep;
	int i = 0;
	FILE *fo;
	if (!(fo = fopen((std::string(getenv("HOME")) + "/Desktop/trace.txt").c_str(), "w"))) exit(1);
	do {
		if (++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
		fprintf(fo, "%7d %04x %02x\t", i++, tracep->pc, tracep->op);
		fprintf(fo, "%04x %04x %04x %04x ", 
			(u16)tracep->r0, (u16)(tracep->r0 >> 16), (u16)tracep->r1, (u16)(tracep->r1 >> 16));
		switch (tracep->acs1) {
			case acsLoad8:
			fprintf(fo, "L %04x %02x ", tracep->adr1, tracep->data1 & 0xff);
			break;
			case acsLoad16:
			fprintf(fo, "L %04x %04x ", tracep->adr1, tracep->data1);
			break;
			case acsIn:
			fprintf(fo, "I %04x %02x ", tracep->adr1, tracep->data1 & 0xff);
			break;
		}
		switch (tracep->acs2) {
			case acsStore8:
			fprintf(fo, "S %04x %02x ", tracep->adr2, tracep->data2 & 0xff);
			break;
			case acsStore16:
			fprintf(fo, "S %04x %04x ", tracep->adr2, tracep->data2);
			break;
			case acsOut:
			fprintf(fo, "O %04x %02x ", tracep->adr2, tracep->data2 & 0xff);
			break;
		}
		fprintf(fo, "\n");
	} while (tracep != endp);
	fclose(fo);
	fprintf(stderr, "trace dumped.\n");
	exit(1);
}

#endif	// R800_TRACE
