#include "LineageScreen.h"

#include <cmath>

#include "../../Audio/Audio.h"

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

		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const uint32_t OUTLINE = 0xFFB58E27u;   // Color(181, 142, 39), the plaque text outline
		const float SPACING = 400;               // m_xPosOffset
		const float BRANCH_Y[3] = { -450, 0, 450 };

		DS2DEngine::SpriteObj *Make(const char *name, float sx, float sy)
		{
			if (!SpriteSystem::SpriteLibrary::ContainsSprite(name)) return nullptr;
			DS2DEngine::SpriteObj *s = new DS2DEngine::SpriteObj(name);
			s->Scale(Vector2(sx, sy));
			s->ForceDraw(true);
			return s;
		}

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
			static const char *const tags[][2] = { { "[Input:0]", "B" }, { "[Input:9]", "X" }, { "[Input:24]", "Y" }, { "[Input:16]", "Haut/Bas" }, { "[Input:18]", "Gauche/Droite" }, { "[Button:LeftStick]", "Gauche/Droite" } };
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
		m_camera.Zoom(0.5f);
		m_screen.Position(Vector2(660, 360));
		m_screen.Zoom(0.5f);
	}

	LineageScreen::~LineageScreen()
	{
		Clear();
		delete m_frame; delete m_plaque; delete m_iconHolder; delete m_spellIcon;
		delete m_shadow; delete m_title; delete m_plate; delete m_bg;
	}

	bool LineageScreen::LoadContent(const char *assetsDir)
	{
		bool ok = true;
		if (!SpriteSystem::SpriteLibrary::ContainsCharacter("PlayerIdle_Character"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "playerSpritesheetBig") && ok;
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("DaggerIcon_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "startingRoomSpritesheet") && ok;
		static const char *const sheets[][2] = { { "LineageScreenFrame_Sprite", "parallaxBGSpritesheet" },
			{ "LineageScreenPlate_Sprite", "getItemScreenSpritesheet" }, { "BlacksmithUI_IconBG_Sprite", "blacksmithUISpritesheet" },
			{ "LineageTitleText_Sprite_FR", "languageSpritesheet" } };
		m_assetsDir = assetsDir;
		for (const auto &sh : sheets)
			if (!SpriteSystem::SpriteLibrary::ContainsSprite(sh[0]))
				ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, sh[1]) && ok;
		ok = m_nameFont.Load(assetsDir, "JunicodeLarge") && ok;
		ok = m_textFont.Load(assetsDir, "Junicode") && ok;
		ok = m_titleFont.Load(assetsDir, "JunicodeLarge") && ok;

		// LineageObj and LineageScreen.LoadContent
		m_frame = Make("LineageScreenFrame_Sprite", 2.8f, 2.8f);
		m_plaque = Make("LineageScreenPlaque1Long_Sprite", 1.8f, 2);
		m_iconHolder = Make("BlacksmithUI_IconBG_Sprite", 1, 1);
		m_spellIcon = Make("DaggerIcon_Sprite", 1, 1);
		// x11 in the original's 720 lines; taller here, the screen shows 960
		m_shadow = Make("LineageScreenShadow_Sprite", 11, 14);
		if (m_shadow != nullptr) { m_shadow->Opacity(0.9f); m_shadow->Position(Vector2(1320 / 2.0f, 720 / 2.0f - 10)); }
		m_title = Make("LineageTitleText_Sprite_FR", 1, 1);
		if (m_title == nullptr) m_title = Make("LineageTitleText_Sprite", 1, 1);
		if (m_title != nullptr) m_title->Position(Vector2(1320 / 2.0f, 720 * 0.1f));
		m_plate = Make("LineageScreenPlate_Sprite", 1, 1);
		if (m_plate != nullptr) m_plate->Position(Vector2(1320 - 400 - 30, (720 - 500) / 2.0f));
		m_bg = Make("LineageScreenBG_Sprite", 1, 1);
		return ok;
	}

	void LineageScreen::Clear()
	{
		for (Entry &e : m_entries) delete e.sprite;
		m_entries.clear();
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


	// LoadFamilyTreeData + AddLineageRow: the ancestors left to right, the
	// heirs in a column after them
	void LineageScreen::Build()
	{
		Clear();
		int era = GameEV::BASE_ERA;
		auto add = [&](const Lineage::Heir &h, float x, float y, bool dead, int enemies, bool boss)
		{
			Entry e;
			e.heir = h;
			e.x = x;
			e.y = y;
			e.dead = dead;
			e.sprite = new ObjContainer("PlayerIdle_Character");
			e.sprite->AnimationDelay(1 / 10.0f);
			ApplyHeir(e.sprite, h);
			if (e.sprite->NumChildren() > PlayerPart::Sword2)
			{
				e.sprite->GetChildAt(PlayerPart::Sword1)->Visible(false);
				e.sprite->GetChildAt(PlayerPart::Sword2)->Visible(false);
				e.sprite->GetChildAt(PlayerPart::Cape)->TextureColor(Color(255, 0, 0));
				e.sprite->GetChildAt(PlayerPart::Hair)->TextureColor(Color(255, 0, 0));
				e.sprite->GetChildAt(PlayerPart::Bowtie)->TextureColor(Color(251, 156, 172));
			}
			const char *cls = Text::Loc::Get(LocIDs::ClassType_ToStringID(h.classType, h.isFemale));
			if (dead)
			{
				// UpdateAge and UpdateClassRank: "Le Barbare inutile"
				char years[32];
				snprintf(years, sizeof(years), "%d - %d", era - h.childAge, era + h.age);
				e.years = years;
				era += h.age;
				const char *rank = "LOC_ID_LINEAGE_OBJ_4_NEW";
				static const char *const ranks[] = { "LOC_ID_LINEAGE_OBJ_4_NEW", "LOC_ID_LINEAGE_OBJ_5_NEW", "LOC_ID_LINEAGE_OBJ_6_NEW",
					"LOC_ID_LINEAGE_OBJ_7_NEW", "LOC_ID_LINEAGE_OBJ_8_NEW", "LOC_ID_LINEAGE_OBJ_9_NEW", "LOC_ID_LINEAGE_OBJ_10_NEW", "LOC_ID_LINEAGE_OBJ_11_NEW" };
				if (boss) rank = "LOC_ID_LINEAGE_OBJ_3_NEW";
				else rank = ranks[enemies / 5 < 7 ? enemies / 5 : 7];
				std::string id = std::string(rank) + (h.isFemale ? "_F" : "");
				const char *fmt = Text::Loc::Get(id.c_str());
				e.rank = Text::Loc::Format(strcmp(fmt, id.c_str()) != 0 ? id.c_str() : rank, cls);
			}
			else
				e.rank = cls;
			m_entries.push_back(e);
		};

		const std::vector<Save::FamilyTreeNode> &tree = m_state->familyTree;
		float x = 0;
		if (tree.empty())
		{
			// No ancestor yet: Sir Johannes, the knight
			Lineage::Heir j;
			j.name = "Johannes";
			j.age = 30;
			j.childAge = 5;
			add(j, x, 0, true, 50, false);
			x += SPACING;
		}
		for (const Save::FamilyTreeNode &n : tree)
		{
			Lineage::Heir h;
			h.name = n.name;
			h.romanNumeral = n.romanNumeral;
			h.isFemale = n.isFemale;
			h.classType = n.classType;
			h.trait1 = n.trait1;
			h.trait2 = n.trait2;
			h.age = n.age;
			h.childAge = n.childAge;
			h.headPiece = n.headPiece;
			h.chestPiece = n.chestPiece;
			h.shoulderPiece = n.shoulderPiece;
			add(h, x, 0, true, n.numEnemiesBeaten, n.beatenABoss);
			x += SPACING;
		}
		m_firstHeir = m_entries.size();
		const std::vector<Lineage::Heir> &heirs = m_state->currentBranches;
		for (size_t i = 0; i < heirs.size() && i < 3; i++)
			add(heirs[i], x, heirs.size() == 2 ? (i == 0 ? -200.0f : 200.0f) : BRANCH_Y[i], false, 0, false);
		m_xShift = 0;
		const Entry &sel = m_entries[m_firstHeir + m_selected];
		m_camX = sel.x;
		m_camY = sel.y;
		m_camera.Position(Vector2(m_camX, m_camY));
	}

	bool LineageScreen::CanReroll() const
	{
		return m_state != nullptr && !m_state->RerolledChildren && m_state->skills.Level(Skills::SkillType::Randomize_Children) > 0;
	}

	void LineageScreen::Open(Save::GameState &state, Lineage::Generator &generator)
	{
		m_state = &state;
		m_generator = &generator;
		// The title screen frees the plate's sheet (getItemScreen) to save memory
		SpriteSystem::SpriteLibrary::LoadSheet(m_assetsDir.c_str(), "getItemScreenSpritesheet");
		if (state.currentBranches.empty())
			state.currentBranches = generator.CreateBranches(state.skills.ClassUnlocks(state.ChallengeLastBossBeaten), state.FamilyTreeNames());
		m_selected = state.currentBranches.size() > 1 ? 1 : 0;   // the middle one
		m_time = 0;
		Build();
	}

	bool LineageScreen::Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds)
	{
		(void) buttons;
		m_time += elapsedSeconds;
		m_camera.ElapsedTotalSeconds(elapsedSeconds);
		if (m_state == nullptr || m_state->currentBranches.empty() || m_entries.empty()) return false;
		int count = (int) (m_entries.size() - m_firstHeir);
		if (m_xShift == 0)
		{
			// Up/Down: the heir above or below
			int before = m_selected;
			if ((pressed & BTN_UP) && m_selected > 0) m_selected--;
			if ((pressed & BTN_DOWN) && m_selected < count - 1) m_selected++;
			if (m_selected != before) Audio::PlaySound("frame_swap");
		}
		// Left/Right: back along the family tree, then forward again
		if ((pressed & BTN_LEFT) && (int) m_firstHeir + m_xShift > 0) { m_xShift--; Audio::PlaySound("frame_swap"); }
		if ((pressed & BTN_RIGHT) && m_xShift < 0) { m_xShift++; Audio::PlaySound("frame_swap"); }
		const Entry &sel = m_entries[m_firstHeir + m_selected];
		m_camX = sel.x + m_xShift * SPACING;
		m_camY = sel.y;
		// Tween.To(Camera, 0.3, QuadOut): ease towards the target
		Vector2 c = m_camera.Position();
		float k = (float) (1 - pow(0.0001, elapsedSeconds));   // ~0.3 s
		m_camera.Position(Vector2(c.X + (m_camX - c.X) * k, c.Y + (m_camY - c.Y) * k));

		if ((pressed & BTN_X) && CanReroll() && m_xShift == 0)
		{
			// LineageScreen.RerollCurrentBranch
			m_state->RerolledChildren = true;
			m_state->currentBranches = m_generator->CreateBranches(m_state->skills.ClassUnlocks(m_state->ChallengeLastBossBeaten),
				m_state->FamilyTreeNames());
			m_selected = m_state->currentBranches.size() > 1 ? 1 : 0;
			Build();
		}
		if (pressed & (BTN_B | BTN_A))
		{
			if (m_xShift != 0) { m_xShift = 0; return false; }   // back to the heirs first
			// LineageScreen.StartGame: the chosen heir becomes the hero
			m_state->hero = m_state->currentBranches[m_selected];
			m_state->HasProsopagnosia = false;
			m_state->currentBranches.clear();
			m_state->RerolledChildren = false;
			SpriteSystem::SpriteLibrary::FreeSheet("getItemScreenSpritesheet");
			return true;
		}
		return false;
	}

	// LineageObj.Draw: the frame, the spell icon, the portrait, the plaque
	// and its text (name, class, traits or years)
	void LineageScreen::DrawEntry(uint32_t *frame, const Entry &e)
	{
		auto sx = [&](float x) { return (x - m_camera.X()) * 0.5f + SCREEN_W / 2; };
		auto sy = [&](float y) { return (y - m_camera.Y()) * 0.5f + SCREEN_H / 2; };
		if (sx(e.x) < -200 || sx(e.x) > SCREEN_W + 200 || sy(e.y) < -200 || sy(e.y) > SCREEN_H + 200) return;
		float frameY = e.y - 12;
		if (m_frame != nullptr)
		{
			m_frame->Position(Vector2(e.x + 5, frameY));
			m_frame->Draw(&m_camera);
		}
		if (!e.dead && e.heir.spell != 0 && m_iconHolder != nullptr && m_frame != nullptr)
		{
			Vector2 at(e.x + 5, (float) m_frame->Bounds().Bottom() - 20);
			m_iconHolder->Position(at);
			m_iconHolder->Draw(&m_camera);
			const char *icon = LocIDs::SpellType_Icon(e.heir.spell);
			if (m_spellIcon != nullptr && SpriteSystem::SpriteLibrary::ContainsSprite(icon))
			{
				m_spellIcon->ChangeSprite(icon);
				m_spellIcon->Position(at);
				m_spellIcon->Draw(&m_camera);
			}
		}
		bool flip = e.heir.HasTrait(TraitType::Vertigo);
		e.sprite->Position(Vector2(e.x + (flip ? 0 : 10), e.y - (flip ? 30 : 0)));
		e.sprite->Draw(&m_camera);

		float plaqueTop = frameY + (m_frame != nullptr ? m_frame->Height() : 226) - 30;
		if (m_plaque != nullptr)
		{
			// the plaque widens for a long class line
			float w = 0, h = 0;
			m_textFont.Measure(e.rank.c_str(), w, h);
			float needed = w * 0.24f * 2 + 50, base = 145 * 1.8f;
			m_plaque->Scale(Vector2(needed > base ? 1.8f * needed / base : 1.8f, 2));
			m_plaque->Position(Vector2(e.x + 5, plaqueTop));
			m_plaque->Draw(&m_camera);
		}
		// the text: name at 140 under the centre, then the class line, then
		// the traits (the living) or the years (the dead)
		float cx = sx(e.x + 5), y = sy(e.y + 140) - 10;
		std::string name = FullName(e.heir);
		m_textFont.DrawOutlined(frame, name.c_str(), cx, y, 0.32f, WHITE, OUTLINE, 1, Text::Align::Centre);
		y += 16;
		m_textFont.DrawOutlined(frame, e.rank.c_str(), cx, y, 0.24f, WHITE, OUTLINE, 1, Text::Align::Centre);
		y += 14;
		std::string third;
		if (e.dead) third = e.years;
		else if (!m_state->HasProsopagnosia)
		{
			if (e.heir.trait1 == 0 && e.heir.trait2 == 0) third = Text::Loc::Get("LOC_ID_LINEAGE_OBJ_1");
			else
			{
				if (e.heir.trait1) third += Text::Loc::Get(LocIDs::TraitType_ToStringID(e.heir.trait1));
				if (e.heir.trait2) third += std::string(e.heir.trait1 ? ", " : "") + Text::Loc::Get(LocIDs::TraitType_ToStringID(e.heir.trait2));
			}
		}
		if (!third.empty())
			m_textFont.DrawOutlined(frame, third.c_str(), cx, y, 0.24f, WHITE, OUTLINE, 1, Text::Align::Centre);
	}

	// The description plate (LineageScreen.UpdateDescriptionPlate)
	void LineageScreen::DrawPlate(uint32_t *frame, const Lineage::Heir &h)
	{
		if (m_plate == nullptr) return;
		m_plate->Draw(&m_screen);
		const float px = SX(1320 - 400 - 30), top = SY((720 - 500) / 2.0f), width = 400 * 0.5f;
		float y = top + 8;
		std::string name = FullName(h);
		m_textFont.DrawOutlined(frame, name.c_str(), px + width / 2, y, 0.38f, WHITE, OUTLINE, 1, Text::Align::Centre);
		y += 22;
		const float left = px + 10, indent = px + 20, wrapW = width - 30;
		auto section = [&](const std::string &title, const std::string &text)
		{
			m_textFont.DrawOutlined(frame, title.c_str(), left, y, 0.3f, WHITE, OUTLINE, 1);
			y += 16;
			std::string w = m_textFont.Wrap(text, 0.24f, wrapW);
			m_textFont.DrawOutlined(frame, w.c_str(), indent, y, 0.24f, WHITE, OUTLINE, 1);
			int lines = 1;
			for (char c : w) lines += c == '\n';
			y += lines * m_textFont.LineSpacing() * 0.24f + 6;
		};
		std::string sep = " - ";
		section(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_5") + sep + Text::Loc::Get(LocIDs::ClassType_ToStringID(h.classType, h.isFemale)),
			WithButtons(Text::Loc::Get(LocIDs::ClassType_DescriptionID(h.classType))));
		if (h.trait1 == 0 && h.trait2 == 0)
		{
			m_textFont.DrawOutlined(frame, Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_7"), left, y, 0.3f, WHITE, OUTLINE, 1);
			y += 22;
		}
		for (uint8_t t : { h.trait1, h.trait2 })
			if (t != 0)
				section(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_6") + sep + Text::Loc::Get(LocIDs::TraitType_ToStringID(t)),
					Text::Loc::Get(LocIDs::TraitType_DescriptionID(t, h.isFemale)));
		section(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_8") + sep + Text::Loc::Get(LocIDs::SpellType_ToStringID(h.spell)),
			WithButtons(Text::Loc::Get(LocIDs::SpellType_DescriptionID(h.spell))));
	}

	void LineageScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		gfx_clear(BLACK);
		if (m_state == nullptr || m_entries.empty()) return;

		// The red damask wall, repeated and moving with the camera
		if (m_bg != nullptr)
		{
			float tw = m_bg->Width() - 2, th = m_bg->Height() - 2;   // overlap: no seams
			Rectangle view = m_camera.Bounds();
			float x0 = floorf(view.X / tw) * tw, y0 = floorf(view.Y / th) * th;
			for (float y = y0; y < view.Y + view.Height; y += th)
				for (float x = x0; x < view.X + view.Width; x += tw)
				{
					m_bg->Position(Vector2(x, y));
					m_bg->Draw(&m_camera);
				}
		}
		for (const Entry &e : m_entries) DrawEntry(frame, e);

		// The spotlight, the title, the hints and the plate on top
		if (m_shadow != nullptr) m_shadow->Draw(&m_screen);
		if (m_title != nullptr) m_title->Draw(&m_screen);
		std::string confirm = WithButtons(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_1_NEW"));
		m_textFont.DrawOutlined(frame, confirm.c_str(), SX(1320 - 40), SY(630) - 10, 0.38f, WHITE, BLACK, 1, Text::Align::Right);
		std::string nav = WithButtons(Text::Loc::Get(m_firstHeir > 0 ? "LOC_ID_LINEAGE_SCREEN_2_NEW" : "LOC_ID_LINEAGE_SCREEN_3"));
		m_textFont.DrawOutlined(frame, nav.c_str(), SX(1320 - 40), SY(670) - 10, 0.38f, WHITE, BLACK, 1, Text::Align::Right);
		if (CanReroll())
		{
			std::string reroll = WithButtons(Text::Loc::Get("LOC_ID_LINEAGE_SCREEN_4_NEW"));
			m_textFont.DrawOutlined(frame, reroll.c_str(), SX(30), SY(670) - 10, 0.38f, WHITE, BLACK, 1);
		}
		if (m_xShift == 0) DrawPlate(frame, m_entries[m_firstHeir + m_selected].heir);
	}
}
