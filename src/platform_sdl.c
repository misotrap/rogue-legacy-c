/* Development platform: an SDL2 window on the computer (2x scale).
 * Keys: arrows, Z = jump (B), X = attack (Y), C = A, V = X,
 * A/S = L1/R1, Q/W = L2/R2, Enter = Start, Right Shift = Select,
 * Escape = Menu (quit).
 */
#include "platform.h"
#include <SDL.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static uint32_t frame[SCREEN_W * SCREEN_H];

int plat_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;
	window = SDL_CreateWindow("Rogue Legacy C", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * 2, SCREEN_H * 2, 0);
	if (window == NULL) return 0;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) return 0;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
	return texture != NULL;
}

void plat_quit(void)
{
	SDL_Quit();
}

uint32_t *plat_frame(void)
{
	return frame;
}

void plat_present(void)
{
	SDL_UpdateTexture(texture, NULL, frame, SCREEN_W * 4);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

static uint32_t KeyBit(SDL_Keycode k)
{
	switch (k)
	{
	case SDLK_UP: return BTN_UP;
	case SDLK_DOWN: return BTN_DOWN;
	case SDLK_LEFT: return BTN_LEFT;
	case SDLK_RIGHT: return BTN_RIGHT;
	case SDLK_z: return BTN_B;
	case SDLK_x: return BTN_Y;
	case SDLK_c: return BTN_A;
	case SDLK_v: return BTN_X;
	case SDLK_a: return BTN_L1;
	case SDLK_s: return BTN_R1;
	case SDLK_q: return BTN_L2;
	case SDLK_w: return BTN_R2;
	case SDLK_RETURN: return BTN_START;
	case SDLK_RSHIFT: return BTN_SELECT;
	case SDLK_ESCAPE: return BTN_MENU;
	default: return 0;
	}
}

int plat_poll(uint32_t *buttons)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) return 0;
		if ((e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) && !e.key.repeat)
		{
			uint32_t bit = KeyBit(e.key.keysym.sym);
			if (e.type == SDL_KEYDOWN) *buttons |= bit;
			else *buttons &= ~bit;
		}
	}
	return !(*buttons & BTN_MENU);
}

double plat_time(void)
{
	return SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency();
}

void plat_sleep(double seconds)
{
	if (seconds > 0) SDL_Delay((Uint32) (seconds * 1000.0));
}
