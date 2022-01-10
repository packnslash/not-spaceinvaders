#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "i8080.h"

#define MEMORY_SIZE 0x10000
static byte* memory = NULL;
static bool test_finished = 0;

static byte rb(void* userdata, word addr)
{
	return memory[addr];
}

static void wb(void* userdata, word addr, byte data)
{
	memory[addr] = data;
}

static byte port_in(void* userdata, byte port)
{
	return 0x00;
}

static void port_out(void* userdata, byte port, byte value)
{
	i8080* const c = (i8080*)userdata;

	if (port == 0)
	{
		test_finished = 1;
	}
	else if (port == 1)
	{
		byte operation = c->c;

		if (operation == 2)
		{
			printf("%c", c->e);
		}
		else if (operation == 9)
		{
			word addr = (c->d << 8) | c->e;
			do {
				printf("%c", rb(c, addr++));
			} while (rb(c, addr) != '$');
		}
	}
}

static inline int load_file(const char* filename, word addr)
{
	FILE* f = fopen(filename, "rb");

	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	rewind(f);

	size_t result = fread(&memory[addr], sizeof(byte), file_size, f);

	fclose(f);
	return 0;
}

static inline void run_test(i8080* const c, const char* filename, unsigned long cyc_expected)
{
	i8080_init(c);

	c->userdata = c;
	c->read_byte = rb;
	c->write_byte = wb;
	c->port_in = port_in;
	c->port_out = port_out;

	memset(memory, 0, MEMORY_SIZE);

	if (load_file(filename, 0x100) != 0) return;

	printf("*** TEST: %s\n", filename);

	c->pc = 0x100;

	memory[0x0000] = 0xD3;
	memory[0x0001] = 0x00;

	memory[0x0005] = 0xD3;
	memory[0x0006] = 0x01;
	memory[0x0007] = 0xC9;

	long nb_instructions = 0;

	test_finished = 0;
	while (!test_finished)
	{
		nb_instructions += 1;

		// i8080_debug(c, false);

		i8080_step(c);
	}

	long long diff = cyc_expected - c->cyc;
	printf("\n*** %lu instructions executed on %lu cycles"
		" (expected=%lu, diff=%lld)\n\n",
		nb_instructions, c->cyc, cyc_expected, diff);
}

/*
int main(int argc, char** argv)
{
	memory = (byte*)malloc(MEMORY_SIZE);
	if (memory == NULL) return 1;

	i8080 cpu;
	run_test(&cpu, "TST8080.COM", 4924LU);
	run_test(&cpu, "CPUTEST.COM", 255653383LU);
	run_test(&cpu, "8080PRE.COM", 7817LU);
	run_test(&cpu, "8080EXM.COM", 23803381171LU);

	free(memory);

	return 0;
}
*/