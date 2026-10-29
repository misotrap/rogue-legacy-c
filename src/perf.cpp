#include "perf.h"
#include "platform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
	const char *NAMES[PERF_COUNT] = { "logique", "decor", "effets", "interface", "ecran" };

	int s_on = -1;
	double s_last = 0;
	double s_sum[PERF_COUNT] = {}, s_shown[PERF_COUNT] = {};   // seconds
	double s_frameMax = 0, s_frameStart = 0, s_shownMax = 0;
	int s_frames = 0, s_steps = 0, s_shownFps = 0, s_shownSteps = 0;
	double s_secondStart = 0, s_logStart = 0;
	// The log: sums over 5 s
	double s_logSum[PERF_COUNT] = {}, s_logMax = 0;
	int s_logFrames = 0, s_logSteps = 0;
	char s_label[32] = "";

	// A 3x5 font: digits, then '.', ' ', 'L','D','E','I','C','F','M','S'
	const char *GLYPHS = "0123456789. LDEICFMS";
	const uint16_t FONT[] = {
		075557, 026227, 071747, 071717, 055711, 074717, 074757, 071111, 075757, 075717,
		000002, 000000, 044447, 065556, 074647, 072227, 074447, 074644, 057555, 074717,
	};

	void DrawText(uint32_t *frame, int x, int y, const char *text, uint32_t colour)
	{
		for (; *text; text++, x += 4)
		{
			const char *g = strchr(GLYPHS, *text);
			if (g == nullptr) continue;
			uint16_t bits = FONT[g - GLYPHS];
			for (int row = 0; row < 5; row++)
				for (int col = 0; col < 3; col++)
					if (bits >> ((4 - row) * 3 + (2 - col)) & 1)
						for (int dy = 0; dy < 2; dy++)
							for (int dx = 0; dx < 2; dx++)
							{
								int px = x * 2 + col * 2 + dx, py = y + row * 2 + dy;
								if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H)
									frame[py * SCREEN_W + px] = colour;
							}
		}
	}
}

bool perf_on()
{
	if (s_on < 0) s_on = getenv("RL_PERF") != nullptr ? 1 : 0;
	return s_on == 1;
}

void perf_start()
{
	if (!perf_on()) return;
	s_last = s_frameStart = plat_time();
	if (s_secondStart == 0) s_secondStart = s_logStart = s_last;
}

void perf_mark(PerfSlot slot)
{
	if (!perf_on()) return;
	double now = plat_time();
	s_sum[slot] += now - s_last;
	s_logSum[slot] += now - s_last;
	s_last = now;
}

void perf_label(const char *label)
{
	snprintf(s_label, sizeof(s_label), "%s", label);
}

void perf_frame(int steps)
{
	if (!perf_on()) return;
	double now = plat_time(), frame = now - s_frameStart;
	if (frame > s_frameMax) s_frameMax = frame;
	if (frame > s_logMax) s_logMax = frame;
	s_frames++;
	s_steps += steps;
	s_logFrames++;
	s_logSteps += steps;
	if (now - s_secondStart >= 1)
	{
		for (int i = 0; i < PERF_COUNT; i++)
		{
			s_shown[i] = s_frames > 0 ? s_sum[i] / s_frames : 0;
			s_sum[i] = 0;
		}
		s_shownFps = s_frames;
		s_shownSteps = s_steps;
		s_shownMax = s_frameMax;
		s_frames = s_steps = 0;
		s_frameMax = 0;
		s_secondStart = now;
	}
	if (now - s_logStart >= 5 && s_logFrames > 0)
	{
		double elapsed = now - s_logStart;
		fprintf(stderr, "perf [%s] %.1f img/s, %.1f pas/img, pire %.1f ms |", s_label,
		        s_logFrames / elapsed, (double) s_logSteps / s_logFrames, s_logMax * 1000);
		for (int i = 0; i < PERF_COUNT; i++)
		{
			fprintf(stderr, " %s %.1f", NAMES[i], s_logSum[i] / s_logFrames * 1000);
			s_logSum[i] = 0;
		}
		fprintf(stderr, " ms\n");
		fflush(stderr);
		s_logFrames = s_logSteps = 0;
		s_logMax = 0;
		s_logStart = now;
	}
}

void perf_draw(uint32_t *frame)
{
	if (!perf_on()) return;
	// "F 45 S 1.3 M 31.0" then "L 4.1 D 12.0 E 0.5 I 3.2 C 6.0" (ms per frame)
	char line1[48], line2[64];
	snprintf(line1, sizeof(line1), "F %d S %.1f M %.1f", s_shownFps,
	         s_shownFps > 0 ? (double) s_shownSteps / s_shownFps : 0.0, s_shownMax * 1000);
	snprintf(line2, sizeof(line2), "L %.1f D %.1f E %.1f I %.1f C %.1f", s_shown[0] * 1000,
	         s_shown[1] * 1000, s_shown[2] * 1000, s_shown[3] * 1000, s_shown[4] * 1000);
	for (int y = 0; y < 24; y++)
		for (int x = 0; x < 260; x++)
			frame[(SCREEN_H - 24 + y) * SCREEN_W + x] = 0xFF000000u;
	DrawText(frame, 1, SCREEN_H - 23, line1, 0xFFFFFF00u);
	DrawText(frame, 1, SCREEN_H - 11, line2, 0xFFFFFFFFu);
}
