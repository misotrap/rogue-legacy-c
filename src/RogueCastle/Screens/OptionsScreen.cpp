#include "OptionsScreen.h"

#include "../../Audio/Audio.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using DS2DEngine::ObjContainer;
using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u, YELLOW = 0xFFFFFF00u;
		const char *const NAMES[] = { "LOC_ID_MUSICVOL_OPTIONS_1", "LOC_ID_SFXVOL_OPTIONS_1",
			"LOC_ID_OPTIONS_LANGUAGE_TITLE", "LOC_ID_BACK_TO_MENU_OPTIONS_1", "LOC_ID_EXIT_ROGUE_LEGACY_OPTIONS_1" };
		const float ITEM_X = 420, TEXT_OFFSET = 300;   // OptionsObj.X, m_optionsTextOffset

		float SX(float x) { return (x - 660) * 0.5f + SCREEN_W / 2; }
		float SY(float y) { return (y - 360) * 0.5f + SCREEN_H / 2; }

		SpriteObj *Make(const char *name)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			return s;
		}

		float Clamp(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
	}

	OptionsScreen::~OptionsScreen()
	{
		delete m_plate; delete m_title; delete m_bar; delete m_volumeBG; delete m_volume;
	}

	bool OptionsScreen::LoadContent(const char *assetsDir, const char *configPath)
	{
		m_configPath = configPath;
		m_assetsDir = assetsDir;
		for (int i = 0; i < Text::Loc::LANGUAGE_COUNT; i++)
			if (strcmp(Text::Loc::Language(), Text::Loc::LANGUAGES[i]) == 0) m_language = i;
		bool ok = m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("OptionsBar_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "uiExtraSpritesheet") && ok;
		m_plate = new ObjContainer("SkillUnlockPlate_Character");
		m_plate->ForceDraw(true);
		m_plate->Position(Vector2(1320 / 2.0f, 720 / 2.0f));
		// The translated titles are on the language sheet
		if (!SpriteSystem::SpriteLibrary::ContainsSprite(Text::Loc::Sprite("OptionsScreenTitle_Sprite").c_str()))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "languageSpritesheet") && ok;
		m_title = Make(Text::Loc::Sprite("OptionsScreenTitle_Sprite").c_str());   // Game.ChangeBitmapLanguage
		m_title->Position(Vector2(1320 / 2.0f, 720 / 2.0f - m_plate->Width() / 2 + 60));
		m_bar = Make("OptionsBar_Sprite");
		m_volumeBG = Make("OptionsScreenVolumeBG_Sprite");
		m_volume = Make("OptionsScreenVolumeBar_Sprite");
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);

		FILE *f = fopen(configPath, "r");
		if (f != nullptr)
		{
			char key[32], value[32];
			while (fscanf(f, "%31s %31s", key, value) == 2)
			{
				if (strcmp(key, "music") == 0) m_music = Clamp((float) atof(value));
				else if (strcmp(key, "sfx") == 0) m_sfx = Clamp((float) atof(value));
			}
			fclose(f);
		}
		Apply();
		return ok;
	}

	void OptionsScreen::Apply()
	{
		// SoundManager.GlobalMusicVolume: the music sits under the effects
		Audio::SetVolumes(m_sfx, m_music * 0.625f);
	}

	std::string OptionsScreen::ConfigLanguage(const char *configPath)
	{
		std::string language = "en";   // the default: English
		if (FILE *f = fopen(configPath, "r"))
		{
			char key[32], value[32];
			while (fscanf(f, "%31s %31s", key, value) == 2)
				if (strcmp(key, "language") == 0)
					for (int i = 0; i < Text::Loc::LANGUAGE_COUNT; i++)
						if (strcmp(value, Text::Loc::LANGUAGES[i]) == 0) language = value;
			fclose(f);
		}
		return language;
	}

	void OptionsScreen::SaveConfig() const
	{
		FILE *f = fopen(m_configPath.c_str(), "w");
		if (f == nullptr) return;
		fprintf(f, "music %.2f\nsfx %.2f\nlanguage %s\n", m_music, m_sfx, Text::Loc::LANGUAGES[m_language]);
		fclose(f);
	}

	void OptionsScreen::Open(bool inCastle)
	{
		m_inCastle = inCastle;
		m_selected = 0;
		Audio::PlaySound("Option_Menu_Select");
	}

	OptionsScreen::Result OptionsScreen::Update(uint32_t pressed, double)
	{
		if (pressed & (BTN_UP | BTN_DOWN))
		{
			int step = (pressed & BTN_UP) ? COUNT - 1 : 1;
			do m_selected = (m_selected + step) % COUNT; while (!Shown(m_selected));
			Audio::PlaySound("Option_Menu_Nav");
		}
		if ((pressed & (BTN_LEFT | BTN_RIGHT)) && (m_selected == MUSIC || m_selected == SFX))
		{
			float delta = (pressed & BTN_RIGHT) ? 0.05f : -0.05f;
			float &v = m_selected == MUSIC ? m_music : m_sfx;
			v = Clamp(v + delta);
			Apply();
			if (m_selected == SFX) Audio::PlaySound("frame_swap");
		}
		if ((pressed & (BTN_LEFT | BTN_RIGHT)) && m_selected == LANGUAGE)
		{
			// The texts change now; the title pictures and the Chinese font
			// when they are next loaded (a restart for some)
			int n = Text::Loc::LANGUAGE_COUNT;
			m_language = (m_language + ((pressed & BTN_RIGHT) ? 1 : n - 1)) % n;
			Text::Loc::Load(m_assetsDir.c_str(), Text::Loc::LANGUAGES[m_language]);
			SaveConfig();
			Audio::PlaySound("frame_swap");
		}
		if (pressed & BTN_A)
		{
			if (m_selected == BACK_TO_TITLE) { SaveConfig(); return Result::BackToTitle; }
			if (m_selected == QUIT) { SaveConfig(); return Result::Quit; }
		}
		if (pressed & (BTN_B | BTN_START))
		{
			SaveConfig();
			Audio::PlaySound("Options_Menu_Deselect");
			return Result::Closed;
		}
		return Result::Open;
	}

	void OptionsScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		// BackBufferOpacity: the screen behind darkened
		for (size_t i = 0; i < (size_t) SCREEN_W * SCREEN_H; i++)
		{
			uint32_t c = frame[i];
			frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * 90 >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * 90 >> 8) & 0xFF00u);
		}
		m_plate->Draw(&m_camera);
		m_title->Draw(&m_camera);

		int row = 0;
		for (int i = 0; i < COUNT; i++)
		{
			if (!Shown(i)) continue;
			float y = 160 + row * 30 + 50;   // OnEnter: 160 + counter * 30 (+ the plate's top margin)
			row++;
			if (i == m_selected)
			{
				m_bar->Position(Vector2(ITEM_X - 15, y));
				m_bar->Draw(&m_camera);
			}
			m_font.DrawOutlined(frame, Text::Loc::Get(NAMES[i]), SX(ITEM_X), SY(y + 4), 0.3f,
				i == m_selected ? YELLOW : WHITE, BLACK, 1);
			if (i == MUSIC || i == SFX)
			{
				m_volumeBG->Position(Vector2(ITEM_X + TEXT_OFFSET, y + 4));
				m_volumeBG->Draw(&m_camera);
				m_volume->Position(Vector2(ITEM_X + TEXT_OFFSET + 6, y + 4));
				m_volume->ScaleX(i == MUSIC ? m_music : m_sfx);
				if ((i == MUSIC ? m_music : m_sfx) > 0) m_volume->Draw(&m_camera);
			}
			if (i == LANGUAGE)
				m_font.DrawOutlined(frame, Text::Loc::Get(Text::Loc::LANGUAGE_NAMES[m_language]), SX(ITEM_X + TEXT_OFFSET), SY(y + 4), 0.3f,
					i == m_selected ? YELLOW : WHITE, BLACK, 1);
		}

		// The key hints, bottom right of the plate
		std::string pick = Text::Loc::Get("LOC_ID_OPTIONS_SCREEN_4_NEW"), leave = Text::Loc::Get("LOC_ID_OPTIONS_SCREEN_5_NEW");
		size_t at = pick.find("[Input:0]");
		if (at != std::string::npos) pick.replace(at, 9, "A");
		at = leave.find("[Input:2]");
		if (at != std::string::npos) leave.replace(at, 9, "B");
		m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_OPTIONS_SCREEN_3"), SX(1290), SY(570), 0.24f, WHITE, BLACK, 1, Text::Align::Right);
		m_font.DrawOutlined(frame, pick.c_str(), SX(1290), SY(610), 0.24f, WHITE, BLACK, 1, Text::Align::Right);
		m_font.DrawOutlined(frame, leave.c_str(), SX(1290), SY(650), 0.24f, WHITE, BLACK, 1, Text::Align::Right);
	}
}
