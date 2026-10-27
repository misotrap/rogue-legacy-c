#include "EquipmentScreen.h"

#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstdio>
#include <cstring>
#include <string>

using namespace DS2DEngine;

namespace RogueCastle
{
	namespace
	{
		const uint32_t GOLD = 0xFFF0C850u, WHITE = 0xFFFFFFFFu, GREY = 0xFFB4AAB4u, BLACK = 0xFF000000u;
		const uint32_t RED = 0xFFE04040u, GREEN = 0xFF60D060u;
		const int COLUMNS = 5;
		const float GRID_X = 230, GRID_Y = 210, SPACING = 130; // 1320x720 game coordinates
		const int PANEL_X = 392;

		// EquipmentCategoryType.ToStringEN (sprite names)
		const char *const CATEGORY_EN[] = { "Sword", "Helm", "Chest", "Limbs", "Cape" };

		Color ToColor(uint32_t argb)
		{
			return Color((argb >> 16) & 255, (argb >> 8) & 255, argb & 255, argb >> 24);
		}

		GameObj *MakeIcon(const std::string &name)
		{
			if (SpriteSystem::SpriteLibrary::ContainsCharacter(name)) return new ObjContainer(name);
			return new SpriteObj(name);
		}

		std::string WithButtons(std::string s)
		{
			static const char *const tags[][2] = { { "[Input:0]", "B" }, { "[Input:2]", "START" } };
			for (const auto &t : tags)
			{
				size_t p = s.find(t[0]);
				if (p != std::string::npos) s.replace(p, strlen(t[0]), t[1]);
			}
			return s;
		}

		Vector2 GridPos(int index)
		{
			return Vector2(GRID_X + SPACING * (float) (index % COLUMNS), GRID_Y + SPACING * (float) (index / COLUMNS));
		}

		float SX(float x) { return x * 0.5f - 10.0f; }
		float SY(float y) { return y * 0.5f + 60.0f; }
	}

	EquipmentScreen::EquipmentScreen(Mode mode) : m_mode(mode)
	{
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
	}

	EquipmentScreen::~EquipmentScreen()
	{
		ClearIcons();
		delete m_selection;
		delete m_equipped;
		delete m_new;
	}

	bool EquipmentScreen::LoadContent(const char *assetsDir)
	{
		bool ok = true;
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("BlacksmithUI_SelectionIcon_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "blacksmithUISpritesheet") && ok;
		ok = m_font.Load(assetsDir, "Junicode") && ok;
		ok = m_titleFont.Load(assetsDir, "JunicodeLarge") && ok;
		if (m_selection == nullptr)
		{
			m_selection = new SpriteObj("BlacksmithUI_SelectionIcon_Sprite");
			m_selection->PlayAnimation(true);
			m_equipped = new SpriteObj("BlacksmithUI_EquippedIcon_Sprite");
			m_new = new SpriteObj("BlacksmithUI_NewIcon_Sprite");
		}
		return ok;
	}

	int EquipmentScreen::ItemCount() const
	{
		return m_mode == Mode::Blacksmith ? Equipment::BASE_TYPES : Equipment::ABILITIES;
	}

	uint8_t EquipmentScreen::State(int c, int i) const
	{
		return m_mode == Mode::Blacksmith ? m_state->equipment.blueprint[c][i] : m_state->equipment.rune[c][i];
	}

	int EquipmentScreen::Equipped(int c) const
	{
		return m_mode == Mode::Blacksmith ? m_state->equipment.equipped[c] : m_state->equipment.equippedRune[c];
	}

	void EquipmentScreen::ClearIcons()
	{
		for (GameObj *o : m_icons) delete o;
		m_icons.clear();
	}

	// BlacksmithScreen / EnchantressScreen.UpdateIconStates
	void EquipmentScreen::BuildIcons()
	{
		ClearIcons();
		for (int i = 0; i < ItemCount(); i++)
		{
			uint8_t state = State(m_category, i);
			GameObj *icon;
			if (state == EquipmentState::NotFound)
				icon = MakeIcon(m_mode == Mode::Blacksmith ? "BlacksmithUI_QuestionMarkIcon_Character" : "BlacksmithUI_QuestionMarkIcon_Sprite");
			else if (m_mode == Mode::Enchantress)
				icon = MakeIcon(LocIDs::EquipmentAbilityType_Icon(i));
			else
			{
				ObjContainer *c = new ObjContainer(std::string("BlacksmithUI_") + CATEGORY_EN[m_category] + std::to_string(i % 5 + 1) + "Icon_Character");
				icon = c;
				const Equipment::ItemData *item = Equipment::EquipmentSystem::Item(m_category, i);
				if (c != nullptr && item != nullptr)
				{
					// Not bought: faded; bought: the piece's colours
					for (int j = 1; j < c->NumChildren(); j++)
						if (c->GetChildAt(j)) c->GetChildAt(j)->Opacity(state > EquipmentState::FoundAndSeen ? 1.0f : 0.2f);
					if (state > EquipmentState::FoundAndSeen)
					{
						int colourIndex = m_category == EquipmentCategoryType::Sword ? 2 : 1;
						if (c->GetChildAt(colourIndex)) c->GetChildAt(colourIndex)->TextureColor(ToColor(item->firstColour));
						if (m_category != EquipmentCategoryType::Cape && c->GetChildAt(colourIndex + 1))
							c->GetChildAt(colourIndex + 1)->TextureColor(ToColor(item->secondColour));
					}
				}
			}
			icon->Position(GridPos(i));
			m_icons.push_back(icon);
		}
	}

	void EquipmentScreen::Open(Save::GameState &state, int maxWeight)
	{
		m_state = &state;
		m_maxWeight = maxWeight;
		m_category = 0;
		BuildIcons();
		Select(0);
	}

	void EquipmentScreen::Select(int index)
	{
		m_index = index;
		m_heavy = false;
		m_selection->Position(GridPos(index));
		// Seeing a new item clears its "new" mark
		uint8_t &state = m_mode == Mode::Blacksmith ? m_state->equipment.blueprint[m_category][index] : m_state->equipment.rune[m_category][index];
		if (state == EquipmentState::FoundButNotSeen) state = EquipmentState::FoundAndSeen;
	}

	void EquipmentScreen::Confirm()
	{
		Equipment::EquipmentSystem &eq = m_state->equipment;
		bool ok = false;
		uint8_t state = State(m_category, m_index);
		if (m_mode == Mode::Blacksmith)
		{
			if (state == EquipmentState::FoundAndSeen || state == EquipmentState::FoundButNotSeen)
				ok = eq.BuyBlueprint(m_category, m_index, m_state->Gold);
			else if (state == EquipmentState::Purchased)
			{
				ok = eq.Equip(m_category, m_index, m_maxWeight);
				m_heavy = !ok;
			}
		}
		else
		{
			if (state == EquipmentState::FoundAndSeen || state == EquipmentState::FoundButNotSeen)
				ok = eq.BuyRune(m_category, m_index, m_state->Gold);
			else if (state == EquipmentState::Purchased)
				ok = eq.EquipRune(m_category, m_index);
		}
		if (ok) BuildIcons();
		else m_flash = 0.6;
	}

	bool EquipmentScreen::Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds)
	{
		(void) buttons;
		m_camera.ElapsedTotalSeconds(elapsedSeconds);
		if (m_flash > 0) m_flash -= elapsedSeconds;
		if (m_state == nullptr) return false;
		int n = ItemCount();
		int category = m_category;
		if (pressed & BTN_L1) category = (m_category + Equipment::CATEGORIES - 1) % Equipment::CATEGORIES;
		if (pressed & BTN_R1) category = (m_category + 1) % Equipment::CATEGORIES;
		if ((pressed & BTN_UP) && m_index < COLUMNS) category = (m_category + Equipment::CATEGORIES - 1) % Equipment::CATEGORIES;
		else if (pressed & BTN_UP) Select(m_index - COLUMNS);
		if ((pressed & BTN_DOWN) && m_index + COLUMNS < n) Select(m_index + COLUMNS);
		if ((pressed & BTN_LEFT) && m_index % COLUMNS > 0) Select(m_index - 1);
		if ((pressed & BTN_RIGHT) && m_index % COLUMNS < COLUMNS - 1 && m_index + 1 < n) Select(m_index + 1);
		if (category != m_category)
		{
			m_category = category;
			BuildIcons();
			Select(m_index < n ? m_index : 0);
		}
		if (pressed & (BTN_B | BTN_A)) Confirm();
		return (pressed & BTN_START) != 0;
	}

	void EquipmentScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		gfx_clear(m_mode == Mode::Blacksmith ? 0xFF2A1E18u : 0xFF20183Au);
		if (m_state == nullptr) return;
		char buf[160];
		const float s = 0.3f;

		// Category list
		for (int c = 0; c < Equipment::CATEGORIES; c++)
		{
			const char *name = Text::Loc::Get(LocIDs::EquipmentCategoryType_ToStringID(c));
			bool selected = c == m_category;
			if (selected) gfx_fill(4, 86 + c * 30, 92, 26, 0xFF5A4630u);
			m_font.DrawOutlined(frame, name, 10, 90 + c * 30, 0.32f, selected ? GOLD : GREY, BLACK, 1);
		}

		// Item grid
		for (int i = 0; i < (int) m_icons.size(); i++)
		{
			m_icons[i]->Draw(&m_camera);
			uint8_t state = State(m_category, i);
			if (Equipped(m_category) == i)
			{
				m_equipped->Position(GridPos(i) + Vector2(30, 30));
				m_equipped->Draw(&m_camera);
			}
			else if (state == EquipmentState::FoundButNotSeen)
			{
				m_new->Position(GridPos(i) + Vector2(30, -30));
				m_new->Draw(&m_camera);
			}
		}
		m_selection->Draw(&m_camera);

		// Title and gold
		const char *title = m_mode == Mode::Blacksmith ? "Forgeron" : "Enchanteresse";
		m_titleFont.DrawOutlined(frame, title, 10, 8, 0.32f, GOLD, BLACK, 1);
		snprintf(buf, sizeof(buf), "%d", m_state->Gold);
		m_titleFont.DrawOutlined(frame, buf, SCREEN_W - 8, 8, 0.3f, GOLD, BLACK, 1, Text::Align::Right);

		// Details panel
		gfx_fill(PANEL_X - 4, 50, SCREEN_W - PANEL_X, SCREEN_H - 50, 0xE0100C10u);
		gfx_fill(PANEL_X - 4, 50, 2, SCREEN_H - 50, GOLD);
		const float w = (float) (SCREEN_W - PANEL_X - 8);
		float y = 58;
		auto line = [&](const std::string &text, uint32_t colour, float scale)
		{
			std::string wrapped = m_font.Wrap(text, scale, w);
			m_font.DrawOutlined(frame, wrapped.c_str(), PANEL_X, y, scale, colour, BLACK, 1);
			int lines = 1;
			for (char c : wrapped) lines += c == '\n';
			y += m_font.LineSpacing() * scale * lines + 2;
		};
		uint8_t state = State(m_category, m_index);
		const bool blacksmith = m_mode == Mode::Blacksmith;
		if (state == EquipmentState::NotFound)
		{
			line(Text::Loc::Get(blacksmith ? "LOC_ID_BLACKSMITH_SCREEN_13" : "LOC_ID_ENCHANTRESS_SCREEN_10"), GREY, 0.34f);
		}
		else if (blacksmith)
		{
			const Equipment::ItemData *item = Equipment::EquipmentSystem::Item(m_category, m_index);
			std::string name = Text::Loc::Format("LOC_ID_EQUIPMENT_BASE_FORMAT", Text::Loc::Get(LocIDs::EquipmentBaseType_ToStringID(m_index)),
				Text::Loc::Get(LocIDs::EquipmentCategoryType_ToStringID(m_category)));
			line(name, GOLD, 0.36f);
			// Stats against what is worn now (BlacksmithScreen.UpdateTextInfo)
			int worn = m_state->equipment.equipped[m_category];
			const Equipment::ItemData *cur = worn >= 0 ? Equipment::EquipmentSystem::Item(m_category, worn) : nullptr;
			const struct { const char *id; int value, current; } stats[] = {
				{ "LOC_ID_BLACKSMITH_SCREEN_1", item->bonusHealth, cur ? cur->bonusHealth : 0 },
				{ "LOC_ID_BLACKSMITH_SCREEN_2", item->bonusMana, cur ? cur->bonusMana : 0 },
				{ "LOC_ID_BLACKSMITH_SCREEN_3", item->bonusDamage, cur ? cur->bonusDamage : 0 },
				{ "LOC_ID_BLACKSMITH_SCREEN_4", item->bonusMagic, cur ? cur->bonusMagic : 0 },
				{ "LOC_ID_BLACKSMITH_SCREEN_5", item->bonusArmor, cur ? cur->bonusArmor : 0 },
				{ "LOC_ID_BLACKSMITH_SCREEN_6", item->weight, cur ? cur->weight : 0 } };
			for (const auto &st : stats)
			{
				if (st.value == 0 && st.current == 0) continue;
				int diff = worn == m_index ? 0 : st.value - st.current;
				snprintf(buf, sizeof(buf), "%s : %d", Text::Loc::Get(st.id), st.value);
				std::string text = buf;
				if (diff != 0)
				{
					snprintf(buf, sizeof(buf), "  (%+d)", diff);
					text += buf;
				}
				bool weightLine = strcmp(st.id, "LOC_ID_BLACKSMITH_SCREEN_6") == 0;
				line(text, diff == 0 ? WHITE : ((diff > 0) != weightLine ? GREEN : RED), s);
			}
			if (item->numSecondary > 0)
			{
				line(Text::Loc::Get("LOC_ID_BLACKSMITH_SCREEN_7"), GOLD, s);
				for (int k = 0; k < item->numSecondary; k++)
				{
					const Equipment::SecondaryAttribute &a = item->secondary[k];
					if (a.amount < 1) snprintf(buf, sizeof(buf), "+%d%% ", (int) (a.amount * 100 + 0.5f));
					else snprintf(buf, sizeof(buf), "+%g ", a.amount);
					line(std::string(buf) + Text::Loc::Get(LocIDs::EquipmentSecondaryDataType_ToStringID(a.type)), WHITE, s);
				}
			}
			snprintf(buf, sizeof(buf), "%s : %d/%d", Text::Loc::Get("LOC_ID_BLACKSMITH_SCREEN_6"),
				m_state->equipment.CurrentWeight(), m_maxWeight);
			y += 4;
			line(buf, m_heavy ? RED : GREY, s);
			if (state < EquipmentState::Purchased)
			{
				std::string cost = Text::Loc::Format("LOC_ID_BLACKSMITH_SCREEN_16_NEW", std::to_string(item->cost));
				line(cost, (m_state->Gold >= item->cost && !(m_flash > 0)) ? GOLD : RED, 0.34f);
			}
		}
		else
		{
			std::string name = Text::Loc::Format("LOC_ID_RUNE_BASE_FORMAT", Text::Loc::Get(LocIDs::EquipmentAbilityType_ToStringID(m_index)),
				Text::Loc::Get("LOC_ID_ENCHANTRESS_SCREEN_13"));
			line(name, GOLD, 0.36f);
			line(Text::Loc::Get(LocIDs::EquipmentAbilityType_DescriptionID(m_index)), WHITE, s);
			if (state < EquipmentState::Purchased)
			{
				int cost = m_state->equipment.RuneCost(m_category, m_index);
				line(Text::Loc::Format("LOC_ID_ENCHANTRESS_SCREEN_12_NEW", std::to_string(cost)),
					(m_state->Gold >= cost && !(m_flash > 0)) ? GOLD : RED, 0.34f);
			}
		}

		// Hints
		std::string confirm = WithButtons(Text::Loc::Get(blacksmith ? "LOC_ID_BLACKSMITH_SCREEN_8_NEW" : "LOC_ID_ENCHANTRESS_SCREEN_6_NEW"));
		std::string leave = WithButtons(Text::Loc::Get(blacksmith ? "LOC_ID_BLACKSMITH_SCREEN_9_NEW" : "LOC_ID_ENCHANTRESS_SCREEN_7_NEW"));
		m_font.DrawOutlined(frame, (confirm + "   L1/R1 : catégorie").c_str(), 10, SCREEN_H - 44, 0.28f, GREY, BLACK, 1);
		m_font.DrawOutlined(frame, leave.c_str(), 10, SCREEN_H - 24, 0.28f, GREY, BLACK, 1);
		(void) SX;
		(void) SY;
	}
}
