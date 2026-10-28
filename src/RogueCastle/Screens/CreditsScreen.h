// The end credits (CreditsScreen.cs, IsEnding): after the last boss, the
// Cellar Door Games credits scroll up over the night sky to the credits
// song, then "Merci d'avoir joué !" and the number of heirs. A, once shown,
// leaves to the next heir (New Game+).
#pragma once

#include "../../Text/Font.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	class CreditsScreen
	{
	public:
		bool LoadContent(const char *assetsDir);
		void Open(int numChildren);
		// true once the player leaves
		bool Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		Text::Font m_font;
		double m_time = 0;
		int m_children = 0;
		float m_height = 0;   // the whole list, in screen pixels
	};
}
