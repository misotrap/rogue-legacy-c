/* Miyoo Mini (Plus) platform: /dev/fb0 (640x480, 32 bpp, mounted upside
 * down, two pages flipped with FBIOPAN_DISPLAY) and /dev/input/event0.
 * No SDL: the binary is linked statically and runs on stock OnionOS.
 */
#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#include "platform.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/* <linux/input.h> defines BTN_LEFT, BTN_A, BTN_START... as Linux key codes:
 * they would replace the BTN_* bits of platform.h below and scramble the
 * buttons (left arrow read as A + L1, Y as left + jump...). */
#undef BTN_LEFT
#undef BTN_RIGHT
#undef BTN_A
#undef BTN_B
#undef BTN_X
#undef BTN_Y
#undef BTN_START
#undef BTN_SELECT
#undef BTN_MODE

static int fbFd = -1, inFd = -1;
static uint8_t *fbMem;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static int pages = 1, page;
static uint32_t frame[SCREEN_W * SCREEN_H];
static struct timespec start;

int plat_init(void)
{
	size_t len;
	clock_gettime(CLOCK_MONOTONIC, &start);
	fbFd = open("/dev/fb0", O_RDWR);
	if (fbFd < 0) return 0;
	if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) < 0 || ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) < 0) return 0;
	vinfo.yres_virtual = vinfo.yres * 2;
	vinfo.bits_per_pixel = 32;
	if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vinfo) == 0 && vinfo.yres_virtual >= vinfo.yres * 2) pages = 2;
	ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo);
	ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo);
	len = (size_t) finfo.line_length * vinfo.yres_virtual;
	fbMem = (uint8_t*) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);
	if (fbMem == MAP_FAILED) return 0;
	inFd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
	return 1;
}

void plat_quit(void)
{
	if (fbFd >= 0)
	{
		vinfo.yoffset = 0;
		ioctl(fbFd, FBIOPAN_DISPLAY, &vinfo);
		memset(fbMem, 0, (size_t) finfo.line_length * vinfo.yres);
	}
}

uint32_t *plat_frame(void)
{
	return frame;
}

void plat_present(void)
{
	int target = (pages > 1) ? (page ^ 1) : 0;
	uint8_t *base = fbMem + (size_t) target * vinfo.yres * finfo.line_length;
	int y, x;
	/* The panel is mounted upside down: rotate 180 degrees while copying */
	for (y = 0; y < SCREEN_H; y += 1)
	{
		const uint32_t *src = frame + (size_t) (SCREEN_H - 1 - y) * SCREEN_W + (SCREEN_W - 1);
		uint32_t *dst = (uint32_t*) (base + (size_t) y * finfo.line_length);
#if defined(__ARM_NEON)
		/* Four pixels at a time: load, reverse, set the alpha, store */
		const uint32x4_t alpha = vdupq_n_u32(0xFF000000u);
		for (x = 0; x < SCREEN_W; x += 4, src -= 4)
		{
			uint32x4_t v = vrev64q_u32(vld1q_u32(src - 3));
			v = vcombine_u32(vget_high_u32(v), vget_low_u32(v));
			vst1q_u32(dst + x, vorrq_u32(v, alpha));
		}
#else
		for (x = 0; x < SCREEN_W; x += 1) dst[x] = *src-- | 0xFF000000u;
#endif
	}
	if (pages > 1)
	{
		vinfo.yoffset = target * vinfo.yres;
		if (ioctl(fbFd, FBIOPAN_DISPLAY, &vinfo) == 0) page = target;
	}
}

static uint32_t KeyBit(int code)
{
	switch (code)
	{
	case 103: return BTN_UP;
	case 108: return BTN_DOWN;
	case 105: return BTN_LEFT;
	case 106: return BTN_RIGHT;
	case 57: return BTN_A;
	case 29: return BTN_B;
	case 42: return BTN_X;
	case 56: return BTN_Y;
	case 18: return BTN_L1;
	case 20: return BTN_R1;
	case 15: return BTN_L2;
	case 14: return BTN_R2;
	case 28: return BTN_START;
	case 97: return BTN_SELECT;
	case 1: return BTN_MENU;
	default: return 0;
	}
}

int plat_poll(uint32_t *buttons)
{
	struct input_event ev;
	while (inFd >= 0 && read(inFd, &ev, sizeof(ev)) == (ssize_t) sizeof(ev))
	{
		if (ev.type != EV_KEY) continue;
		if (ev.value) *buttons |= KeyBit(ev.code);
		else *buttons &= ~KeyBit(ev.code);
	}
	return !(*buttons & BTN_MENU);
}

double plat_time(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) * 1e-9;
}

void plat_sleep(double seconds)
{
	if (seconds > 0) usleep((useconds_t) (seconds * 1e6));
}

/* Sound through the console's own SDL 1.2 (/customer/lib, which drives
 * the MI_AO audio block), loaded at run time so the binary still starts
 * without it. dlopen/dlsym are weak: the toolchain's glibc 2.28 keeps them
 * in libdl, the shipped glibc 2.36 has them in libc (no libdl needed). */
#include <dlfcn.h>
#include <stdio.h>
#pragma weak dlopen
#pragma weak dlsym
#pragma weak dlerror

struct SDL12_AudioSpec
{
	int freq;
	uint16_t format;
	uint8_t channels, silence;
	uint16_t samples, padding;
	uint32_t size;
	void (*callback)(void *userdata, uint8_t *stream, int len);
	void *userdata;
};

static plat_audio_fn audioFn;
static void (*sdlLockAudio)(void), (*sdlUnlockAudio)(void);

static void AudioCallback(void *, uint8_t *stream, int len)
{
	audioFn((int16_t *) stream, len / 2);
}

int plat_audio_open(int rate, plat_audio_fn fn)
{
	if (dlopen == NULL || dlsym == NULL)
	{
		fprintf(stderr, "audio: no dlopen\n");
		return 0;
	}
	void *sdl = dlopen("/customer/lib/libSDL-1.2.so.0", RTLD_NOW | RTLD_GLOBAL);
	if (sdl == NULL) sdl = dlopen("libSDL-1.2.so.0", RTLD_NOW | RTLD_GLOBAL);
	if (sdl == NULL)
	{
		fprintf(stderr, "audio: %s\n", dlerror != NULL ? dlerror() : "no SDL 1.2");
		return 0;
	}
	int (*init)(uint32_t) = (int (*)(uint32_t)) dlsym(sdl, "SDL_Init");
	int (*openAudio)(SDL12_AudioSpec *, SDL12_AudioSpec *) = (int (*)(SDL12_AudioSpec *, SDL12_AudioSpec *)) dlsym(sdl, "SDL_OpenAudio");
	void (*pauseAudio)(int) = (void (*)(int)) dlsym(sdl, "SDL_PauseAudio");
	sdlLockAudio = (void (*)(void)) dlsym(sdl, "SDL_LockAudio");
	sdlUnlockAudio = (void (*)(void)) dlsym(sdl, "SDL_UnlockAudio");
	if (init == NULL || openAudio == NULL || pauseAudio == NULL || sdlLockAudio == NULL || sdlUnlockAudio == NULL)
	{
		fprintf(stderr, "audio: SDL 1.2 symbols missing\n");
		sdlLockAudio = sdlUnlockAudio = NULL;
		return 0;
	}
	if (init(0x10 /* SDL_INIT_AUDIO */) != 0)
	{
		fprintf(stderr, "audio: SDL_Init failed\n");
		sdlLockAudio = sdlUnlockAudio = NULL;
		return 0;
	}
	SDL12_AudioSpec want;
	memset(&want, 0, sizeof(want));
	want.freq = rate;
	want.format = 0x8010;   /* AUDIO_S16LSB */
	want.channels = 1;
	want.samples = 1024;
	want.callback = AudioCallback;
	audioFn = fn;
	/* obtained = NULL: SDL converts to what the device wants */
	if (openAudio(&want, NULL) != 0)
	{
		fprintf(stderr, "audio: SDL_OpenAudio failed\n");
		sdlLockAudio = sdlUnlockAudio = NULL;
		return 0;
	}
	pauseAudio(0);
	fprintf(stderr, "audio: SDL 1.2 at %d Hz\n", rate);
	return 1;
}

void plat_audio_lock(void)
{
	if (sdlLockAudio != NULL) sdlLockAudio();
}

void plat_audio_unlock(void)
{
	if (sdlUnlockAudio != NULL) sdlUnlockAudio();
}
