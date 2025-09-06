// R800
// Copyright 2025 © Yasuo Kuwahara
// MIT License

#include <cstdint>
#include <cstdio>

#define R800_TRACE		0

#if R800_TRACE
#define TRACE_LOG1(x) (tracep->acs1 = (x), tracep->adr1 = adr, tracep->data1 = data)
#define TRACE_LOG2(x) (tracep->acs2 = (x), tracep->adr2 = adr, tracep->data2 = data)
#define TRACE_LOG_OP(x) (tracep->op = (x))
#else
#define TRACE_LOG1(x)
#define TRACE_LOG2(x)
#define TRACE_LOG_OP(x)	(x)
#endif

class R800 {
	using s8 = int8_t;
	using u8 = uint8_t;
	using u16 = uint16_t;
	using s32 = int32_t;
	using u32 = uint32_t;
	enum {
		LC, LN, LP, LX, LH, LY, LZ, LS
	};
	enum {
		MC = 1 << LC, MN = 1 << LN, MP = 1 << LP,
		MH = 1 << LH, MZ = 1 << LZ, MS = 1 << LS
	};
	enum {
		F0 = 2, F1, FADD8, FSUB8, FADD16, FSUB16, FB, FPARITY, FV, FVZ, FIO, FA, FMUL, F32,
	};
#define F(flag, type)	flag##type = F##type << (L##flag << 2)
	enum {
		F(C, 0), F(C, 1), F(C, B), F(C, ADD8), F(C, SUB8), F(C, ADD16), F(C, SUB16), F(C, IO), F(C, MUL),
		F(N, 0), F(N, 1), F(N, B), F(N, V),
		F(P, 0), F(P, B), F(P, ADD8), F(P, SUB8), F(P, ADD16), F(P, SUB16), F(P, PARITY), F(P, V), F(P, VZ), F(P, IO),
		F(H, 0), F(H, 1), F(H, B), F(H, ADD8), F(H, SUB8), F(H, ADD16), F(H, SUB16), F(H, IO),
		F(Z, 1), F(Z, B), F(Z, A), Z8 = FADD8 << (LZ << 2), Z16 = FADD16 << (LZ << 2), F(Z, 32),
		F(S, 0), F(S, B), S8 = FADD8 << (LS << 2), S16 = FADD16 << (LS << 2),
	};
#undef F
	enum {
		S_INT, S_NMI, S_IFF = 4, S_HALT = 8
	};
public:
	R800();
	void Reset();
	s32 Execute(s32 n = 1);
	u16 GetPC() const { return pc; }
	void IRQ(u8 data_bus = 0xff) { intdata = data_bus; status |= S_INT; }
	void NMI() { status |= S_NMI; }
	void SetMemoryPtr(u8 *p) { m = p; }
#if R800_TRACE
	void StopTrace();
#endif
private:
	// customized memory access -- start
	s32 imm8() {
		return m[pc++];
	}
	s32 imm8n() {
		return m[pc + 1];
	}
	s32 imm16() {
		u16 r = (u16 &)m[pc];
		pc += 2;
		return r;
	}
	s32 ld8(u16 adr) {
		if (rofs) {
			adr += (s8)m[pc];
			clock += clock_i;
		}
		s32 data = m[adr];
		TRACE_LOG1(acsLoad8);
		return data;
	}
	s32 ld16(u16 adr) {
		s32 data = (u16 &)m[adr];
		TRACE_LOG1(acsLoad16);
		return data;
	}
	void st8(u16 adr, u8 data) {
		if (rofs) {
			adr += (s8)m[pc];
			clock += clock_i;
		}
		m[adr] = data;
		TRACE_LOG2(acsStore8);
	}
	void st16(u16 adr, u16 data) {
		(u16 &)m[adr] = data;
		TRACE_LOG2(acsStore16);
	}
	// customized memory access -- end
	s32 in(u16 adr);
	void out(u16 adr, u8 data);
	virtual bool Extender(u16) { return false; }
	virtual void RETI() {}
	void DDCB_FDCB();
	template<int M> void fset(s32 b, s32 a, u16 s, u16 pv);
	void fmov(s32 a) { fset<S8 | Z8 | H0 | PB | N0>(Iff2 << LP, a, 0, 0); }
	void fbtr(u16 pv) { fset<H0 | PV | N0>(0, 0, 0, pv); }
	void fcp(u16 pv, s32 b, s32 a) { fset<S8 | Z8 | HSUB8 | PV | N1>(b, a, 0, pv); }
	void fadd(s32 b, u16 s, s32 a, u16 pv) { fset<S8 | Z8 | HADD8 | PADD8 | N0 | CADD8>(b, a, s, pv); }
	void fsub(s32 b, u16 s, s32 a, u16 pv) { fset<S8 | Z8 | HSUB8 | PSUB8 | N1 | CSUB8>(b, a, s, pv); }
	void finc(s32 b, s32 a) { fset<S8 | Z8 | HADD8 | PADD8 | N0>(b, a, 0, 1); }
	void fdec(s32 b, s32 a) { fset<S8 | Z8 | HSUB8 | PSUB8 | N1>(b, a, 0, 1); }
	void fand(s32 a) { fset<S8 | Z8 | H1 | PPARITY | N0 | C0>(0, a, 0, 0); }
	void fxor(s32 a) { fset<S8 | Z8 | H0 | PPARITY | N0 | C0>(0, a, 0, 0); }
	void fadd16(s32 b, s32 a, u16 pv) { fset<HADD16 | N0 | CADD16>(b, a, 0, pv); }
	void fadc16(s32 b, s32 a, u16 pv) { fset<S16 | Z16 | HADD16 | PADD16 | N0 | CADD16>(b, a, 0, pv); }
	void fsbc16(s32 b, s32 a, u16 pv) { fset<S16 | Z16 | HSUB16 | PSUB16 | N1 | CSUB16>(b, a, 0, pv); }
	void fdaa(s32 b, s32 a) { fset<S8 | Z8 | HB | PPARITY | CB>(b, a, 0, 0); }
	void fcpl() { fset<H1 | N1>(0, 0, 0, 0); }
	void fccf(s32 b) { fset<HB | N0 | CB>(b, 0, 0, 0); }
	void fscf() { fset<H0 | N0 | C1>(0, 0, 0, 0); }
	void frot(s32 b) { fset<H0 | N0 | CB>(b, 0, 0, 0); }
	void frd(s32 a) { fset<S8 | Z8 | H0 | PPARITY | N0>(0, a, 0, 0); }
	void fbit(s32 a) { fset<S0 | Z8 | H1 | PVZ | N0>(0, a & 1, 0, 0); }
	void fbits(s32 a) { fset<S8 | Z8 | H1 | PVZ | N0>(0, a & 0x80, 0, 0); }
	void frs(s32 a, s32 b) { fset<S8 | Z8 | H0 | PPARITY | N0 | CB>(b, a, 0, 0); }
	void fin(s32 b, s32 a) { fset<S8 | Z8 | H0 | PPARITY | N0>(b, a, 0, 0); }
	void fbio(s32 a, s32 b, u16 pv) { fset<S8 | Z8 | HIO | PIO | NV | CIO>(b, a, 0, pv); }
	void fmulub(s32 b, s32 a) { fset<S0 | Z16 | P0 | CMUL>(b, a, 0, 0); }
	void fmuluw(s32 b, s32 a) { fset<S0 | Z32 | P0 | CMUL>(b, a, 0, 0); }
//
	u8 *m;
	u8 r[24];
	u16 pc, sp, hl;
	u32 bcde, rofs;
	s32 clock, clock_i;
	u8 IntVec, RefReg, RefReg7, Iff1, Iff2, IntMode, a_, f_;
	u8 intdata, status;
#if R800_TRACE
	static constexpr int TRACEMAX = 100;
	enum {
		acsStore8 = 4, acsStore16, acsLoad8, acsLoad16, acsIn, acsOut
	};
	struct TraceBuffer {
		u16 pc; u8 acs1, acs2, op;
		u16 adr1, data1, adr2, data2;
		s32 r0, r1;
	};
	TraceBuffer tracebuf[TRACEMAX];
	TraceBuffer *tracep;
#endif
};
