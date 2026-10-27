// The blacksmith (BlacksmithScreen.cs) and the enchantress
// (EnchantressScreen.cs) in one screen: categories on the left (sword,
// helm, chest, limbs, cape), the items of that category in a grid (15
// armour pieces or 11 runes), details on the right. L1/R1 or Up/Down on
// the first row change the category, the arrows move in the grid, B buys
// or equips, START leaves.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/GameObj.h"
#include "../../Text/Font.h"
#include "../Save/SaveGame.h"

#include <vector>

namespace RogueCastle
{
	class EquipmentScreen
	{
	public:
		enum class Mode { Blacksmith, Enchantress };

		explicit EquipmentScreen(Mode mode);
		~EquipmentScreen();

		bool LoadContent(const char *assetsDir);
		// maxWeight: PlayerObj.MaxWeight of the current hero (the blacksmith
		// refuses pieces that would exceed it)
		void Open(Save::GameState &state, int maxWeight);
		// Returns true when the player leaves (START)
		bool Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds);
		void Draw(uint32_t *frame);

	private:
		Mode m_mode;
		Save::GameState *m_state = nullptr;
		int m_maxWeight = 0;
		DS2DEngine::Camera2D m_camera;
		std::vector<DS2DEngine::GameObj *> m_icons; // current category's items
		DS2DEngine::GameObj *m_selection = nullptr, *m_equipped = nullptr, *m_new = nullptr;
		Text::Font m_font, m_titleFont;
		int m_category = 0, m_index = 0;
		double m_flash = 0;  // refused action feedback
		bool m_heavy = false;

		int ItemCount() const;
		uint8_t State(int category, int index) const;
		int Equipped(int category) const;
		void BuildIcons();
		void ClearIcons();
		void Select(int index);
		void Confirm();
	};
}
