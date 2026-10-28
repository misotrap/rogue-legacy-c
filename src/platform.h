/* Platform layer: a 640x480 frame to draw into, buttons and a clock.
 * Two implementations: platform_sdl.c (computer, for development) and
 * platform_miyoo.c (the console: framebuffer + evdev, no SDL).
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#define SCREEN_W 640
#define SCREEN_H 480

enum
{
	BTN_UP = 1 << 0,
	BTN_DOWN = 1 << 1,
	BTN_LEFT = 1 << 2,
	BTN_RIGHT = 1 << 3,
	BTN_A = 1 << 4,      /* right face button */
	BTN_B = 1 << 5,      /* bottom: jump */
	BTN_X = 1 << 6,      /* top */
	BTN_Y = 1 << 7,      /* left: attack */
	BTN_L1 = 1 << 8,
	BTN_R1 = 1 << 9,
	BTN_L2 = 1 << 10,
	BTN_R2 = 1 << 11,
	BTN_START = 1 << 12,
	BTN_SELECT = 1 << 13,
	BTN_MENU = 1 << 14
};

int plat_init(void);
void plat_quit(void);
/* XRGB8888 pixels, SCREEN_W * SCREEN_H, row stride SCREEN_W */
uint32_t *plat_frame(void);
void plat_present(void);
/* Updates *buttons (bitmask of BTN_*); returns 0 when the game must quit */
int plat_poll(uint32_t *buttons);
/* Seconds since start */
double plat_time(void);
void plat_sleep(double seconds);
/* Sound: fn is called from the platform's audio thread to fill 'frames'
 * mono int16 samples at 'rate'. Returns 0 when there is no sound device.
 * lock/unlock keep fn out while the game changes what it plays. */
typedef void (*plat_audio_fn)(int16_t *out, int frames);
int plat_audio_open(int rate, plat_audio_fn fn);
void plat_audio_lock(void);
void plat_audio_unlock(void);

#endif
