/* Port of RogueCastle/src/Screens/GameOverScreen.cs. */
#include "GameOverScreen.h"
#include "EnemyNames.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../../Text/LocIDs.h"

#include "../../Audio/Audio.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cmath>
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
		const uint32_t OUTLINE = 0xFFB58E27u;   // the lineage plaque outline
		const uint32_t HINT_COLOUR = 0xFFFFFE80u;   // Color(255, 254, 128)
		/* The C# caps nothing, but every killed enemy becomes a drawn
		 * container here: 60 covers a long run without bliting the
		 * whole bestiary. */
		const size_t PARADE_MAX = 60;

		SpriteObj *Make(const char *name, float x, float y)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			s->Position(Vector2(x, y));
			return s;
		}

		float Clamp01(double v) { return v < 0 ? 0 : v > 1 ? 1 : (float) v; }
		float QuadInOut(double t)
		{
			t = Clamp01(t);
			return (float) (t < 0.5 ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t));
		}

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

	// A hero container dressed as the dead one (UpdateEquipmentObjs/Colours)
	DS2DEngine::ObjContainer *GameOverScreen::MakeHero(const char *sprite, float scale)
	{
		DS2DEngine::ObjContainer *hero = new DS2DEngine::ObjContainer(sprite);
		hero->Scale(Vector2(scale, scale));
		hero->Flip(m_look.flip ? SpriteEffects::FlipHorizontally
		                       : SpriteEffects::None);
		hero->ForceDraw(true);
		Dress(hero, sprite);
		return hero;
	}

	void GameOverScreen::Dress(DS2DEngine::ObjContainer *hero, const char *sprite)
	{
		hero->ChangeSprite(sprite);
		for (int i = 0; i < hero->NumChildren(); i++)
		{
			if (i < (int) m_look.visible.size()) hero->GetChildAt(i)->Visible(m_look.visible[i]);
			if (i < (int) m_look.colours.size()) hero->GetChildAt(i)->TextureColor(m_look.colours[i]);
		}
		const int EXTRA = 15;   // PlayerPart.Extra
		if (!m_look.extraSuffix.empty() && hero->NumChildren() > EXTRA)
		{
			std::string base = sprite;
			base.replace(base.find("_Character"), 10, m_look.extraSuffix);
			if (SpriteSystem::SpriteLibrary::ContainsSprite(base)) hero->GetChildAt(EXTRA)->ChangeSprite(base);
		}
	}

	bool GameOverScreen::Open(const char *assetsDir, const std::vector<uint32_t> &lastFrame,
	                          const Lineage::Heir &hero, const HeroLook &look,
	                          const std::vector<std::pair<uint8_t, int>> &kills,
	                          int killerKind, const std::string &killerName)
	{
		Close();
		m_look = look;
		m_heir = hero;
		m_female = hero.isFemale;
		bool ok = m_font.Loaded() || m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("GameOverSpotlight_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, SHEET) && ok;
		m_lastFrame = lastFrame;
		m_lastFrame.resize((size_t) SCREEN_W * SCREEN_H, BLACK);

		/* The camera starts as the castle view (Zoom 0.5) and closes in on
		 * the body (GameOverScreen.cs line 270: Tween Camera 1 s to the
		 * player, feet 10 under the centre, Zoom 1). */
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		// The light cone points down onto the body (LoadContent line 126)
		m_spotlight = Make("GameOverSpotlight_Sprite", 1320 / 2, 40 + 150);
		m_spotlight->Rotation(90);
		m_plate = Make("DialogBox_Sprite", 1320 / 2, 610);
		// The heir's framed portrait (m_playerFrame, line 137): above the body
		m_frame = Make("LineageScreenFrame_Sprite", look.x, look.y - 120 - 82);
		m_frame->Scale(Vector2(2, 2));
		m_portrait = MakeHero("PlayerIdle_Character", look.scale);
		m_portrait->Position(Vector2(look.x + 10, look.y - 120));
		m_plaque = Make("LineageScreenPlaque1Long_Sprite", look.x, look.y - 120 + 82 - 10);
		m_plaque->Scale(Vector2(1.8f, 1.8f));
		m_hero = MakeHero("PlayerHurt_Character", look.scale);
		m_hero->Position(Vector2(look.x, look.y));

		/* DropStats (line 300): the killed enemies of the run, laid out in
		 * a line from the camera's top-left + (200, 450), 25 apart,
		 * wrapping every 950, half scale, facing left. */
		m_parade.clear();
		m_paradeAt.clear();
		double delay = 0;
		float px = look.x - 320 + 200, py = look.y + look.feet - 10 - 240 + 450;
		for (const std::pair<uint8_t, int> &kill : kills)
		{
			if (m_parade.size() >= PARADE_MAX) break;
			EnemyObj *enemy = RoomObj::CreateEnemyOfType(kill.first, kill.second, nullptr);
			if (enemy == nullptr) continue;
			enemy->IsPaused = true;
			enemy->DisableAllWeightFlag = true;
			
			enemy->IsCollidableFlag = false;
			enemy->Scale(Vector2(enemy->Scale().X * 0.5f, enemy->Scale().Y * 0.5f));
			enemy->Flip(SpriteEffects::FlipHorizontally);
			enemy->Opacity(0);
			enemy->PlayAnimation(true);
			enemy->Position(Vector2(px, py));
			px += 25;
			if (px + 60 > look.x - 320 + 200 + 950)
			{
				px = look.x - 320 + 200;
				py += 30;
			}
			m_parade.push_back(enemy);
			m_paradeAt.push_back(4 + delay);
			delay += 0.05;
		}

		std::string name = Text::Loc::Format(m_female ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
		                                     hero.name, hero.romanNumeral);
		while (!name.empty() && name.back() == ' ') name.pop_back();
		// GameOverScreen.cs line 395: "slain by" a boss, "killed by" an
		// enemy, a shot without a shooter, a trap, or just "killed"
		const char *slainID = killerKind == 2 ? "LOC_ID_GAME_OVER_SCREEN_3_NEW" : killerKind == 1 ? "LOC_ID_GAME_OVER_SCREEN_4_NEW"
		                    : killerKind == 3 ? "LOC_ID_GAME_OVER_SCREEN_5_NEW" : killerKind == 4 ? "LOC_ID_GAME_OVER_SCREEN_6_NEW"
		                    : "LOC_ID_GAME_OVER_SCREEN_7_NEW";
		std::string enemy = killerName;
		for (const EnemyNameLoc &e : ENEMY_NAMES)
			if (killerName == e.name) { enemy = Text::Loc::Get(e.locID); break; }
		m_slain = Text::Loc::Format((std::string(slainID) + (m_female ? "_F" : "")).c_str(), name, enemy);
		char id[32];
		snprintf(id, sizeof(id), "LOC_ID_HINT_%d", 1 + rand() % NUM_HINTS);
		m_hint = Text::Loc::Get(id);
		m_partingTitle = Text::Loc::Format(m_female ? "LOC_ID_GAME_OVER_SCREEN_8_NEW_F" : "LOC_ID_GAME_OVER_SCREEN_8_NEW", name);
		m_continue = Text::Loc::Get("LOC_ID_GAME_OVER_SCREEN_1_NEW");
		size_t at = m_continue.find("[Input:0]");
		if (at != std::string::npos) m_continue.replace(at, 9, "A");
		// The heir's card: the class line and the traits (LineageObj)
		m_classLine = Text::Loc::Get(LocIDs::ClassType_ToStringID(hero.classType, m_female));
		if (hero.trait1 == 0 && hero.trait2 == 0)
			m_traitLine = Text::Loc::Get("LOC_ID_LINEAGE_OBJ_1");
		else
		{
			if (hero.trait1) m_traitLine += Text::Loc::Get(LocIDs::TraitType_ToStringID(hero.trait1));
			if (hero.trait2) m_traitLine += std::string(hero.trait1 ? ", " : "") + Text::Loc::Get(LocIDs::TraitType_ToStringID(hero.trait2));
		}
		m_time = 0;
		m_ghostDrift = 0;
		m_open = true;
		return ok;
	}

	void GameOverScreen::Close()
	{
		if (!m_open) return;
		delete m_spotlight; delete m_ghost; delete m_plate; delete m_hero;
		delete m_frame; delete m_plaque; delete m_portrait;
		for (EnemyObj *e : m_parade) delete e;
		m_parade.clear();
		m_paradeAt.clear();
		m_hero = m_portrait = nullptr;
		m_spotlight = m_ghost = m_plate = m_frame = m_plaque = nullptr;
		m_lastFrame.clear();
		m_lastFrame.shrink_to_fit();
		m_open = false;
	}

	bool GameOverScreen::Update(uint32_t pressed, double dt)
	{
		if (!m_open) return true;
		// GameOverScreen.OnEnter: the music stops, the stinger plays at 1.2 s
		if (m_time == 0) { Audio::StopMusic(); Audio::PlaySound("Player_Death_FadeToBlack"); }
		double before = m_time;
		m_time += dt;
		m_camera.ElapsedTotalSeconds(dt);

		/* The camera closes in on the body: 1 s from the castle view to
		 * (the player, his feet 10 under the centre) at Zoom 1. */
		if (m_time >= 1 && m_time <= 2)
		{
			float e = QuadInOut(m_time - 1);
			m_camera.X(660 + (m_look.x - 660) * e);
			m_camera.Y(360 + (m_look.y + m_look.feet - 10 - 360) * e);
			m_camera.Zoom(0.5f + 0.5f * e);
		}
		else if (m_time > 2)
		{
			m_camera.X(m_look.x);
			m_camera.Y(m_look.y + m_look.feet - 10);
			m_camera.Zoom(1);
		}

		if (before < 1.1 && m_time >= 1.1) Audio::PlaySound("Player_Death_Spotlight");
		if (before < 1.2 && m_time >= 1.2) Audio::PlayMusic("GameOverStinger", false);
		if (before < 2 && m_time >= 2)
		{
			// RunDeathAnimation1
			if (m_female) Audio::PlaySound("Player_Female_Death_01", "Player_Female_Death_02");
			else Audio::PlaySound("Player_Male_Death_01", "Player_Male_Death_02");
			Dress(m_hero, "PlayerDeath_Character");
			m_hero->PlayAnimation(false);
		}
		/* The death animation's own sounds (PassInData line 55): the
		 * sword twirl at frame 2, the sword landing at 9, the body
		 * falling at 14. */
		if (m_time > 2)
		{
			int frame = m_hero->CurrentFrame();
			if (frame >= 2 && !m_deathSound[2]) { m_deathSound[2] = true; Audio::PlaySound("Player_Death_SwordTwirl"); }
			if (frame >= 9 && !m_deathSound[9]) { m_deathSound[9] = true; Audio::PlaySound("Player_Death_SwordLand"); }
			if (frame >= 14 && !m_deathSound[14]) { m_deathSound[14] = true; Audio::PlaySound("Player_Death_BodyFall"); }
		}
		/* The ghost (line 168): 0.4 opacity at 3 s, rises 150 over 2 s
		 * swaying (Draw line 528: X += sin(t*5) * 60 * dt), gone at 4.5. */
		if (before < 3 && m_time >= 3)
		{
			Audio::PlaySound("Player_Ghost");
			if (m_ghost == nullptr)
			{
				m_ghost = Make("PlayerGhost_Sprite", m_look.x - 36, m_look.y - 130);
				m_ghost->AnimationDelay(1 / 10.0f);
				m_ghost->PlayAnimation(true);
			}
		}
		if (m_time >= 3 && m_ghost != nullptr)
		{
			double g = m_time - 3;
			m_ghost->Opacity(g < 0.5 ? 0.8f * (float) g
			                : g < 3.5 ? 0.4f
			                : g < 4 ? 0.4f * (float) (4 - g) / 0.5f : 0.0f);
			m_ghost->Y(m_look.y - 130 - 150 * Clamp01(g / 2));
			m_ghostDrift += sinf((float) m_time * 5) * 60 * (float) dt;
			m_ghost->X(m_look.x - 36 + m_ghostDrift);
		}
		/* DropStats at 4 s: the parade appears enemy by enemy. */
		for (size_t i = 0; i < m_parade.size(); i++)
			if (before < m_paradeAt[i] && m_time >= m_paradeAt[i])
			{
				m_parade[i]->Opacity(1);
				Audio::PlaySound("Enemy_Kill_Plant");
			}
		return m_time > 4 && (pressed & (BTN_A | BTN_START)) != 0;
	}

	void GameOverScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		if (!m_open) { gfx_clear(BLACK); return; }
		Darken(frame, m_lastFrame, Clamp01(m_time / 0.5));
		float zoom = m_camera.Zoom();
		// camera space -> screen (the world draws go through m_camera, the
		// texts are screen space)
		auto sx = [&](float wx) { return (wx - m_camera.X()) * zoom + SCREEN_W / 2; };

		// The parade of the killed enemies (DropStats)
		for (size_t i = 0; i < m_parade.size(); i++)
			if (m_time >= m_paradeAt[i])
				m_parade[i]->Draw(&m_camera);
		// The heir's framed portrait (m_playerFrame), fading in over 1 s at 4 s
		float card = m_time < 4 ? 0 : Clamp01((m_time - 4) / 1);
		if (card > 0)
		{
			m_frame->Opacity(card);
			m_portrait->Opacity(card);
			m_plaque->Opacity(card);
			m_frame->Draw(&m_camera);
			m_portrait->Draw(&m_camera);
			m_plaque->Draw(&m_camera);
			float cx = sx(m_look.x);
			float y = (m_look.y - 120 + 82 - 10 + 20 - m_camera.Y()) * zoom + 240;
			m_font.DrawOutlined(frame, m_heir.name.c_str(), cx, y, 0.32f * zoom, WHITE, OUTLINE, 1, Text::Align::Centre);
			y += 16;
			m_font.DrawOutlined(frame, m_classLine.c_str(), cx, y, 0.24f * zoom, WHITE, OUTLINE, 1, Text::Align::Centre);
			y += 14;
			m_font.DrawOutlined(frame, m_traitLine.c_str(), cx, y, 0.24f * zoom, WHITE, OUTLINE, 1, Text::Align::Centre);
		}
		m_hero->Draw(&m_camera);
		if (m_time >= 3 && m_ghost != nullptr && m_ghost->Opacity() > 0.01f)
			m_ghost->Draw(&m_camera);

		// The spotlight comes with the fade to black (1 s, 0.1 s ramp)
		if (m_time >= 1) m_spotlight->Draw(&m_camera);
		// The dialogue plate fades in with the death animation (2 s)
		if (m_time >= 2)
		{
			m_plate->Opacity(Clamp01((m_time - 2) / 0.5));
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
