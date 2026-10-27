// Draws sample text with each font into a PPM (no window):
//   make texttest && ./texttest text.ppm
#include "../src/Text/Font.h"
#include "../src/platform.h"

#include <cstdio>

static uint32_t frame[SCREEN_W * SCREEN_H];

int main(int argc, char **argv)
{
	const char *out = argc > 1 ? argv[1] : "text.ppm";
	gfx_begin(frame);
	gfx_clear(0xFF241C28u);
	static const char *const fonts[] = { "Junicode", "JunicodeLarge", "CinzelFont", "HerzogVonGraf24", "RobotoSlab", "BitFont", "GoldFont", "PlayerLevelFont" };
	float y = 8;
	for (const char *name : fonts)
	{
		Text::Font f;
		if (!f.Load("assets", name))
		{
			printf("cannot load %s\n", name);
			continue;
		}
		float scale = f.LineSpacing() > 60 ? 0.4f : 1.0f;
		char sample[160];
		snprintf(sample, sizeof(sample), "%s : Héritier Sir Lancelot II, 1234 or", name);
		f.DrawOutlined(frame, sample, 10, y, scale, 0xFFFFFFFFu, 0xFF000000u, 1);
		y += f.LineSpacing() * scale + 6;
	}
	Text::Font title;
	title.Load("assets", "JunicodeLarge");
	title.DrawOutlined(frame, "Choisis ton héritier", SCREEN_W / 2, 400, 0.5f, 0xFFF0C850u, 0xFF000000u, 2, Text::Align::Centre);
	FILE *f = fopen(out, "wb");
	fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
	for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
	{
		unsigned char rgb[3] = { (unsigned char) (frame[i] >> 16), (unsigned char) (frame[i] >> 8), (unsigned char) frame[i] };
		fwrite(rgb, 1, 3, f);
	}
	fclose(f);
	printf("wrote %s\n", out);
	return 0;
}
