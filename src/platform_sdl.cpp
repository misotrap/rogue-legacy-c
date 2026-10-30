/* Development platform: an SDL2 window on the computer (2x scale).
 * Keys: arrows, Z = jump (B), X = attack (Y), C = A, V = X,
 * A/S = L1/R1, Q/W = L2/R2, Enter = Start, Right Shift = Select,
 * Escape = Menu (quit).
 */
#include "platform.h"
#include "gfx.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static uint32_t frame[SCREEN_W * SCREEN_H];

/* Dev-only headless testing: RL_SHOT=path saves the frame as a PPM and
 * quits RL_SHOT_MS later; RL_BTN=<hex bitmask> fakes held buttons. */
static const char *shotPath;
static double shotAt = -1.0;
static uint32_t fakeButtons = 0, fakeButtons2 = 0, fakeButtons3 = 0, fakeButtons4 = 0;
static double buttonsAt = 0.0, buttons2At = 0.0, buttons3At = 0.0, buttons4At = 0.0;
/* RL_BTN_PULSE: the mask pressed ~0.1 s every RL_BTN_PULSE_EVERY from
 * RL_BTN_PULSE_MS, for the many-press dialogues of the tests. */
static uint32_t pulseButtons = 0;
static double pulseAt = 0.0, pulseEvery = 0.6, pulseNext = 0.0, pulseUntil = 0.0;
static int quitAfterShot;
static double startTime;   /* performance counter at init: plat_time is
                            * seconds since start (platform.h contract) */

int plat_init(void)
{
	const char *s;
	if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;
	window = SDL_CreateWindow("Rogue Legacy C", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * 2, SCREEN_H * 2, 0);
	if (window == NULL) return 0;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) return 0;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
	startTime = SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency();
	shotPath = getenv("RL_SHOT");
	s = getenv("RL_SHOT_MS");
	if (shotPath != NULL && s != NULL) shotAt = atof(s) / 1000.0;
	s = getenv("RL_BTN");
	if (s != NULL) fakeButtons = (uint32_t) strtoul(s, NULL, 0);
	s = getenv("RL_BTN_MS");
	if (s != NULL) buttonsAt = atof(s) / 1000.0;
	s = getenv("RL_BTN2");
	if (s != NULL) fakeButtons2 = (uint32_t) strtoul(s, NULL, 0);
	s = getenv("RL_BTN2_MS");
	if (s != NULL) buttons2At = atof(s) / 1000.0;
	s = getenv("RL_BTN3");
	if (s != NULL) fakeButtons3 = (uint32_t) strtoul(s, NULL, 0);
	s = getenv("RL_BTN3_MS");
	if (s != NULL) buttons3At = atof(s) / 1000.0;
	s = getenv("RL_BTN4");
	if (s != NULL) fakeButtons4 = (uint32_t) strtoul(s, NULL, 0);
	s = getenv("RL_BTN4_MS");
	if (s != NULL) buttons4At = atof(s) / 1000.0;
	s = getenv("RL_BTN_PULSE");
	if (s != NULL) pulseButtons = (uint32_t) strtoul(s, NULL, 0);
	s = getenv("RL_BTN_PULSE_MS");
	if (s != NULL) pulseAt = atof(s) / 1000.0;
	s = getenv("RL_BTN_PULSE_EVERY");
	if (s != NULL) pulseEvery = atof(s) / 1000.0;
	quitAfterShot = getenv("RL_SHOT_QUIT") != NULL;
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
	/* Dev: the draw target must be the real frame here; a redirect left
	 * over (the room cache build) means the frame was never completed. */
	if (gfx_frame() != frame)
	{
		static const bool warned = []() { return getenv("RL_TRACE_TARGET") != nullptr; }();
		if (warned)
			fprintf(stderr, "target leaked at %.2f: gfx_frame=%p real=%p\n",
				plat_time(), (void *) gfx_frame(), (void *) frame);
	}
	SDL_UpdateTexture(texture, NULL, frame, SCREEN_W * 4);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	/* RL_SHOT_SEQ=<n> (dev): n frames, one every 3rd present (~50 ms),
	 * saved as <path>_0.._N.ppm; shotAt == -3 means the run is armed */
	static int shotSeqLeft = 0, shotSeqIndex = 0, shotSeqTick = 0;
	if (shotPath != NULL && shotAt >= 0.0 && plat_time() >= shotAt)
	{
		const char *seq = getenv("RL_SHOT_SEQ");
		shotSeqLeft = seq != NULL ? atoi(seq) : 0;
		shotSeqIndex = 0;
		shotSeqTick = 0;
		if (shotSeqLeft > 1)
			shotAt = -3.0;   /* armed: plat_poll must not quit yet */
		else
		{
			shotAt = -1.0;
			FILE *f = fopen(shotPath, "wb");
			if (f != NULL)
			{
				int i;
				fputs("P6\n640 480\n255\n", f);
				for (i = 0; i < SCREEN_W * SCREEN_H; i += 1)
				{
					uint32_t p = frame[i];
					fputc((p >> 16) & 0xFF, f);
					fputc((p >> 8) & 0xFF, f);
					fputc(p & 0xFF, f);
				}
				fclose(f);
			}
			if (quitAfterShot) exit(0);
		}
	}
	else if (shotAt == -3.0 && shotSeqLeft > 0 && ++shotSeqTick >= 3)
	{
		shotSeqTick = 0;
		char path[512];
		snprintf(path, sizeof(path), "%s_%d.ppm", shotPath, shotSeqIndex++);
		FILE *f = fopen(path, "wb");
		if (f != NULL)
		{
			int i;
			fputs("P6\n640 480\n255\n", f);
			for (i = 0; i < SCREEN_W * SCREEN_H; i += 1)
			{
				uint32_t p = frame[i];
				fputc((p >> 16) & 0xFF, f);
				fputc((p >> 8) & 0xFF, f);
				fputc(p & 0xFF, f);
			}
			fclose(f);
		}
		if (--shotSeqLeft == 0)
		{
			shotAt = -1.0;
			if (quitAfterShot) exit(0);
		}
	}
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
	/* Each mask replaces the previous one from its _MS time (RL_BTN from
	 * RL_BTN_MS, then RL_BTN2, RL_BTN3, RL_BTN4), for four-press test
	 * sequences. The fake bits are replaced each poll so releases are
	 * seen too. */
	double t = plat_time();
	static uint32_t lastFake = 0;
	uint32_t fake = 0;
	if (t >= buttonsAt && (buttons2At == 0.0 || t < buttons2At - 0.05))
		fake |= fakeButtons;
	if (buttons2At > 0.0 && t >= buttons2At && (buttons3At == 0.0 || t < buttons3At - 0.05))
		fake |= fakeButtons2;
	if (buttons3At > 0.0 && t >= buttons3At && (buttons4At == 0.0 || t < buttons4At - 0.05))
		fake |= fakeButtons3;
	if (buttons4At > 0.0 && t >= buttons4At)
		fake |= fakeButtons4;
	if (pulseButtons != 0 && t >= pulseAt)
	{
		if (t >= pulseNext)
		{
			pulseNext = t + pulseEvery;
			pulseUntil = t + 0.1;
		}
		if (t < pulseUntil)
			fake |= pulseButtons;
	}
	*buttons = (*buttons & ~lastFake) | fake;
	lastFake = fake;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) return 0;
		if ((e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) && !e.key.repeat)
		{
			uint32_t bit = KeyBit(e.key.keysym.sym);
			if (e.type == SDL_KEYDOWN) *buttons |= bit;
			else *buttons &= ~bit;
			/* Dev: F12 dumps the current frame to /tmp/live.ppm and the
			 * diagnostics (camera, hero) to /tmp/live.txt */
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F12)
			{
				{
					extern void rlc_debug_dump(const char *path);
					rlc_debug_dump("/tmp/live.txt");
				}
				FILE *f = fopen("/tmp/live.ppm", "wb");
				if (f != NULL)
				{
					int i;
					fputs("P6\n640 480\n255\n", f);
					for (i = 0; i < SCREEN_W * SCREEN_H; i += 1)
					{
						uint32_t px = frame[i];
						fputc((px >> 16) & 0xFF, f);
						fputc((px >> 8) & 0xFF, f);
						fputc(px & 0xFF, f);
					}
					fclose(f);
				}
			}
		}
	}
	if (quitAfterShot && shotAt == -1.0) return 0;
	return !(*buttons & BTN_MENU);
}

double plat_time(void)
{
	return SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency() - startTime;
}

void plat_sleep(double seconds)
{
	if (seconds > 0) SDL_Delay((Uint32) (seconds * 1000.0));
}

/* Sound through SDL2: mono int16 */
static SDL_AudioDeviceID audioDev;
static plat_audio_fn audioFn;

static void AudioCallback(void *, Uint8 *stream, int len)
{
	audioFn((int16_t *) stream, len / 2);
}

int plat_audio_open(int rate, plat_audio_fn fn)
{
	if (getenv("RL_NO_AUDIO") != NULL || SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return 0;
	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = rate;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 1024;
	want.callback = AudioCallback;
	audioFn = fn;
	audioDev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (audioDev == 0) return 0;
	SDL_PauseAudioDevice(audioDev, 0);
	return 1;
}

void plat_audio_lock(void)
{
	if (audioDev != 0) SDL_LockAudioDevice(audioDev);
}

void plat_audio_unlock(void)
{
	if (audioDev != 0) SDL_UnlockAudioDevice(audioDev);
}
