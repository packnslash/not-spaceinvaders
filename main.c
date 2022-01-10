#include <SDL.h>

#include "invaders.h"

#define JOYSTICK_DEAD_ZONE 8000

static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_Event e;

static invaders si;

static bool should_quit = false;
static int speed = 1;
static uint32_t current_time = 0;
static uint32_t last_time = 0;
static uint32_t dt = 0;

static void update_screen(invaders* const si)
{
	int pitch = 0;
	void* pixels = NULL;
	if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0)
	{
		SDL_Log("Unable to lock texture: %s", SDL_GetError());
	}
	else
	{
		memcpy(pixels, si->screen_buffer, pitch * SCREEN_HEIGHT);
	}
	SDL_UnlockTexture(texture);
}

void mainloop(void)
{
	current_time = SDL_GetTicks();
	dt = current_time - last_time;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			should_quit = true;
		}
		else if (e.type == SDL_KEYDOWN)
		{
			SDL_Scancode key = e.key.keysym.scancode;
			switch (key)
			{
			case SDL_SCANCODE_C: si.port1 |= 1 << 0; break;
			case SDL_SCANCODE_2: si.port1 |= 1 << 1; break;
			case SDL_SCANCODE_RETURN: si.port1 |= 1 << 2; break;
			case SDL_SCANCODE_SPACE: si.port1 |= 1 << 4; si.port2 |= 1 << 4; break;
			case SDL_SCANCODE_LEFT: si.port1 |= 1 << 5; si.port2 |= 1 << 5; break;
			case SDL_SCANCODE_RIGHT: si.port1 |= 1 << 6; si.port2 |= 1 << 6; break;
			case SDL_SCANCODE_T: si.port2 |= 1 << 2; break;
			case SDL_SCANCODE_G: si.colored_screen = !si.colored_screen; break;
			case SDL_SCANCODE_ESCAPE: break;
			case SDL_SCANCODE_TAB: speed = 5; break;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			SDL_Scancode key = e.key.keysym.scancode;
			switch (key)
			{
			case SDL_SCANCODE_C: si.port1 &= 0b11111110; break;
			case SDL_SCANCODE_2: si.port1 &= 0b11111101; break;
			case SDL_SCANCODE_RETURN: si.port1 &= 0b11111011; break;
			case SDL_SCANCODE_SPACE: si.port1 &= 0b11101111; si.port2 &= 0b11101111; break;
			case SDL_SCANCODE_LEFT: si.port1 &= 0b11011111; si.port2 &= 0b11011111; break;
			case SDL_SCANCODE_RIGHT: si.port1 &= 0b10111111; si.port2 &= 0b10111111; break;
			case SDL_SCANCODE_T: si.port2 &= 0b11111011; break;
			case SDL_SCANCODE_TAB: speed = 1; break;
			}
		}
		else if (e.type == SDL_JOYAXISMOTION)
		{
			if (e.jaxis.axis == 0)
			{
				if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
				{
					si.port1 |= 1 << 5;
					si.port2 |= 1 << 5;
				}
				else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
				{
					si.port1 |= 1 << 6;
					si.port2 |= 1 << 6;
				}
				else
				{
					si.port1 &= 0b11011111;
					si.port2 &= 0b11011111;

					si.port1 &= 0b10111111;
					si.port2 &= 0b10111111;
				}
			}
		}
		else if (e.type == SDL_JOYBUTTONDOWN)
		{
			if (e.jbutton.button == 1)
			{
				si.port1 |= 1 << 0;
			}
			else if (e.jbutton.button == 0)
			{
				si.port1 |= 1 << 4;
				si.port2 |= 1 << 4;
			}
			else if (e.jbutton.button == 8)
			{
				si.port1 |= 1 << 2;
			}
			else if (e.jbutton.button == 9)
			{
				si.port1 |= 1 << 1;
			}
			else if (e.jbutton.button == 13)
			{
				si.port1 |= 1 << 5;
				si.port2 |= 1 << 5;
			}
			else if (e.jbutton.button == 0)
			{
				si.port1 |= 1 << 6;
				si.port2 |= 1 << 6;
			}
			else if (e.jbutton.button == 4)
			{
				si.colored_screen = !si.colored_screen;
			}
		}
		else if (e.type == SDL_JOYBUTTONUP)
		{
			if (e.jbutton.button == 1)
			{
				si.port1 &= 0b11111110;
			}
			else if (e.jbutton.button == 0)
			{
				si.port1 &= 0b11101111;
				si.port2 &= 0b11101111;
			}
			else if (e.jbutton.button == 8)
			{
				si.port1 &= 0b11111011;
			}
			else if (e.jbutton.button == 9)
			{
				si.port1 &= 0b11111101;
			}
			else if (e.jbutton.button == 13)
			{
				si.port1 &= 0b11011111;
				si.port2 &= 0b11011111;
			}
			else if (e.jbutton.button == 0)
			{
				si.port1 &= 0b10111111;
				si.port2 &= 0b10111111;
			}
		}
	}

	invaders_update(&si, dt * speed);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	last_time = current_time;
}

int main(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
	{
		SDL_Log("error");
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_SHOWN);

	if (window == NULL)
	{
		SDL_Log("error");
		return 1;
	}

	SDL_SetWindowMinimumSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL)
	{
		SDL_Log("error");
		return 1;
	}

	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	if (texture == NULL)
	{
		SDL_Log("error");
		return 1;
	}

	SDL_Joystick* joystick = NULL;
	if (SDL_NumJoysticks() > 0)
	{
		joystick = SDL_JoystickOpen(0);
		if (joystick)
		{
			SDL_Log("joystick 0");
		}
		else
		{
			SDL_Log("error");
		}
	}

	invaders_init(&si);
	si.update_screen = update_screen;
	update_screen(&si);

	if (invaders_load_rom(&si, "invaders.concatenated", 0x0000) != 0) return 1;

	while (!should_quit)
	{
		mainloop();
	}

	if (joystick) {
		SDL_JoystickClose(joystick);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}