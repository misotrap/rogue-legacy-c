#include "DialogueScreen.h"

#include <string>

#include <cstdlib>

#include <vector>

#include <cctype>

#include "../../Audio/Audio.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstring>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const uint32_t SPEAKER = 0xFFECC584u;   // Color(236, 197, 132)
		const float BOX_X = 1320 / 2, BOX_Y = 100;   // world, zoom 0.5

		// world (1320x720 screen) -> 640x480 frame
		float SX(float x) { return (x - 660) * 0.5f + SCREEN_W / 2; }
		float SY(float y) { return (y - 360) * 0.5f + SCREEN_H / 2; }

		SpriteObj *Make(const char *name)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			return s;
		}

		const Dialogue::Entry *Find(const char *name)
		{
			if (name == nullptr) return nullptr;
			for (int i = 0; i < Dialogue::ENTRY_COUNT; i++)
				if (strcmp(Dialogue::ENTRIES[i].name, name) == 0) return &Dialogue::ENTRIES[i];
			return nullptr;
		}
	}

	DialogueScreen::~DialogueScreen()
	{
		delete m_box; delete m_continue; delete m_plate; delete m_highlight;
	}

	bool DialogueScreen::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("DialogBox_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "uiExtraSpritesheet") && ok;
		m_box = Make("DialogBox_Sprite");
		m_box->Position(Vector2(BOX_X, BOX_Y));
		m_continue = Make("ContinueTextIcon_Sprite");
		m_continue->Scale(Vector2(2, 2));
		m_plate = Make("GameOverStatPlate_Sprite");
		m_plate->Position(Vector2(1320 / 2, 720 / 2));
		m_highlight = Make("DialogueChoiceHighlight_Sprite");
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		return ok;
	}

	// LocaleBuilder.getString: the "_F" text for a heroine when there is one
	const char *DialogueScreen::Text(const char *id) const
	{
		if (m_female)
		{
			std::string f = std::string(id) + "_F";
			const char *t = Text::Loc::Get(f.c_str());
			if (strcmp(t, f.c_str()) != 0) return t;
		}
		const char *t = Text::Loc::Get(id);
		// an empty text in the original (no speaker name) is not converted
		return strcmp(t, id) == 0 ? "" : t;
	}

	// TextObj.RandomizeSentence(false): in each word of more than three
	// letters (numbers left alone), the letters between the first and the
	// last are shuffled. Letters are UTF-8 code points.
	static std::string RandomizeSentence(const std::string &text)
	{
		std::string out;
		size_t i = 0;
		while (i < text.size())
		{
			auto isLetter = [&](size_t at) { unsigned char c = text[at]; return c >= 0x80 || isalnum(c) || c == '\''; };
			if (!isLetter(i)) { out += text[i++]; continue; }
			std::vector<std::string> letters;
			bool number = true;
			while (i < text.size() && isLetter(i))
			{
				size_t len = 1;
				unsigned char c = text[i];
				if (c >= 0xF0) len = 4; else if (c >= 0xE0) len = 3; else if (c >= 0xC0) len = 2;
				if (!isdigit(c)) number = false;
				letters.push_back(text.substr(i, len));
				i += len;
			}
			if (letters.size() > 3 && !number)
				for (size_t k = letters.size() - 2; k > 1; k--)
					std::swap(letters[k], letters[1 + rand() % k]);
			for (const std::string &l : letters) out += l;
		}
		return out;
	}

	bool DialogueScreen::Open(const char *name, bool isFemale, const char *choice)
	{
		m_scrambledPage = -1;
		m_entry = Find(name);
		m_choice = Find(choice);
		m_female = isFemale;
		m_page = 0;
		m_selected = 0;
		m_choosing = false;
		m_time = 0;
		return m_entry != nullptr;
	}

	DialogueScreen::Result DialogueScreen::Update(uint32_t pressed, double dt)
	{
		if (m_entry == nullptr) return Result::Closed;
		m_time += dt;
		if (m_choosing)
		{
			if (pressed & (BTN_UP | BTN_DOWN))
			{
				m_selected = 1 - m_selected;
				Audio::PlaySound("frame_swap");
			}
			if (pressed & (BTN_A | BTN_B))
			{
				m_entry = nullptr;
				Audio::PlaySound(m_selected == 0 ? "ShopMenuConfirm" : "ShopMenuCancel");
				return m_selected == 0 ? Result::Confirmed : Result::Cancelled;
			}
			return Result::Open;
		}
		if (pressed & (BTN_A | BTN_B))
		{
			if (++m_page >= m_entry->count)
			{
				if (m_choice != nullptr)
				{
					m_choosing = true;
					m_page = m_entry->count - 1;
				}
				else
				{
					m_entry = nullptr;
					return Result::Closed;
				}
			}
		}
		return Result::Open;
	}

	void DialogueScreen::Draw(uint32_t *frame)
	{
		if (m_entry == nullptr) return;
		gfx_begin(frame);
		const Dialogue::Page &page = m_entry->pages[m_page];
		m_box->Draw(&m_camera);
		const float w = m_box->Width(), h = m_box->Height();
		// speaker at (-W/2.2, -H/1.6), text at (-W/2.15, -H/3.5) from the box centre
		m_font.DrawOutlined(frame, Text(page.speaker), SX(BOX_X - w / 2.2f), SY(BOX_Y - h / 1.6f), 0.34f, SPEAKER, BLACK, 1);
		const char *pageText = Text(page.text);
		if (Dyslexia)
		{
			if (m_scrambledPage != m_page) { m_scrambled = RandomizeSentence(pageText); m_scrambledPage = m_page; }
			pageText = m_scrambled.c_str();
		}
		std::string text = m_font.Wrap(pageText, 0.28f, w * 0.5f * 0.9f);
		m_font.DrawOutlined(frame, text.c_str(), SX(BOX_X - w / 2.15f), SY(BOX_Y - h / 3.5f), 0.28f, WHITE, BLACK, 1);
		if (!m_choosing && ((int) (m_time * 3)) % 2 == 0)
		{
			m_continue->Position(Vector2(BOX_X + w / 2 - 60, BOX_Y + h / 2 - 40));
			m_continue->Draw(&m_camera);
		}

		if (m_choosing && m_choice != nullptr)
		{
			// The choice plate in the middle: "Confirmer" over "Annuler"
			m_plate->Draw(&m_camera);
			const float y0 = 720 / 2 - 30, y1 = 720 / 2 + 20;
			m_highlight->Position(Vector2(1320 / 2, (m_selected == 0 ? y0 : y1) + 20));
			m_highlight->Draw(&m_camera);
			m_font.DrawOutlined(frame, Text(m_choice->pages[0].speaker), SX(1320 / 2), SY(y0), 0.3f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, Text(m_choice->pages[0].text), SX(1320 / 2), SY(y1), 0.3f, WHITE, BLACK, 1, Text::Align::Centre);
		}
	}
}
