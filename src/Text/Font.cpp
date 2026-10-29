#include "Font.h"

#include "../platform.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Text
{
	uint32_t NextCodepoint(const char *&s, const char *end)
	{
		const unsigned char *p = (const unsigned char *) s;
		uint32_t c = p[0];
		int n = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
		if (n == 0 || s + n > end)
		{
			s++;
			return '?';
		}
		if (n == 2) c &= 0x1F;
		else if (n == 3) c &= 0x0F;
		else if (n == 4) c &= 0x07;
		for (int i = 1; i < n; i++)
		{
			if ((p[i] & 0xC0) != 0x80)
			{
				s++;
				return '?';
			}
			c = (c << 6) | (p[i] & 0x3F);
		}
		s += n;
		return c;
	}

	bool Font::Load(const char *assetsDir, const char *name)
	{
		char path[512];
		Free();
		snprintf(path, sizeof(path), "%s/fonts/%s.tex", assetsDir, name);
		if (!gfx_load_texture(&sheet, path)) return false;
		snprintf(path, sizeof(path), "%s/fonts/%s.fnt", assetsDir, name);
		FILE *f = fopen(path, "r");
		if (f == nullptr)
		{
			Free();
			return false;
		}
		char line[256];
		while (fgets(line, sizeof(line), f))
		{
			if (line[0] == 'F')
			{
				unsigned d = 0;
				sscanf(line, "F %d %f %u", &lineSpacing, &spacing, &d);
				defaultChar = d;
			}
			else if (line[0] == 'G')
			{
				Glyph g;
				int v[9];
				if (sscanf(line, "G %d %d %d %d %d %d %d %d %d %f %f %f", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8],
					&g.kernA, &g.kernB, &g.kernC) == 12)
				{
					g.code = (uint32_t) v[0];
					g.gx = (short) v[1]; g.gy = (short) v[2]; g.gw = (short) v[3]; g.gh = (short) v[4];
					g.cx = (short) v[5]; g.cy = (short) v[6]; g.cw = (short) v[7]; g.ch = (short) v[8];
					glyphs.push_back(g);
				}
			}
		}
		fclose(f);
		std::sort(glyphs.begin(), glyphs.end(), [](const Glyph &a, const Glyph &b) { return a.code < b.code; });
		return !glyphs.empty();
	}

	void Font::Free()
	{
		gfx_free_texture(&sheet);
		glyphs.clear();
	}

	const Glyph *Font::Find(uint32_t code) const
	{
		auto it = std::lower_bound(glyphs.begin(), glyphs.end(), code, [](const Glyph &g, uint32_t c) { return g.code < c; });
		if (it != glyphs.end() && it->code == code) return &*it;
		if (code != defaultChar && defaultChar != 0) return Find(defaultChar);
		if (code != '?') return Find('?');
		return nullptr;
	}

	// SpriteFont.MeasureString for one line
	float Font::LineWidth(const char *s, const char *end) const
	{
		float x = 0, width = 0;
		bool first = true;
		while (s < end)
		{
			uint32_t c = NextCodepoint(s, end);
			if (c == '\r') continue;
			const Glyph *g = Find(c);
			if (g == nullptr) continue;
			if (first)
			{
				x = std::max(g->kernA, 0.0f);
				first = false;
			}
			else
				x += spacing + g->kernA;
			x += g->kernB;
			width = std::max(width, x + std::max(g->kernC, 0.0f));
			x += g->kernC;
		}
		return width;
	}

	void Font::Measure(const char *utf8, float &width, float &height) const
	{
		width = 0;
		int lines = 1;
		const char *line = utf8, *end = utf8 + strlen(utf8);
		for (const char *p = utf8; p <= end; p++)
		{
			if (p == end || *p == '\n')
			{
				width = std::max(width, LineWidth(line, p));
				if (p != end) lines++;
				line = p + 1;
			}
		}
		height = (float) (lines * lineSpacing);
	}

	float Font::Width(const char *utf8) const
	{
		float w, h;
		Measure(utf8, w, h);
		return w;
	}

	std::string Font::Wrap(const std::string &text, float scale, float maxWidth) const
	{
		std::string out, line, word;
		auto flushWord = [&]()
		{
			if (word.empty()) return;
			std::string tryLine = line.empty() ? word : line + " " + word;
			if (!line.empty() && Width(tryLine.c_str()) * scale > maxWidth)
			{
				out += line + "\n";
				line = word;
			}
			else
				line = tryLine;
			word.clear();
		};
		for (char c : text)
		{
			if (c == ' ') flushWord();
			else if (c == '\n')
			{
				flushWord();
				out += line + "\n";
				line.clear();
			}
			else word += c;
		}
		flushWord();
		return out + line;
	}

	// Tinted, scaled, premultiplied blit of one glyph (nearest texel)
	static void BlitGlyph(uint32_t *fb, const Texture &t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, uint32_t colour)
	{
		const uint32_t ca = colour >> 24, cr = (colour >> 16) & 255, cg = (colour >> 8) & 255, cb = colour & 255;
		int x0 = (int) floorf(dx + 0.5f), y0 = (int) floorf(dy + 0.5f);
		int x1 = x0 + (int) floorf(sw * scale + 0.5f), y1 = y0 + (int) floorf(sh * scale + 0.5f);
		float inv = 1.0f / scale;
		for (int y = std::max(y0, 0); y < y1 && y < SCREEN_H; y++)
		{
			int ty = sy + (int) ((y - y0 + 0.5f) * inv);
			if (ty < 0 || ty >= t.h) continue;
			const uint32_t *src = t.px + (size_t) (ty >> t.shift) * t.stride;
			uint32_t *dst = fb + (size_t) y * SCREEN_W;
			for (int x = std::max(x0, 0); x < x1 && x < SCREEN_W; x++)
			{
				int tx = sx + (int) ((x - x0 + 0.5f) * inv);
				if (tx < 0 || tx >= t.w) continue;
				uint32_t p = src[tx >> t.shift];
				// premultiplied texel * tint (also premultiplied by its alpha)
				uint32_t a = ((p >> 24) * ca + 127) / 255;
				if (a == 0) continue;
				uint32_t r = (((p >> 16) & 255) * cr / 255) * ca / 255;
				uint32_t g = (((p >> 8) & 255) * cg / 255) * ca / 255;
				uint32_t b = ((p & 255) * cb / 255) * ca / 255;
				uint32_t d = dst[x], ia = 255 - a;
				r += ((d >> 16) & 255) * ia / 255;
				g += ((d >> 8) & 255) * ia / 255;
				b += (d & 255) * ia / 255;
				dst[x] = 0xFF000000u | (std::min(r, 255u) << 16) | (std::min(g, 255u) << 8) | std::min(b, 255u);
			}
		}
	}

	// SpriteFont.DrawString for one line
	void Font::DrawLine(uint32_t *frame, const char *s, const char *end, float x, float y, float scale, uint32_t colour) const
	{
		float offset = 0;
		bool first = true;
		while (s < end)
		{
			uint32_t c = NextCodepoint(s, end);
			if (c == '\r') continue;
			const Glyph *g = Find(c);
			if (g == nullptr) continue;
			if (first)
			{
				offset = std::max(g->kernA, 0.0f);
				first = false;
			}
			else
				offset += spacing + g->kernA;
			if (g->gw > 0 && g->gh > 0)
				BlitGlyph(frame, sheet, g->gx, g->gy, g->gw, g->gh, x + (offset + g->cx) * scale, y + g->cy * scale, scale, colour);
			offset += g->kernB + g->kernC;
		}
	}

	void Font::Draw(uint32_t *frame, const char *utf8, float x, float y, float scale, uint32_t colour, Align align) const
	{
		if (!Loaded() || utf8 == nullptr) return;
		const char *line = utf8, *end = utf8 + strlen(utf8);
		float lineY = y;
		for (const char *p = utf8; p <= end; p++)
		{
			if (p != end && *p != '\n') continue;
			float lx = x;
			if (align != Align::Left)
			{
				float w = LineWidth(line, p) * scale;
				lx = align == Align::Centre ? x - w / 2 : x - w;
			}
			DrawLine(frame, line, p, lx, lineY, scale, colour);
			lineY += lineSpacing * scale;
			line = p + 1;
		}
	}

	void Font::DrawOutlined(uint32_t *frame, const char *utf8, float x, float y, float scale, uint32_t colour,
		uint32_t outlineColour, int outlineWidth, Align align) const
	{
		for (int dy = -outlineWidth; dy <= outlineWidth; dy += outlineWidth)
			for (int dx = -outlineWidth; dx <= outlineWidth; dx += outlineWidth)
				if (dx != 0 || dy != 0) Draw(frame, utf8, x + dx, y + dy, scale, outlineColour, align);
		Draw(frame, utf8, x, y, scale, colour, align);
	}
}
