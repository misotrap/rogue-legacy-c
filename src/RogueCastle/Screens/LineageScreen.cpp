#include "LineageScreen.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"
#include "../EVs/GameEV.h"
#include "../Skills/SkillTree.h"
#include "../Types/ClassType.h"
#include "../Types/PlayerPart.h"
#include "../Types/TraitType.h"

#include <cstdio>
#include <cstring>
#include <string>

using namespace DS2DEngine;

namespace RogueCastle
{
	namespace
	{
		// The screen is laid out in the game's 1320x720 coordinates, drawn
		// at 0.5 into the 640x480 frame (Camera2D: x * 0.5 - 10, y * 0.5 + 60)
		float SX(float x) { return x * 0.5f - 10.0f; }
		float SY(float y) { return y * 0.5f + 60.0f; }

		const uint32_t GOLD = 0xFFF0C850u, WHITE = 0xFFFFFFFFu, GREY = 0xFFB4AAB4u, BLACK = 0xFF000000u;

		// "PlayerIdleHead1_Sprite" -> "PlayerIdleHead<piece>_Sprite"
		std::string WithPiece(const std::string &spriteName, int piece)
		{
			std::string s = spriteName;
			size_t underscore = s.find('_');
			if (underscore == std::string::npos || underscore == 0) return s;
			s.erase(underscore - 1, 1);
			s.insert(underscore - 1, std::to_string(piece));
			return s;
		}

		// Game.NameToString: "Sieur {0} {1}" / "Dame {0} {1}"
		std::string FullName(const Lineage::Heir &h)
		{
			std::string s = Text::Loc::Format(h.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW", h.name, h.romanNumeral);
			while (!s.empty() && s.back() == ' ') s.pop_back();
			return s;
		}

		// "[Input:0]" -> the Miyoo button that does it
		std::string WithButtons(std::string s)
		{
			static const char *const tags[][2] = { { "[Input:0]", "B" }, { "[Input:9]", "X" } };
			for (const auto &t : tags)
			{
				size_t p = s.find(t[0]);
				if (p != std::string::npos) s.replace(p, strlen(t[0]), t[1]);
			}
			return s;
		}
	}

	LineageScreen::LineageScreen()
	{
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
	}

	LineageScreen::~LineageScreen()
	{
		ClearPortraits();
	}

	bool LineageScreen::LoadContent(const char *assetsDir)
	{
		bool ok = true;
		if (!SpriteSystem::SpriteLibrary::ContainsCharacter("PlayerIdle_Character"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "playerSpritesheetBig") && ok;
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("DaggerIcon_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "startingRoomSpritesheet") && ok;
		ok = m_nameFont.Load(assetsDir, "JunicodeLarge") && ok;
		ok = m_textFont.Load(assetsDir, "Junicode") && ok;
		ok = m_titleFont.Load(assetsDir, "JunicodeLarge") && ok;
		return ok;
	}

	void LineageScreen::ClearPortraits()
	{
		for (ObjContainer *p : m_portraits) delete p;
		m_portraits.clear();
	}

	void LineageScreen::ApplyHeir(ObjContainer *sprite, const Lineage::Heir &h)
	{
		auto part = [&](int index) { return sprite->GetChildAt(index); };
		// SetPortrait: head, chest and both shoulders take the heir's pieces
		const int pieces[][2] = { { PlayerPart::Head, h.headPiece }, { PlayerPart::Chest, h.chestPiece },
			{ PlayerPart::ShoulderA, h.shoulderPiece }, { PlayerPart::ShoulderB, h.shoulderPiece } };
		for (const auto &p : pieces)
		{
			GameObj *o = part(p[0]);
			if (o != nullptr) o->ChangeSprite(WithPiece(o->SpriteName(), p[1]));
		}
		// UpdateData
		if (part(PlayerPart::Hair)) part(PlayerPart::Hair)->Visible(!h.HasTrait(TraitType::Baldness));
		const char *extra = nullptr;
		switch (h.classType)
		{
		case ClassType::Knight: case ClassType::Knight2: extra = "PlayerIdleShield_Sprite"; break;
		case ClassType::Banker: case ClassType::Banker2: extra = "PlayerIdleLamp_Sprite"; break;
		case ClassType::Wizard: case ClassType::Wizard2: extra = "PlayerIdleBeard_Sprite"; break;
		case ClassType::Ninja: case ClassType::Ninja2: extra = "PlayerIdleHeadband_Sprite"; break;
		case ClassType::Barbarian: case ClassType::Barbarian2: extra = "PlayerIdleHorns_Sprite"; break;
		default: break;
		}
		if (part(PlayerPart::Extra))
		{
			part(PlayerPart::Extra)->Visible(extra != nullptr);
			if (extra != nullptr) part(PlayerPart::Extra)->ChangeSprite(extra);
		}
		if (part(PlayerPart::Wings)) part(PlayerPart::Wings)->Visible(h.classType == ClassType::Dragon);
		if (part(PlayerPart::Head))
		{
			if (h.classType == ClassType::Dragon)
				part(PlayerPart::Head)->ChangeSprite("PlayerIdleHead" + std::to_string(PlayerPart::DragonHelm) + "_Sprite");
			else if (h.classType == ClassType::Traitor)
				part(PlayerPart::Head)->ChangeSprite("PlayerIdleHead" + std::to_string(PlayerPart::IntroHelm) + "_Sprite");
		}
		if (part(PlayerPart::Boobs)) part(PlayerPart::Boobs)->Visible(h.isFemale);
		if (part(PlayerPart::Bowtie)) part(PlayerPart::Bowtie)->Visible(h.isFemale);
		if (part(PlayerPart::Glasses)) part(PlayerPart::Glasses)->Visible(false);
		if (part(PlayerPart::Light)) part(PlayerPart::Light)->Visible(false);

		float scale = 2.0f;
		if (h.HasTrait(TraitType::Gigantism)) scale = GameEV::TRAIT_GIGANTISM;
		else if (h.HasTrait(TraitType::Dwarfism)) scale = GameEV::TRAIT_DWARFISM;
		sprite->Scale(Vector2(scale, scale));
		sprite->Rotation(h.HasTrait(TraitType::Vertigo) ? 180.0f : 0.0f); // FlipPortrait
		sprite->PlayAnimation(true);
	}

	void LineageScreen::BuildPortraits()
	{
		ClearPortraits();
		const std::vector<Lineage::Heir> &heirs = m_state->currentBranches;
		for (size_t i = 0; i < heirs.size(); i++)
		{
			ObjContainer *p = new ObjContainer("PlayerIdle_Character");
			ApplyHeir(p, heirs[i]);
			p->Position(Vector2(330.0f + 330.0f * (float) i, 290.0f));
			m_portraits.push_back(p);
		}
	}

	bool LineageScreen::CanReroll() const
	{
		return m_state != nullptr && !m_state->RerolledChildren && m_state->skills.Level(Skills::SkillType::Randomize_Children) > 0;
	}

	void LineageScreen::Open(Save::GameState &state, Lineage::Generator &generator)
	{
		m_state = &state;
		m_generator = &generator;
		if (state.currentBranches.empty())
			state.currentBranches = generator.CreateBranches(state.skills.ClassUnlocks(state.ChallengeLastBossBeaten), state.FamilyTreeNames());
		m_selected = 0;
		m_time = 0;
		BuildPortraits();
	}

	bool LineageScreen::Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds)
	{
		(void) buttons;
		m_time += elapsedSeconds;
		m_camera.ElapsedTotalSeconds(elapsedSeconds);
		if (m_state == nullptr || m_state->currentBranches.empty()) return false;
		int count = (int) m_state->currentBranches.size();
		if (pressed & BTN_LEFT) m_selected = (m_selected + count - 1) % count;
		if (pressed & BTN_RIGHT) m_selected = (m_selected + 1) % count;
		if ((pressed & BTN_X) && CanReroll())
		{
			// LineageScreen.RerollCurrentBranch
			m_state->RerolledChildren = true;
			m_state->currentBranches.clear();
			m_state->currentBranches = m_generator->CreateBranches(m_state->skills.ClassUnlocks(m_state->ChallengeLastBossBeaten),
				m_state->FamilyTreeNames());
			m_selected = 0;
			BuildPortraits();
		}
		if (pressed & (BTN_B | BTN_A))
		{
			// LineageScreen.StartGame: the chosen heir becomes the hero
			m_state->hero = m_state->currentBranches[m_selected];
			m_state->currentBranches.clear();
			m_state->RerolledChildren = false;
			return true;
		}
		return false;
	}

	void LineageScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		gfx_clear(0xFF120E16u);
		gfx_fill(0, 60, SCREEN_W, 360, 0xFF241C28u);
		if (m_state == nullptr) return;
		const std::vector<Lineage::Heir> &heirs = m_state->currentBranches;

		// Title and hints
		std::string title = WithButtons(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_1_NEW"));
		m_titleFont.DrawOutlined(frame, title.c_str(), SCREEN_W / 2, 66, 0.32f, GOLD, BLACK, 1, Text::Align::Centre);
		if (CanReroll())
		{
			std::string hint = WithButtons(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_4_NEW"));
			m_textFont.DrawOutlined(frame, hint.c_str(), SCREEN_W - 8, 8, 0.32f, GREY, BLACK, 1, Text::Align::Right);
		}

		// The three heirs
		for (size_t i = 0; i < heirs.size() && i < m_portraits.size(); i++)
		{
			const Lineage::Heir &h = heirs[i];
			float cx = SX(330.0f + 330.0f * (float) i);
			bool selected = (int) i == m_selected;
			if (selected)
			{
				// selection frame behind the portrait
				int x0 = (int) cx - 70, y0 = (int) SY(110);
				gfx_fill(x0, y0, 140, 2, GOLD);
				gfx_fill(x0, y0 + 112, 140, 2, GOLD);
				gfx_fill(x0, y0, 2, 114, GOLD);
				gfx_fill(x0 + 138, y0, 2, 114, GOLD);
			}
			m_portraits[i]->Draw(&m_camera);
			std::string name = FullName(h);
			m_nameFont.DrawOutlined(frame, name.c_str(), cx, SY(346), 0.24f, selected ? GOLD : WHITE, BLACK, 1, Text::Align::Centre);
			const char *cls = Text::Loc::Get(LocIDs::ClassType_ToStringID(h.classType, h.isFemale));
			m_textFont.DrawOutlined(frame, cls, cx, SY(412), 0.34f, WHITE, BLACK, 1, Text::Align::Centre);
			std::string traits;
			if (h.trait1 == 0 && h.trait2 == 0) traits = Text::Loc::Get("LOC_ID_LINEAGE_OBJ_1");
			else
			{
				if (h.trait1) traits += Text::Loc::Get(LocIDs::TraitType_ToStringID(h.trait1));
				if (h.trait2) traits += std::string(h.trait1 ? ", " : "") + Text::Loc::Get(LocIDs::TraitType_ToStringID(h.trait2));
			}
			std::string wrapped = m_textFont.Wrap(traits, 0.3f, 190);
			m_textFont.DrawOutlined(frame, wrapped.c_str(), cx, SY(446), 0.3f, GREY, BLACK, 1, Text::Align::Centre);
		}

		// Details of the selected heir: class, traits, spell
		if (m_selected < (int) heirs.size())
		{
			const Lineage::Heir &h = heirs[m_selected];
			float y = SY(500);
			const float left = 16, width = SCREEN_W - 32, scale = 0.3f;
			auto section = [&](const std::string &head, const std::string &body)
			{
				m_textFont.DrawOutlined(frame, head.c_str(), left, y, scale, GOLD, BLACK, 1);
				y += m_textFont.LineSpacing() * scale;
				std::string w = m_textFont.Wrap(body, scale, width);
				m_textFont.Draw(frame, w.c_str(), left, y, scale, WHITE);
				int lines = 1;
				for (char c : w) lines += c == '\n';
				y += m_textFont.LineSpacing() * scale * lines + 2;
			};
			section(std::string(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_5")) + " : " + Text::Loc::Get(LocIDs::ClassType_ToStringID(h.classType, h.isFemale)),
				Text::Loc::Get(LocIDs::ClassType_DescriptionID(h.classType)));
			for (uint8_t t : { h.trait1, h.trait2 })
				if (t != 0)
					section(Text::Loc::Get(LocIDs::TraitType_ToStringID(t)), Text::Loc::Get(LocIDs::TraitType_DescriptionID(t, h.isFemale)));
			section(std::string(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_8")) + " : " + Text::Loc::Get(LocIDs::SpellType_ToStringID(h.spell)),
				Text::Loc::Get(LocIDs::SpellType_DescriptionID(h.spell)));
		}
	}
}
