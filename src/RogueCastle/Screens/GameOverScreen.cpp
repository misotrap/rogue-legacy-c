#include "GameOverScreen.h"
#include "EnemyNames.h"

#include "../../Audio/Audio.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstdio>
#include <cstdlib>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const char *SHEET = "uiExtraSpritesheet";   // tools/make_subsheet.py
		const int NUM_HINTS = 44;   // GameEV.GAME_HINTS: LOC_ID_HINT_1..44
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const float GHOST_Y = 280;
		const uint32_t HINT_COLOUR = 0xFFFFFE80u;   // Color(255, 254, 128)

		SpriteObj *Make(const char *name, float x, float y)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			s->Position(Vector2(x, y));
			return s;
		}

		float Clamp01(double v) { return v < 0 ? 0 : v > 1 ? 1 : (float) v; }

		// frame = last * (1 - k), k in 0..1
		void Darken(uint32_t *frame, const std::vector<uint32_t> &last, float k)
		{
			uint32_t m = (uint32_t) ((1 - k) * 256);
			for (size_t i = 0; i < last.size(); i++)
			{
				uint32_t c = last[i];
				frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * m >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * m >> 8) & 0xFF00u);
			}
		}
	}

	// The hero's parts as in the castle (UpdateEquipmentObjs/Colours)
	void GameOverScreen::DressHero(const char *sprite)
	{
		m_hero->ChangeSprite(sprite);
		for (int i = 0; i < m_hero->NumChildren(); i++)
		{
			if (i < (int) m_look.visible.size()) m_hero->GetChildAt(i)->Visible(m_look.visible[i]);
			if (i < (int) m_look.colours.size()) m_hero->GetChildAt(i)->TextureColor(m_look.colours[i]);
		}
		const int EXTRA = 15;   // PlayerPart.Extra
		if (!m_look.extraSuffix.empty() && m_hero->NumChildren() > EXTRA)
		{
			std::string base = sprite;
			base.replace(base.find("_Character"), 10, m_look.extraSuffix);
			if (SpriteSystem::SpriteLibrary::ContainsSprite(base)) m_hero->GetChildAt(EXTRA)->ChangeSprite(base);
		}
	}

	bool GameOverScreen::Open(const char *assetsDir, const std::vector<uint32_t> &lastFrame, const std::string &heroName, bool isFemale, const HeroLook &look,
	                          int killerKind, const std::string &killerName)
	{
		Close();
		bool ok = m_font.Loaded() || m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("GameOverSpotlight_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, SHEET) && ok;
		m_lastFrame = lastFrame;
		m_lastFrame.resize((size_t) SCREEN_W * SCREEN_H, BLACK);

		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		// The light cone points down onto the body (the screen centre)
		m_spotlight = Make("GameOverSpotlight_Sprite", 1320 / 2, 40 + 150);
		m_spotlight->Rotation(90);
		m_ghost = Make("PlayerGhost_Sprite", 1320 / 2 - 36, GHOST_Y);
		m_ghost->AnimationDelay(1 / 10.0f);
		m_ghost->PlayAnimation(true);
		m_plate = Make("DialogBox_Sprite", 1320 / 2, 610);
		m_look = look;
		m_female = isFemale;
		m_hero = new DS2DEngine::ObjContainer("PlayerHurt_Character");
		m_hero->Scale(Vector2(look.scale, look.scale));
		m_hero->Flip(look.flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);
		m_hero->Position(Vector2(look.x, look.y));
		m_hero->ForceDraw(true);
		DressHero("PlayerHurt_Character");

		std::string name = heroName;   // "Sieur Lee " when there is no numeral
		while (!name.empty() && name.back() == ' ') name.pop_back();
		// GameOverScreen.cs line 395: "terrassé par" a boss, "tué par" an
		// enemy, a shot without a shooter, a trap, or just "tué"
		const char *slainID = killerKind == 2 ? "LOC_ID_GAME_OVER_SCREEN_3_NEW" : killerKind == 1 ? "LOC_ID_GAME_OVER_SCREEN_4_NEW"
		                    : killerKind == 3 ? "LOC_ID_GAME_OVER_SCREEN_5_NEW" : killerKind == 4 ? "LOC_ID_GAME_OVER_SCREEN_6_NEW"
		                    : "LOC_ID_GAME_OVER_SCREEN_7_NEW";
		std::string enemy = killerName;
		for (const EnemyNameLoc &e : ENEMY_NAMES)
			if (killerName == e.name) { enemy = Text::Loc::Get(e.locID); break; }
		m_slain = Text::Loc::Format((std::string(slainID) + (isFemale ? "_F" : "")).c_str(), name, enemy);
		char id[32];
		snprintf(id, sizeof(id), "LOC_ID_HINT_%d", 1 + rand() % NUM_HINTS);
		m_hint = Text::Loc::Get(id);
		m_partingTitle = Text::Loc::Format(isFemale ? "LOC_ID_GAME_OVER_SCREEN_8_NEW_F" : "LOC_ID_GAME_OVER_SCREEN_8_NEW", name);
		m_continue = Text::Loc::Get("LOC_ID_GAME_OVER_SCREEN_1_NEW");
		size_t at = m_continue.find("[Input:0]");
		if (at != std::string::npos) m_continue.replace(at, 9, "A");
		m_time = 0;
		m_open = true;
		return ok;
	}

	void GameOverScreen::Close()
	{
		if (!m_open) return;
		delete m_spotlight; delete m_ghost; delete m_plate; delete m_hero;
		m_hero = nullptr;
		m_spotlight = m_ghost = m_plate = nullptr;
		m_lastFrame.clear();
		m_lastFrame.shrink_to_fit();
		m_open = false;
	}

	bool GameOverScreen::Update(uint32_t pressed, double dt)
	{
		if (!m_open) return true;
		// GameOverScreen.OnEnter: the music stops, the stinger plays at 1.2 s
		if (m_time == 0) { Audio::StopMusic(); Audio::PlaySound("Player_Death_FadeToBlack"); }
		if (m_time < 1.2 && m_time + dt >= 1.2) Audio::PlayMusic("GameOverStinger", false);
		double before = m_time;
		m_time += dt;
		m_camera.ElapsedTotalSeconds(dt);
		// The camera centres on the body (Tween Camera 1 s, QuadInOut): the
		// hero slides to the middle, feet 10 px under the centre
		float e = (float) Clamp01(m_time);
		e = e < 0.5f ? 2 * e * e : 1 - 2 * (1 - e) * (1 - e);
		m_hero->X(m_look.x + (660 - m_look.x) * e);
		m_hero->Y(m_look.y + (370 - m_look.feet - m_look.y) * e);
		if (before < 1.1 && m_time >= 1.1) Audio::PlaySound("Player_Death_Spotlight");
		if (before < 2 && m_time >= 2)
		{
			// RunDeathAnimation1
			if (m_female) Audio::PlaySound("Player_Female_Death_01", "Player_Female_Death_02");
			else Audio::PlaySound("Player_Male_Death_01", "Player_Male_Death_02", "Player_Male_Death_03");
			DressHero("PlayerDeath_Character");
			m_hero->PlayAnimation(false);
		}
		// The ghost: 0.4 opacity at 3 s, rises 150 over 2 s, gone at 4.5 s
		double g = m_time - 3;
		m_ghost->Opacity(g < 0 ? 0 : g < 0.5 ? 0.8f * (float) g : g < 1 ? 0.4f : g < 1.5 ? 0.4f * (float) (1.5 - g) / 0.5f : 0);
		m_ghost->Y(GHOST_Y - 150 * Clamp01(g / 2));
		return m_time > 4 && (pressed & (BTN_A | BTN_START)) != 0;
	}

	void GameOverScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		if (!m_open) { gfx_clear(BLACK); return; }
		Darken(frame, m_lastFrame, Clamp01(m_time / 0.5));
		if (m_time >= 1) m_spotlight->Draw(&m_camera);
		m_hero->Draw(&m_camera);
		if (m_time >= 3) m_ghost->Draw(&m_camera);

		// The dialogue plate fades in with the spotlight
		if (m_time >= 1)
		{
			m_plate->Opacity(Clamp01((m_time - 1) / 0.5));
			m_plate->Draw(&m_camera);
			const float cx = SCREEN_W / 2, top = 365 - 45;
			m_font.DrawOutlined(frame, m_slain.c_str(), cx, top + 8, 0.42f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, m_hint.c_str(), cx, top + 36, 0.3f, HINT_COLOUR, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, m_partingTitle.c_str(), cx + 220, top + 66, 0.22f, WHITE, BLACK, 1, Text::Align::Right);
		}
		if (m_time > 4)
			m_font.DrawOutlined(frame, m_continue.c_str(), SCREEN_W - 20, 70, 0.3f, WHITE, BLACK, 1, Text::Align::Right);
	}
}
