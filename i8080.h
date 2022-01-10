#ifndef I8080_I8080_H_
#define I8080_I8080_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t word;
typedef uint8_t byte;

typedef struct i8080 {

	byte(*read_byte)(void*, word);
	void(*write_byte)(void*, word, byte);
	byte(*port_in)(void*, byte);
	void(*port_out)(void*, byte, byte);
	void* userdata;

	unsigned long cyc;

	word pc;
	word sp;
	byte a, b, c, d, e, h, l;
	bool sf : 1, zf : 1, hf : 1, pf : 1, cf : 1, iff : 1;
	bool halted : 1;
	bool interrupt_pending : 1;
	byte interrupt_vector;
	byte interrupt_delay;

} i8080;

void i8080_init(i8080* const c);
void i8080_step(i8080* const c);
void i8080_interrupt(i8080* const c, byte opcode);
void i8080_debug(i8080* const c, bool disassemble);

#endif