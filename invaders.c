#include "invaders.h"

static byte rb(void* userdata, word addr)
{
	invaders* const si = (invaders*)userdata;

	if (addr >= 0x6000) return 0;

	if (addr >= 0x4000 && addr < 0x6000)
		addr -= 0x2000;

	return si->memory[addr];
}

static void wb(void* userdata, word addr, byte data)
{
	invaders* const si = (invaders*)userdata;

	if (addr >= 0x2000 && addr < 0x4000)
		si->memory[addr] = data;
}

static byte port_in(void* userdata, byte port)
{
	invaders* const si = (invaders*)userdata;

	byte value = 0xff;

	switch (port)
	{
	case 0: break;
	case 1:
		value = si->port1;
		break;
	case 2:
		value = si->port2;
		break;
	case 3: {
		const word shift = (si->shift_msb << 8) | si->shift_lsb;
		value = (shift >> (8 - si->shift_offset)) & 0xff;
	} break;

	default: fprintf(stderr, "error: unknown IN port %02X\n", port); break;
	}

	return value;
}

static void port_out(void* userdata, byte port, byte value)
{
	invaders* const si = (invaders*)userdata;

	switch (port)
	{
	case 2:
		si->shift_offset = value & 7;
		break;
	case 3:
		// play sound
		break;
	case 4:
		si->shift_lsb = si->shift_msb;
		si->shift_msb = value;
		break;
	case 5:
		// play sound
		break;
	case 6: break;

	default: fprintf(stderr, "error: unknown OUT port %02X\n", port); break;
	}
}

void invaders_init(invaders* const si)
{
	i8080_init(&si->cpu);
	si->cpu.userdata = si;
	si->cpu.read_byte = rb;
	si->cpu.write_byte = wb;
	si->cpu.port_in = port_in;
	si->cpu.port_out = port_out;

	memset(si->memory, 0, sizeof(si->memory));
	memset(si->screen_buffer, 0, sizeof(si->screen_buffer));
	si->next_interrupt = 0xcf;

	si->port1 = 0;
	si->port2 = 0;
	si->shift_msb = 0;
	si->shift_lsb = 0;
	si->shift_offset = 0;
	si->last_out_port3 = 0;
	si->last_out_port5 = 0;
	si->colored_screen = true;
	si->update_screen = NULL;

	// load sounds
}

void invaders_update(invaders* const si, int ms)
{
	int count = 0;
	while (count < ms * CLOCK_SPEED / 1000)
	{
		int cyc = si->cpu.cyc;
		i8080_step(&si->cpu);
		int elapsed = si->cpu.cyc - cyc;
		count += elapsed;

		if (si->cpu.cyc >= CYCLES_PER_FRAME / 2)
		{
			si->cpu.cyc -= CYCLES_PER_FRAME / 2;

			i8080_interrupt(&si->cpu, si->next_interrupt);
			if (si->next_interrupt == 0xd7)
			{
				invaders_render(si);
			}
			si->next_interrupt = si->next_interrupt == 0xcf ? 0xd7 : 0xcf;
		}
	}
}

void invaders_render(invaders* const si)
{
	for (int i = 0; i < 256 * 224 / 8; i++)
	{
		const int y = i * 8 / 256;
		const int base_x = (i * 8) % 256;
		const byte cur_byte = si->memory[VRAM_ADDR + i];

		for (byte bit = 0; bit < 8; bit++)
		{
			int px = base_x + bit;
			int py = y;
			const bool is_pixel_lit = (cur_byte >> bit) & 1;
			byte r = 0, g = 0, b = 0;

			if (!si->colored_screen && is_pixel_lit)
			{
				r = g = b = 255;
			}
			else if (si->colored_screen && is_pixel_lit)
			{
				if (px < 16)
				{
					if (py < 16 || py > 118 + 16)
					{
						r = g = b = 255;
					}
					else {
						g = 255;
					}
				}
				else if (px >= 16 && px <= 16 + 56)
				{
					g = 255;
				}
				else if (px >= 16 + 56 + 120 && px < 16 + 56 + 120 + 32)
				{
					r = 255;
				}
				else
				{
					r = b = g = 255;
				}
			}

			const int temp_x = px;
			px = py;
			py = -temp_x + SCREEN_HEIGHT - 1;

			si->screen_buffer[py][px][0] = r;
			si->screen_buffer[py][px][1] = g;
			si->screen_buffer[py][px][2] = b;
		}
	}

	si->update_screen(si);
}

int invaders_load_rom(invaders* const si, const char* filename, word start_addr)
{
	SDL_RWops* f = SDL_RWFromFile(filename, "rb");
	if (f == NULL)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "cant open rom file %s", filename);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invaders error", "cant open rom file", NULL);
		return 1;
	}

	Sint64 file_size = SDL_RWsize(f);

	if (file_size > 0x3200)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "rom file '%s' is too big to fit in memory", filename);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invaders error", "rom file is too big to fit in memory", NULL);
		return 1;
	}

	SDL_RWread(f, &si->memory[start_addr], 1, file_size);

	SDL_RWclose(f);
	return 0;
}