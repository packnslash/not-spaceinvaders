#ifndef _INVADERS_H_
#define _INVADERS_H_

#include <string.h>
#include <SDL.h>

#include "i8080.h"

#define SCREEN_WIDTH 224
#define SCREEN_HEIGHT 256
#define FPS 59.541985
#define CLOCK_SPEED 1996800
#define CYCLES_PER_FRAME (CLOCK_SPEED / FPS)

#define VRAM_ADDR 0x2400

typedef struct invaders invaders;
struct invaders {
	i8080 cpu;
	byte memory[0x4000];

	byte next_interrupt;
	bool colored_screen;
	
	byte port1, port2;
	byte shift_msb, shift_lsb, shift_offset;
	byte last_out_port3, last_out_port5;

	byte screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH][4];
	void (*update_screen)(invaders* const si);
};

void invaders_init(invaders* const si);
void invaders_update(invaders* const si, int ms);
void invaders_render(invaders* const si);
int invaders_load_rom(invaders* const si, const char* filename, word start_addr);

#endif
