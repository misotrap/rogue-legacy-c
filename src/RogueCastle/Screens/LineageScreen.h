// The heir choice after a death (LineageScreen.cs + LineageObj.cs), laid
// out like the original on the red damask wall: the dead ancestors in a row
// (400 apart), then the three heirs stacked in a column (-450, 0, 450), each
// in a gold frame with its plaque (name, class, traits or years) and its
// spell icon. The camera centres on the selected heir under the spotlight;
// the description plate on the right shows its class, traits and spell.
// Up/Down choose, Left/Right look back along the family tree, B confirms,
// X rerolls the heirs once when the manor's Randomize_Children is bought.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"
#include "../Lineage/Lineage.h"
#include "../Save/SaveGame.h"

#include <string>
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
		// One LineageObj: an ancestor (dead) or an heir
		struct Entry
		{
			Lineage::Heir heir;
			DS2DEngine::ObjContainer *sprite = nullptr;
			float x = 0, y = 0;
			bool dead = false;
			std::string years;        // "1046 - 1081" for the dead
			std::string rank;         // the class line of the plaque
		};

		Save::GameState *m_state = nullptr;
		Lineage::Generator *m_generator = nullptr;
		DS2DEngine::Camera2D m_camera;   // the family tree, zoom 0.5
		DS2DEngine::Camera2D m_screen;   // the fixed 1320x720 overlay
		std::vector<Entry> m_entries;    // ancestors first, then the heirs
		size_t m_firstHeir = 0;
		DS2DEngine::SpriteObj *m_frame = nullptr, *m_plaque = nullptr, *m_iconHolder = nullptr, *m_spellIcon = nullptr;
		DS2DEngine::SpriteObj *m_shadow = nullptr, *m_title = nullptr, *m_plate = nullptr, *m_bg = nullptr;
		Text::Font m_nameFont, m_textFont, m_titleFont;
		int m_selected = 0;
		int m_xShift = 0;                // Left/Right steps back along the tree
		float m_camX = 0, m_camY = 0;    // where the camera eases to
		double m_time = 0;
		std::string m_assetsDir;

		void Build();
		void Clear();
		void DrawEntry(uint32_t *frame, const Entry &e);
		void DrawPlate(uint32_t *frame, const Lineage::Heir &h);
		// LineageObj.SetPortrait / UpdateData for one heir
		static void ApplyHeir(DS2DEngine::ObjContainer *sprite, const Lineage::Heir &h);
		bool CanReroll() const;
	};
}
