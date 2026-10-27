#include "ManorScreen.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"
#include "../Skills/SkillTree.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

using namespace DS2DEngine;

namespace RogueCastle
{
	namespace
	{
		const uint32_t GOLD = 0xFFF0C850u, WHITE = 0xFFFFFFFFu, GREY = 0xFFB4AAB4u, BLACK = 0xFF000000u, RED = 0xFFE04040u;
		const int PANEL_X = 486; // the right part of the screen holds the details

		int SkillIndex(Skills::SkillType t)
		{
			for (int i = 0; i < Skills::SKILL_COUNT; i++)
				if (Skills::SKILLS[i].type == t) return i;
			return -1;
		}

		// SkillSystem.UpdateTraitSprite: "...Locked_Sprite" -> "..._Sprite" / "...Max_Sprite"
		std::string IconFor(const Skills::SkillInfo &s, int level)
		{
			std::string name = s.iconName;
			size_t p = name.find("Locked");
			if (p == std::string::npos) return name;
			if (level > 0 && level < s.maxLevel) name.erase(p, 6);
			else if (level > 0) name.replace(p, 6, "Max");
			return name;
		}

		// UnitOfMeasurement -> the French unit (SkillBuilder UnitLocIDs)
		std::string Unit(const char *unit)
		{
			static const char *const map[][2] = { { " hp", "LOC_ID_SKILL_SCREEN_14" }, { " mp", "LOC_ID_SKILL_SCREEN_15" },
				{ " str", "LOC_ID_SKILL_SCREEN_16" }, { " int", "LOC_ID_SKILL_SCREEN_17" }, { "% hp/mp", "LOC_ID_SKILL_SCREEN_18" },
				{ " armor", "LOC_ID_SKILL_SCREEN_20" }, { " sec", "LOC_ID_SKILL_SCREEN_21" }, { " weight", "LOC_ID_SKILL_SCREEN_13" } };
			for (const auto &m : map)
				if (strcmp(unit, m[0]) == 0)
					return std::string(unit[0] == '%' ? "% " : " ") + Text::Loc::Get(m[1]);
			return unit;
		}

		// SkillScreen.UpdateDescriptionPlate number format: fractions are
		// shown as percentages (except the invulnerability time)
		std::string Amount(const Skills::SkillInfo &s, float value)
		{
			char buf[32];
			if (s.perLevelModifier < 1 && s.type != Skills::SkillType::Invuln_Time_Up)
				snprintf(buf, sizeof(buf), "%d", (int) lroundf(value * 100));
			else if (value == floorf(value))
				snprintf(buf, sizeof(buf), "%d", (int) value);
			else
				snprintf(buf, sizeof(buf), "%.2g", value);
			return buf;
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
	}

	ManorScreen::ManorScreen()
	{
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
	}

	ManorScreen::~ManorScreen()
	{
		Clear();
	}

	void ManorScreen::Clear()
	{
		delete m_manor;
		m_manor = nullptr;
		for (SpriteObj *s : m_icons) delete s;
		m_icons.clear();
		delete m_halo;
		m_halo = nullptr;
	}

	bool ManorScreen::LoadContent(const char *assetsDir)
	{
		bool ok = true;
		if (!SpriteSystem::SpriteLibrary::ContainsCharacter("TraitsCastle_Character"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "traitsCastleSpritesheet") && ok;
		ok = m_font.Load(assetsDir, "Junicode") && ok;
		ok = m_titleFont.Load(assetsDir, "JunicodeLarge") && ok;
		return ok;
	}

	Skills::SkillType ManorScreen::Selected() const
	{
		return Skills::SKILL_GRID[m_y][m_x];
	}

	void ManorScreen::Open(Save::GameState &state)
	{
		m_state = &state;
		Clear();
		// SkillScreen.LoadContent: the manor at (0, 0), scale 2, all pieces hidden
		m_manor = new ObjContainer("TraitsCastle_Character");
		m_manor->Scale(Vector2(2, 2));
		for (int i = 0; i < Skills::SKILL_COUNT; i++)
		{
			SpriteObj *icon = new SpriteObj(Skills::SKILLS[i].iconName);
			m_icons.push_back(icon);
		}
		m_halo = new SpriteObj("IconHalo_Sprite");
		m_halo->Scale(Vector2(1.1f, 1.1f));
		m_halo->PlayAnimation(true);
		Refresh();
		// Start on the cheapest visible skill: the smithy on a new game
		for (int r = 0; r < Skills::GRID_ROWS; r++)
			for (int c = 0; c < Skills::GRID_COLS; c++)
				if (Skills::SKILL_GRID[r][c] == Skills::STARTING_SKILL) Select(c, r);
	}

	void ManorScreen::Refresh()
	{
		if (m_manor == nullptr) return;
		for (int i = 0; i < m_manor->NumChildren(); i++)
			if (m_manor->GetChildAt(i)) m_manor->GetChildAt(i)->Visible(false);
		if (m_manor->GetChildAt(23)) m_manor->GetChildAt(23)->Visible(true); // the road (SkillScreen.OnEnter)
		for (int r = 0; r < Skills::GRID_ROWS; r++)
			for (int c = 0; c < Skills::GRID_COLS; c++)
			{
				Skills::SkillType t = Skills::SKILL_GRID[r][c];
				int i = SkillIndex(t);
				if (i < 0) continue;
				int level = m_state->skills.Level(t);
				SpriteObj *icon = m_icons[i];
				icon->ChangeSprite(IconFor(Skills::SKILLS[i], level));
				icon->Position(Vector2(Skills::SKILL_POSITION[r][c].x, Skills::SKILL_POSITION[r][c].y));
				icon->Visible(m_state->skills.Visible(t));
				int piece = Skills::MANOR_PIECE[r][c];
				if (level > 0 && piece >= 0 && piece < m_manor->NumChildren() && m_manor->GetChildAt(piece))
					m_manor->GetChildAt(piece)->Visible(true);
			}
	}

	void ManorScreen::Select(int x, int y)
	{
		m_x = x;
		m_y = y;
		const Skills::IconPos &p = Skills::SKILL_POSITION[y][x];
		m_halo->Position(Vector2(p.x, p.y));
		// The camera rises for the icon above the manor (SkillScreen: (7, 1))
		m_camera.Y(p.y < 100 ? 360 - (100 - p.y) : 360);
	}

	bool ManorScreen::Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds)
	{
		(void) buttons;
		m_camera.ElapsedTotalSeconds(elapsedSeconds);
		if (m_flash > 0) m_flash -= elapsedSeconds;
		if (m_state == nullptr) return false;

		// Moves follow SkillBuilder.GetSkillLinker; only to skills already shown
		for (int i = 0; i < Skills::SKILL_LINK_COUNT; i++)
		{
			const Skills::SkillLink &l = Skills::SKILL_LINKS[i];
			if (l.x != m_x || l.y != m_y) continue;
			int nx = -1, ny = -1;
			if (pressed & BTN_UP) { nx = l.upX; ny = l.upY; }
			else if (pressed & BTN_DOWN) { nx = l.downX; ny = l.downY; }
			else if (pressed & BTN_LEFT) { nx = l.leftX; ny = l.leftY; }
			else if (pressed & BTN_RIGHT) { nx = l.rightX; ny = l.rightY; }
			if (nx >= 0 && ny >= 0 && m_state->skills.Visible(Skills::SKILL_GRID[ny][nx])) Select(nx, ny);
			break;
		}

		if (pressed & (BTN_B | BTN_A))
		{
			if (m_state->skills.Buy(Selected(), m_state->Gold)) Refresh();
			else m_flash = 0.6; // too expensive or maxed
		}
		return (pressed & BTN_START) != 0;
	}

	void ManorScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		// Night sky (TraitsBG would need a 16 MB sheet: plain colours instead)
		gfx_clear(0xFF1A2440u);
		gfx_fill(0, 340, SCREEN_W, 140, 0xFF2A3A2Cu);
		if (m_state == nullptr || m_manor == nullptr) return;

		m_manor->Draw(&m_camera);
		m_halo->Draw(&m_camera);
		for (SpriteObj *icon : m_icons)
			if (icon->Visible()) icon->Draw(&m_camera);

		// Details panel
		gfx_fill(PANEL_X - 4, 0, SCREEN_W - PANEL_X + 4, SCREEN_H, 0xE0101018u);
		gfx_fill(PANEL_X - 4, 0, 2, SCREEN_H, GOLD);
		char buf[128];
		const float s = 0.3f, w = (float) (SCREEN_W - PANEL_X - 6);
		float y = 8;
		snprintf(buf, sizeof(buf), "%d", m_state->Gold);
		m_titleFont.DrawOutlined(frame, buf, SCREEN_W - 8, y, 0.3f, GOLD, BLACK, 1, Text::Align::Right);
		y += 40;

		int i = SkillIndex(Selected());
		if (i >= 0)
		{
			const Skills::SkillInfo &info = Skills::SKILLS[i];
			Skills::SkillType t = info.type;
			int level = m_state->skills.Level(t);
			auto line = [&](const std::string &text, uint32_t colour, float scale)
			{
				std::string wrapped = m_font.Wrap(text, scale, w);
				m_font.DrawOutlined(frame, wrapped.c_str(), PANEL_X, y, scale, colour, BLACK, 1);
				int lines = 1;
				for (char c : wrapped) lines += c == '\n';
				y += m_font.LineSpacing() * scale * lines + 3;
			};
			line(Text::Loc::Get(info.nameLocID), GOLD, 0.38f);
			snprintf(buf, sizeof(buf), "%s : %d/%d", Text::Loc::Get("LOC_ID_SKILL_SCREEN_9"), level, info.maxLevel);
			line(buf, WHITE, s);
			if (info.displayStat)
			{
				std::string unit = Unit(info.unit);
				line(std::string(Text::Loc::Get("LOC_ID_SKILL_SCREEN_7")) + " : " + Amount(info, m_state->skills.ModifierAmount(t)) + unit, WHITE, s);
				if (level < info.maxLevel)
					line(std::string(Text::Loc::Get("LOC_ID_SKILL_SCREEN_8")) + " : +" + Amount(info, info.perLevelModifier) + unit, WHITE, s);
			}
			y += 4;
			line(Text::Loc::Get(info.descLocID), GREY, s);
			y += 6;
			if (level >= info.maxLevel)
				line(Text::Loc::Get("LOC_ID_SKILL_SCREEN_19"), GOLD, 0.34f);
			else
			{
				std::string cost = Text::Loc::Format("LOC_ID_SKILL_SCREEN_10_NEW", std::to_string(m_state->skills.TotalCost(t)),
					Text::Loc::Get(level > 0 ? "LOC_ID_SKILL_SCREEN_12" : "LOC_ID_SKILL_SCREEN_11"));
				bool affordable = m_state->Gold >= m_state->skills.TotalCost(t);
				line(cost, (m_flash > 0 && (int) (m_flash * 10) % 2) ? RED : (affordable ? GOLD : RED), 0.34f);
			}
		}

		// Hints
		std::string buy = WithButtons(Text::Loc::Get("LOC_ID_SKILL_SCREEN_1_NEW"));
		std::string leave = WithButtons(Text::Loc::Get("LOC_ID_SKILL_SCREEN_3_NEW"));
		// Stacked from the bottom of the panel
		float hy = SCREEN_H - 4;
		for (const std::string &hint : { leave, buy })
		{
			std::string wrapped = m_font.Wrap(hint, 0.28f, w);
			int lines = 1;
			for (char c : wrapped) lines += c == '\n';
			hy -= m_font.LineSpacing() * 0.28f * lines;
			m_font.DrawOutlined(frame, wrapped.c_str(), PANEL_X, hy, 0.28f, GREY, BLACK, 1);
			hy -= 4;
		}
	}
}
