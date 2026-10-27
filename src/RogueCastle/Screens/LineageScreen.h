// The heir choice after a death (LineageScreen.cs + LineageObj.cs), in a
// compact layout for the 640x480 screen: the three heirs side by side with
// their animated portrait, name, class, traits and spell, and the details
// of the selected one below. Left/Right choose, B confirms, X rerolls the
// heirs once when the manor's Randomize_Children skill is bought.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../Text/Font.h"
#include "../Lineage/Lineage.h"
#include "../Save/SaveGame.h"

#include <vector>

namespace RogueCastle
{
	class LineageScreen
	{
	public:
		LineageScreen();
		~LineageScreen();

		// Loads the sprite sheets and fonts it needs (once)
		bool LoadContent(const char *assetsDir);

		// Shows the heirs of state.currentBranches, creating three new ones
		// first when there are none (LoadCurrentBranches)
		void Open(Save::GameState &state, Lineage::Generator &generator);

		// buttons = BTN_* held now, pressed = BTN_* pressed this frame.
		// Returns true once an heir is chosen: it is then state.hero, the
		// offered heirs are cleared, and the caller starts the new run.
		bool Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds);
		void Draw(uint32_t *frame);

		int Selected() const { return m_selected; }

	private:
		Save::GameState *m_state = nullptr;
		Lineage::Generator *m_generator = nullptr;
		DS2DEngine::Camera2D m_camera;
		std::vector<DS2DEngine::ObjContainer *> m_portraits;
		Text::Font m_nameFont, m_textFont, m_titleFont;
		int m_selected = 0;
		double m_time = 0;

		void BuildPortraits();
		void ClearPortraits();
		// LineageObj.SetPortrait / UpdateData for one heir
		static void ApplyHeir(DS2DEngine::ObjContainer *sprite, const Lineage::Heir &h);
		bool CanReroll() const;
	};
}
