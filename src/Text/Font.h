// Text: the game's XNA SpriteFonts, converted by tools/convert_assets.py
// into assets/fonts/<name>.tex + .fnt. Measuring and drawing follow
// SpriteFont.MeasureString / DrawString, so text lays out like the original.
// Draws straight into the 640x480 frame with a colour tint and a scale.
#pragma once

#include "../gfx.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Text
{
	struct Glyph
	{
		uint32_t code = 0;
		short gx = 0, gy = 0, gw = 0, gh = 0;   // in the sheet
		short cx = 0, cy = 0, cw = 0, ch = 0;   // cropping: offset of the glyph in its cell
		float kernA = 0, kernB = 0, kernC = 0;  // left bearing, width, right bearing
	};

	enum class Align { Left, Centre, Right };

	class Font
	{
	public:
		// e.g. Load("assets", "Junicode")
		bool Load(const char *assetsDir, const char *name);
		void Free();
		bool Loaded() const { return sheet.px != nullptr; }

		int LineSpacing() const { return lineSpacing; }
		// Size of a UTF-8 string at scale 1, in font pixels (MeasureString)
		void Measure(const char *utf8, float &width, float &height) const;
		float Width(const char *utf8) const;
		// Breaks text into lines no wider than maxWidth screen pixels at this scale
		std::string Wrap(const std::string &text, float scale, float maxWidth) const;

		// Draws a UTF-8 string with its top-left (or top-centre/right) at
		// screen position (x, y). colour is 0xAARRGGBB (alpha = opacity).
		void Draw(uint32_t *frame, const char *utf8, float x, float y, float scale = 1.0f,
			uint32_t colour = 0xFFFFFFFFu, Align align = Align::Left) const;
		// Draws with an outline (TextObj.OutlineWidth): the text in
		// outlineColour at the 8 surrounding offsets, then the text itself
		void DrawOutlined(uint32_t *frame, const char *utf8, float x, float y, float scale, uint32_t colour,
			uint32_t outlineColour, int outlineWidth = 1, Align align = Align::Left) const;

	private:
		Texture sheet = {};
		int lineSpacing = 0;
		float spacing = 0;
		uint32_t defaultChar = 0;
		std::vector<Glyph> glyphs;         // sorted by code
		const Glyph *Find(uint32_t code) const;
		void DrawLine(uint32_t *frame, const char *s, const char *end, float x, float y, float scale, uint32_t colour) const;
		float LineWidth(const char *s, const char *end) const;
	};

	// Decodes one UTF-8 character and advances s (invalid bytes -> '?')
	uint32_t NextCodepoint(const char *&s, const char *end);
}
