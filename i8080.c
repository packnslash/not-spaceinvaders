#include "i8080.h"

static const uint8_t OPCODES_CYCLES[256] = {
	//  0  1   2   3   4   5   6   7   8  9   A   B   C   D   E  F
		4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 0
		4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 1
		4, 10, 16, 5,  5,  5,  7,  4,  4, 10, 16, 5,  5,  5,  7, 4,  // 2
		4, 10, 13, 5,  10, 10, 10, 4,  4, 10, 13, 5,  5,  5,  7, 4,  // 3
		5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 4
		5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 5
		5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 6
		7, 7,  7,  7,  7,  7,  7,  7,  5, 5,  5,  5,  5,  5,  7, 5,  // 7
		4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 8
		4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 9
		4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // A
		4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // B
		5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // C
		5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // D
		5, 10, 10, 18, 11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11, // E
		5, 10, 10, 4,  11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11  // F
};

static const char* DISASSEMBLE_TABLE[] = { "nop", "lxi b,#", "stax b", "inx b",
	"inr b", "dcr b", "mvi b,#", "rlc", "ill", "dad b", "ldax b", "dcx b",
	"inr c", "dcr c", "mvi c,#", "rrc", "ill", "lxi d,#", "stax d", "inx d",
	"inr d", "dcr d", "mvi d,#", "ral", "ill", "dad d", "ldax d", "dcx d",
	"inr e", "dcr e", "mvi e,#", "rar", "ill", "lxi h,#", "shld", "inx h",
	"inr h", "dcr h", "mvi h,#", "daa", "ill", "dad h", "lhld", "dcx h",
	"inr l", "dcr l", "mvi l,#", "cma", "ill", "lxi sp,#", "sta $", "inx sp",
	"inr M", "dcr M", "mvi M,#", "stc", "ill", "dad sp", "lda $", "dcx sp",
	"inr a", "dcr a", "mvi a,#", "cmc", "mov b,b", "mov b,c", "mov b,d",
	"mov b,e", "mov b,h", "mov b,l", "mov b,M", "mov b,a", "mov c,b", "mov c,c",
	"mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,M", "mov c,a", "mov d,b",
	"mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a",
	"mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,M",
	"mov e,a", "mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l",
	"mov h,M", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h",
	"mov l,l", "mov l,M", "mov l,a", "mov M,b", "mov M,c", "mov M,d", "mov M,e",
	"mov M,h", "mov M,l", "hlt", "mov M,a", "mov a,b", "mov a,c", "mov a,d",
	"mov a,e", "mov a,h", "mov a,l", "mov a,M", "mov a,a", "add b", "add c",
	"add d", "add e", "add h", "add l", "add M", "add a", "adc b", "adc c",
	"adc d", "adc e", "adc h", "adc l", "adc M", "adc a", "sub b", "sub c",
	"sub d", "sub e", "sub h", "sub l", "sub M", "sub a", "sbb b", "sbb c",
	"sbb d", "sbb e", "sbb h", "sbb l", "sbb M", "sbb a", "ana b", "ana c",
	"ana d", "ana e", "ana h", "ana l", "ana M", "ana a", "xra b", "xra c",
	"xra d", "xra e", "xra h", "xra l", "xra M", "xra a", "ora b", "ora c",
	"ora d", "ora e", "ora h", "ora l", "ora M", "ora a", "cmp b", "cmp c",
	"cmp d", "cmp e", "cmp h", "cmp l", "cmp M", "cmp a", "rnz", "pop b",
	"jnz $", "jmp $", "cnz $", "push b", "adi #", "rst 0", "rz", "ret", "jz $",
	"ill", "cz $", "call $", "aci #", "rst 1", "rnc", "pop d", "jnc $", "out p",
	"cnc $", "push d", "sui #", "rst 2", "rc", "ill", "jc $", "in p", "cc $",
	"ill", "sbi #", "rst 3", "rpo", "pop h", "jpo $", "xthl", "cpo $", "push h",
	"ani #", "rst 4", "rpe", "pchl", "jpe $", "xchg", "cpe $", "ill", "xri #",
	"rst 5", "rp", "pop psw", "jp $", "di", "cp $", "push psw", "ori #",
	"rst 6", "rm", "sphl", "jm $", "ei", "cm $", "ill", "cpi #", "rst 7" };

#define SET_ZSP(c, val) \
	do { \
		c->zf = (val) == 0; \
		c->sf = (val) >> 7; \
		c->pf = parity(val); \
	} while (0)

static inline byte i8080_rb(i8080* const c, word addr)
{
	return c->read_byte(c->userdata, addr);
}

static inline void i8080_wb(i8080* const c, word addr, byte data)
{
	c->write_byte(c->userdata, addr, data);
}

static inline word i8080_rw(i8080* const c, word addr)
{
	return c->read_byte(c->userdata, addr + 1) << 8 |
		c->read_byte(c->userdata, addr);
}

static inline void i8080_ww(i8080* const c, word addr, word data)
{
	c->write_byte(c->userdata, addr, data & 0xFF);
	c->write_byte(c->userdata, addr + 1, data >> 8);
}

static inline byte i8080_nextb(i8080* const c)
{
	return i8080_rb(c, c->pc++);
}

static inline word i8080_nextw(i8080* const c)
{
	word result = i8080_rw(c, c->pc);
	c->pc += 2;
	return result;
}

static inline void i8080_set_bc(i8080* const c, word val)
{
	c->b = val >> 8;
	c->c = val & 0xFF;
}

static inline void i8080_set_de(i8080* const c, word val)
{
	c->d = val >> 8;
	c->e = val & 0xFF;
}

static inline void i8080_set_hl(i8080* const c, word val)
{
	c->h = val >> 8;
	c->l = val & 0xFF;
}

static inline word i8080_get_bc(i8080* const c)
{
	return (c->b << 8) | c->c;
}

static inline word i8080_get_de(i8080* const c)
{
	return (c->d << 8) | c->e;
}

static inline word i8080_get_hl(i8080* const c)
{
	return (c->h << 8) | c->l;
}

static inline void i8080_push(i8080* const c, word val)
{
	c->sp -= 2;
	i8080_ww(c, c->sp, val);
}

static inline word i8080_pop(i8080* const c)
{
	word val = i8080_rw(c, c->sp);
	c->sp += 2;
	return val;
}

static inline bool parity(byte val)
{
	byte nb_one_bits = 0;
	for (int i = 0; i < 8; i++)
		nb_one_bits += ((val >> i) & 1);

	return (nb_one_bits & 1) == 0;
}

static inline bool carry(int bit_no, byte a, byte b, bool cy)
{
	int16_t result = a + b + cy;
	int16_t carry = result ^ a ^ b;
	return carry & (1 << bit_no);
}

static inline void i8080_add(i8080* const c, byte* const reg, byte val, bool cy)
{
	byte result = *reg + val + cy;
	c->cf = carry(8, *reg, val, cy);
	c->hf = carry(4, *reg, val, cy);
	SET_ZSP(c, result);
	*reg = result;
}

static inline void i8080_sub(i8080* const c, byte* const reg, byte val, bool cy)
{
	i8080_add(c, reg, ~val, !cy);
	c->cf = !c->cf;
}

static inline void i8080_dad(i8080* const c, word val)
{
	c->cf = ((i8080_get_hl(c) + val) >> 16) & 1;
	i8080_set_hl(c, i8080_get_hl(c) + val);
}

static inline byte i8080_inr(i8080* const c, byte val)
{
	byte result = val + 1;
	c->hf = (result & 0xF) == 0;
	SET_ZSP(c, result);
	return result;
}

static inline byte i8080_dcr(i8080* const c, byte val)
{
	byte result = val - 1;
	c->hf = !((result & 0xF) == 0xF);
	SET_ZSP(c, result);
	return result;
}

static inline void i8080_ana(i8080* const c, byte val)
{
	byte result = c->a & val;
	c->cf = 0;
	c->hf = ((c->a | val) & 0x08) != 0;
	SET_ZSP(c, result);
	c->a = result;
}

static inline void i8080_xra(i8080* const c, byte val)
{
	c->a ^= val;
	c->cf = 0;
	c->hf = 0;
	SET_ZSP(c, c->a);
}

static inline void i8080_ora(i8080* const c, byte val)
{
	c->a |= val;
	c->cf = 0;
	c->hf = 0;
	SET_ZSP(c, c->a);
}

static inline void i8080_cmp(i8080* const c, byte val)
{
	int16_t result = c->a - val;
	c->cf = result >> 8;
	c->hf = ~(c->a ^ result ^ val) & 0x10;
	SET_ZSP(c, result & 0xFF);
}

static inline void i8080_jmp(i8080* const c, word addr)
{
	c->pc = addr;
}

static inline void i8080_cond_jmp(i8080* const c, bool condition)
{
	word addr = i8080_nextw(c);
	if (condition)
	{
		c->pc = addr;
	}
}

static inline void i8080_call(i8080* const c, word addr)
{
	i8080_push(c, c->pc);
	i8080_jmp(c, addr);
}

static inline void i8080_cond_call(i8080* const c, bool condition)
{
	word addr = i8080_nextw(c);
	if (condition)
	{
		i8080_call(c, addr);
		c->cyc += 6;
	}
}

static inline void i8080_ret(i8080* const c)
{
	c->pc = i8080_pop(c);
}

static inline void i8080_cond_ret(i8080* const c, bool condition)
{
	if (condition)
	{
		i8080_ret(c);
		c->cyc += 6;
	}
}

static inline void i8080_push_psw(i8080* const c)
{
	byte f = 0;
	f |= c->sf << 7;
	f |= c->zf << 6;
	f |= c->hf << 4;
	f |= c->pf << 2;
	f |= 1 << 1;
	f |= c->cf << 0;
	i8080_push(c, c->a << 8 | f);
}

static inline void i8080_pop_psw(i8080* const c)
{
	word af = i8080_pop(c);
	c->a = af >> 8;
	byte f = af & 0xFF;

	c->sf = (f >> 7) & 1;
	c->zf = (f >> 6) & 1;
	c->hf = (f >> 4) & 1;
	c->pf = (f >> 2) & 1;
	c->cf = (f >> 0) & 1;
}

static inline void i8080_xchg(i8080* const c)
{
	word de = i8080_get_de(c);
	i8080_set_de(c, i8080_get_hl(c));
	i8080_set_hl(c, de);
}

static inline void i8080_xthl(i8080* const c)
{
	word val = i8080_rw(c, c->sp);
	i8080_ww(c, c->sp, i8080_get_hl(c));
	i8080_set_hl(c, val);
}

static inline void i8080_rlc(i8080* const c)
{
	c->cf = c->a >> 7;
	c->a = (c->a << 1) | c->cf;
}

static inline void i8080_rrc(i8080* const c)
{
	c->cf = c->a & 1;
	c->a = (c->a >> 1) | (c->cf << 7);
}

static inline void i8080_ral(i8080* const c)
{
	bool cy = c->cf;
	c->cf = c->a >> 7;
	c->a = (c->a << 1) | cy;
}

static inline void i8080_rar(i8080* const c)
{
	bool cy = c->cf;
	c->cf = c->a & 1;
	c->a = (c->a >> 1) | (cy << 7);
}

static inline void i8080_daa(i8080* const c)
{
	bool cy = c->cf;
	byte correction = 0;

	byte lsb = c->a & 0x0F;
	byte msb = c->a >> 4;

	if (c->hf || lsb > 9)
	{
		correction += 0x06;
	}

	if (c->cf || msb > 9 || (msb >= 9 && lsb > 9))
	{
		correction += 0x60;
		cy = 1;
	}

	i8080_add(c, &c->a, correction, 0);
	c->cf = cy;
}

static inline void i8080_execute(i8080* const c, byte opcode)
{
	c->cyc += OPCODES_CYCLES[opcode];

	if (c->interrupt_delay > 0) c->interrupt_delay -= 1;

	switch (opcode)
	{
	case 0x7F: c->a = c->a; break;
	case 0x78: c->a = c->b; break;
	case 0x79: c->a = c->c; break;
	case 0x7A: c->a = c->d; break;
	case 0x7B: c->a = c->e; break;
	case 0x7C: c->a = c->h; break;
	case 0x7D: c->a = c->l; break;
	case 0x7E: c->a = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x0A: c->a = i8080_rb(c, i8080_get_bc(c)); break;
	case 0x1A: c->a = i8080_rb(c, i8080_get_de(c)); break;
	case 0x3A: c->a = i8080_rb(c, i8080_nextw(c)); break;

	case 0x47: c->b = c->a; break;
	case 0x40: c->b = c->b; break;
	case 0x41: c->b = c->c; break;
	case 0x42: c->b = c->d; break;
	case 0x43: c->b = c->e; break;
	case 0x44: c->b = c->h; break;
	case 0x45: c->b = c->l; break;
	case 0x46: c->b = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x4F: c->c = c->a; break;
	case 0x48: c->c = c->b; break;
	case 0x49: c->c = c->c; break;
	case 0x4A: c->c = c->d; break;
	case 0x4B: c->c = c->e; break;
	case 0x4C: c->c = c->h; break;
	case 0x4D: c->c = c->l; break;
	case 0x4E: c->c = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x57: c->d = c->a; break;
	case 0x50: c->d = c->b; break;
	case 0x51: c->d = c->c; break;
	case 0x52: c->d = c->d; break;
	case 0x53: c->d = c->e; break;
	case 0x54: c->d = c->h; break;
	case 0x55: c->d = c->l; break;
	case 0x56: c->d = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x5F: c->e = c->a; break;
	case 0x58: c->e = c->b; break;
	case 0x59: c->e = c->c; break;
	case 0x5A: c->e = c->d; break;
	case 0x5B: c->e = c->e; break;
	case 0x5C: c->e = c->h; break;
	case 0x5D: c->e = c->l; break;
	case 0x5E: c->e = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x67: c->h = c->a; break;
	case 0x60: c->h = c->b; break;
	case 0x61: c->h = c->c; break;
	case 0x62: c->h = c->d; break;
	case 0x63: c->h = c->e; break;
	case 0x64: c->h = c->h; break;
	case 0x65: c->h = c->l; break;
	case 0x66: c->h = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x6F: c->l = c->a; break;
	case 0x68: c->l = c->b; break;
	case 0x69: c->l = c->c; break;
	case 0x6A: c->l = c->d; break;
	case 0x6B: c->l = c->e; break;
	case 0x6C: c->l = c->h; break;
	case 0x6D: c->l = c->l; break;
	case 0x6E: c->l = i8080_rb(c, i8080_get_hl(c)); break;

	case 0x77: i8080_wb(c, i8080_get_hl(c), c->a); break;
	case 0x70: i8080_wb(c, i8080_get_hl(c), c->b); break;
	case 0x71: i8080_wb(c, i8080_get_hl(c), c->c); break;
	case 0x72: i8080_wb(c, i8080_get_hl(c), c->d); break;
	case 0x73: i8080_wb(c, i8080_get_hl(c), c->e); break;
	case 0x74: i8080_wb(c, i8080_get_hl(c), c->h); break;
	case 0x75: i8080_wb(c, i8080_get_hl(c), c->l); break;

	case 0x3E: c->a = i8080_nextb(c); break;
	case 0x06: c->b = i8080_nextb(c); break;
	case 0x0E: c->c = i8080_nextb(c); break;
	case 0x16: c->d = i8080_nextb(c); break;
	case 0x1E: c->e = i8080_nextb(c); break;
	case 0x26: c->h = i8080_nextb(c); break;
	case 0x2E: c->l = i8080_nextb(c); break;
	case 0x36: i8080_wb(c, i8080_get_hl(c), i8080_nextb(c)); break;

	case 0x02: i8080_wb(c, i8080_get_bc(c), c->a); break;
	case 0x12: i8080_wb(c, i8080_get_de(c), c->a); break;
	case 0x32: i8080_wb(c, i8080_nextw(c), c->a); break;

	case 0x01: i8080_set_bc(c, i8080_nextw(c)); break;
	case 0x11: i8080_set_de(c, i8080_nextw(c)); break;
	case 0x21: i8080_set_hl(c, i8080_nextw(c)); break;
	case 0x31: c->sp = i8080_nextw(c); break;
	case 0x2A: i8080_set_hl(c, i8080_rw(c, i8080_nextw(c))); break;
	case 0x22: i8080_ww(c, i8080_nextw(c), i8080_get_hl(c)); break;
	case 0xF9: c->sp = i8080_get_hl(c); break;

	case 0xEB: i8080_xchg(c); break;
	case 0xE3: i8080_xthl(c); break;

	case 0x87: i8080_add(c, &c->a, c->a, 0); break;
	case 0x80: i8080_add(c, &c->a, c->b, 0); break;
	case 0x81: i8080_add(c, &c->a, c->c, 0); break;
	case 0x82: i8080_add(c, &c->a, c->d, 0); break;
	case 0x83: i8080_add(c, &c->a, c->e, 0); break;
	case 0x84: i8080_add(c, &c->a, c->h, 0); break;
	case 0x85: i8080_add(c, &c->a, c->l, 0); break;
	case 0x86: i8080_add(c, &c->a, i8080_rb(c, i8080_get_hl(c)), 0); break;
	case 0xC6: i8080_add(c, &c->a, i8080_nextb(c), 0); break;

	case 0x8F: i8080_add(c, &c->a, c->a, c->cf); break;
	case 0x88: i8080_add(c, &c->a, c->b, c->cf); break;
	case 0x89: i8080_add(c, &c->a, c->c, c->cf); break;
	case 0x8A: i8080_add(c, &c->a, c->d, c->cf); break;
	case 0x8B: i8080_add(c, &c->a, c->e, c->cf); break;
	case 0x8C: i8080_add(c, &c->a, c->h, c->cf); break;
	case 0x8D: i8080_add(c, &c->a, c->l, c->cf); break;
	case 0x8E: i8080_add(c, &c->a, i8080_rb(c, i8080_get_hl(c)), c->cf); break;
	case 0xCE: i8080_add(c, &c->a, i8080_nextb(c), c->cf); break;

	case 0x97: i8080_sub(c, &c->a, c->a, 0); break;
	case 0x90: i8080_sub(c, &c->a, c->b, 0); break;
	case 0x91: i8080_sub(c, &c->a, c->c, 0); break;
	case 0x92: i8080_sub(c, &c->a, c->d, 0); break;
	case 0x93: i8080_sub(c, &c->a, c->e, 0); break;
	case 0x94: i8080_sub(c, &c->a, c->h, 0); break;
	case 0x95: i8080_sub(c, &c->a, c->l, 0); break;
	case 0x96: i8080_sub(c, &c->a, i8080_rb(c, i8080_get_hl(c)), 0); break;
	case 0xD6: i8080_sub(c, &c->a, i8080_nextb(c), 0); break;

	case 0x9F: i8080_sub(c, &c->a, c->a, c->cf); break;
	case 0x98: i8080_sub(c, &c->a, c->b, c->cf); break;
	case 0x99: i8080_sub(c, &c->a, c->c, c->cf); break;
	case 0x9A: i8080_sub(c, &c->a, c->d, c->cf); break;
	case 0x9B: i8080_sub(c, &c->a, c->e, c->cf); break;
	case 0x9C: i8080_sub(c, &c->a, c->h, c->cf); break;
	case 0x9D: i8080_sub(c, &c->a, c->l, c->cf); break;
	case 0x9E: i8080_sub(c, &c->a, i8080_rb(c, i8080_get_hl(c)), c->cf); break;
	case 0xDE: i8080_sub(c, &c->a, i8080_nextb(c), c->cf); break;

	case 0x09: i8080_dad(c, i8080_get_bc(c)); break;
	case 0x19: i8080_dad(c, i8080_get_de(c)); break;
	case 0x29: i8080_dad(c, i8080_get_hl(c)); break;
	case 0x39: i8080_dad(c, c->sp); break;

	case 0xF3: c->iff = 0; break;
	case 0xFB: c->iff = 1; c->interrupt_delay = 1; break;
	case 0x00: break;
	case 0x76: c->halted = 1; break;

	case 0x3C: c->a = i8080_inr(c, c->a); break;
	case 0x04: c->b = i8080_inr(c, c->b); break;
	case 0x0C: c->c = i8080_inr(c, c->c); break;
	case 0x14: c->d = i8080_inr(c, c->d); break;
	case 0x1C: c->e = i8080_inr(c, c->e); break;
	case 0x24: c->h = i8080_inr(c, c->h); break;
	case 0x2C: c->l = i8080_inr(c, c->l); break;
	case 0x34: i8080_wb(c, i8080_get_hl(c), i8080_inr(c, i8080_rb(c, i8080_get_hl(c)))); break;

	case 0x3D: c->a = i8080_dcr(c, c->a); break;
	case 0x05: c->b = i8080_dcr(c, c->b); break;
	case 0x0D: c->c = i8080_dcr(c, c->c); break;
	case 0x15: c->d = i8080_dcr(c, c->d); break;
	case 0x1D: c->e = i8080_dcr(c, c->e); break;
	case 0x25: c->h = i8080_dcr(c, c->h); break;
	case 0x2D: c->l = i8080_dcr(c, c->l); break;
	case 0x35: i8080_wb(c, i8080_get_hl(c), i8080_dcr(c, i8080_rb(c, i8080_get_hl(c)))); break;

	case 0x03: i8080_set_bc(c, i8080_get_bc(c) + 1); break;
	case 0x13: i8080_set_de(c, i8080_get_de(c) + 1); break;
	case 0x23: i8080_set_hl(c, i8080_get_hl(c) + 1); break;
	case 0x33: c->sp += 1; break;

	case 0x0B: i8080_set_bc(c, i8080_get_bc(c) - 1); break;
	case 0x1B: i8080_set_de(c, i8080_get_de(c) - 1); break;
	case 0x2B: i8080_set_hl(c, i8080_get_hl(c) - 1); break;
	case 0x3B: c->sp -= 1; break;

	case 0x27: i8080_daa(c); break;
	case 0x2F: c->a = ~c->a; break;
	case 0x37: c->cf = 1; break;
	case 0x3F: c->cf = !c->cf; break;

	case 0x07: i8080_rlc(c); break;
	case 0x0F: i8080_rrc(c); break;
	case 0x17: i8080_ral(c); break;
	case 0x1F: i8080_rar(c); break;

	case 0xA7: i8080_ana(c, c->a); break;
	case 0xA0: i8080_ana(c, c->b); break;
	case 0xA1: i8080_ana(c, c->c); break;
	case 0xA2: i8080_ana(c, c->d); break;
	case 0xA3: i8080_ana(c, c->e); break;
	case 0xA4: i8080_ana(c, c->h); break;
	case 0xA5: i8080_ana(c, c->l); break;
	case 0xA6: i8080_ana(c, i8080_rb(c, i8080_get_hl(c))); break;
	case 0xE6: i8080_ana(c, i8080_nextb(c)); break;

	case 0xAF: i8080_xra(c, c->a); break;
	case 0xA8: i8080_xra(c, c->b); break;
	case 0xA9: i8080_xra(c, c->c); break;
	case 0xAA: i8080_xra(c, c->d); break;
	case 0xAB: i8080_xra(c, c->e); break;
	case 0xAC: i8080_xra(c, c->h); break;
	case 0xAD: i8080_xra(c, c->l); break;
	case 0xAE: i8080_xra(c, i8080_rb(c, i8080_get_hl(c))); break;
	case 0xEE: i8080_xra(c, i8080_nextb(c)); break;

	case 0xB7: i8080_ora(c, c->a); break;
	case 0xB0: i8080_ora(c, c->b); break;
	case 0xB1: i8080_ora(c, c->c); break;
	case 0xB2: i8080_ora(c, c->d); break;
	case 0xB3: i8080_ora(c, c->e); break;
	case 0xB4: i8080_ora(c, c->h); break;
	case 0xB5: i8080_ora(c, c->l); break;
	case 0xB6: i8080_ora(c, i8080_rb(c, i8080_get_hl(c))); break;
	case 0xF6: i8080_ora(c, i8080_nextb(c)); break;

	case 0xBF: i8080_cmp(c, c->a); break;
	case 0xB8: i8080_cmp(c, c->b); break;
	case 0xB9: i8080_cmp(c, c->c); break;
	case 0xBA: i8080_cmp(c, c->d); break;
	case 0xBB: i8080_cmp(c, c->e); break;
	case 0xBC: i8080_cmp(c, c->h); break;
	case 0xBD: i8080_cmp(c, c->l); break;
	case 0xBE: i8080_cmp(c, i8080_rb(c, i8080_get_hl(c))); break;
	case 0xFE: i8080_cmp(c, i8080_nextb(c)); break;

	case 0xC3: i8080_jmp(c, i8080_nextw(c)); break;
	case 0xC2: i8080_cond_jmp(c, c->zf == 0); break;
	case 0xCA: i8080_cond_jmp(c, c->zf == 1); break;
	case 0xD2: i8080_cond_jmp(c, c->cf == 0); break;
	case 0xDA: i8080_cond_jmp(c, c->cf == 1); break;
	case 0xE2: i8080_cond_jmp(c, c->pf == 0); break;
	case 0xEA: i8080_cond_jmp(c, c->pf == 1); break;
	case 0xF2: i8080_cond_jmp(c, c->sf == 0); break;
	case 0xFA: i8080_cond_jmp(c, c->sf == 1); break;

	case 0xE9: c->pc = i8080_get_hl(c); break;
	case 0xCD: i8080_call(c, i8080_nextw(c)); break;
	case 0xC4: i8080_cond_call(c, c->zf == 0); break;
	case 0xCC: i8080_cond_call(c, c->zf == 1); break;
	case 0xD4: i8080_cond_call(c, c->cf == 0); break;
	case 0xDC: i8080_cond_call(c, c->cf == 1); break;
	case 0xE4: i8080_cond_call(c, c->pf == 0); break;
	case 0xEC: i8080_cond_call(c, c->pf == 1); break;
	case 0xF4: i8080_cond_call(c, c->sf == 0); break;
	case 0xFC: i8080_cond_call(c, c->sf == 1); break;

	case 0xC9: i8080_ret(c); break;
	case 0xC0: i8080_cond_ret(c, c->zf == 0); break;
	case 0xC8: i8080_cond_ret(c, c->zf == 1); break;
	case 0xD0: i8080_cond_ret(c, c->cf == 0); break;
	case 0xD8: i8080_cond_ret(c, c->cf == 1); break;
	case 0xE0: i8080_cond_ret(c, c->pf == 0); break;
	case 0xE8: i8080_cond_ret(c, c->pf == 1); break;
	case 0xF0: i8080_cond_ret(c, c->sf == 0); break;
	case 0xF8: i8080_cond_ret(c, c->sf == 1); break;

	case 0xC7: i8080_call(c, 0x00); break;
	case 0xCF: i8080_call(c, 0x08); break;
	case 0xD7: i8080_call(c, 0x10); break;
	case 0xDF: i8080_call(c, 0x18); break;
	case 0xE7: i8080_call(c, 0x20); break;
	case 0xEF: i8080_call(c, 0x28); break;
	case 0xF7: i8080_call(c, 0x30); break;
	case 0xFF: i8080_call(c, 0x38); break;
	
	case 0xC5: i8080_push(c, i8080_get_bc(c)); break;
	case 0xD5: i8080_push(c, i8080_get_de(c)); break;
	case 0xE5: i8080_push(c, i8080_get_hl(c)); break;
	case 0xF5: i8080_push_psw(c); break;
	case 0xC1: i8080_set_bc(c, i8080_pop(c)); break;
	case 0xD1: i8080_set_de(c, i8080_pop(c)); break;
	case 0xE1: i8080_set_hl(c, i8080_pop(c)); break;
	case 0xF1: i8080_pop_psw(c); break;

	case 0xDB: c->a = c->port_in(c->userdata, i8080_nextb(c)); break;
	case 0xD3: c->port_out(c->userdata, i8080_nextb(c), c->a); break;

	case 0x08:
	case 0x10:
	case 0x18:
	case 0x20:
	case 0x28:
	case 0x30:
	case 0x38: break;

	case 0xD9: i8080_ret(c); break;

	case 0xDD:
	case 0xED:
	case 0xFD: i8080_call(c, i8080_nextw(c)); break;

	case 0xCB: i8080_jmp(c, i8080_nextw(c)); break;
	}
}

void i8080_init(i8080* const c)
{
	c->read_byte = NULL;
	c->write_byte = NULL;
	c->port_in = NULL;
	c->port_out = NULL;

	c->cyc = 0;

	c->pc = 0;
	c->sp = 0;

	c->a = 0;
	c->b = 0;
	c->c = 0;
	c->d = 0;
	c->e = 0;
	c->h = 0;
	c->l = 0;

	c->sf = 0;
	c->zf = 0;
	c->hf = 0;
	c->pf = 0;
	c->cf = 0;
	c->iff = 0;

	c->halted = 0;
	c->interrupt_pending = 0;
	c->interrupt_vector = 0;
	c->interrupt_delay = 0;
}

void i8080_step(i8080* const c)
{
	if (c->interrupt_pending && c->iff && c->interrupt_delay == 0)
	{
		c->interrupt_pending = 0;
		c->iff = 0;
		c->halted = 0;

		i8080_execute(c, c->interrupt_vector);
	}
	else if (!c->halted)
	{
		i8080_execute(c, i8080_nextb(c));
	}
}

void i8080_interrupt(i8080* const c, byte opcode)
{
	c->interrupt_pending = 1;
	c->interrupt_vector = opcode;
}

void i8080_debug(i8080* const c, bool disassemble)
{
	byte f = 0;

	f |= c->sf << 7;
	f |= c->zf << 6;
	f |= c->hf << 4;
	f |= c->pf << 2;
	f |= 1 << 1;
	f |= c->cf << 0;

	printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
		c->pc, c->a << 8 | f, c->b << 8 | c->c, c->d << 8 | c->e, c->h << 8 | c->l, c->sp, c->cyc);

	printf("\t(%02X %02X %02X %02X)", i8080_rb(c, c->pc), i8080_rb(c, c->pc + 1),
		i8080_rb(c, c->pc + 2), i8080_rb(c, c->pc + 4));

	if (disassemble)
		printf(" - %s", DISASSEMBLE_TABLE[i8080_rb(c, c->pc)]);

	printf("\n");
}

#undef SET_ZSP