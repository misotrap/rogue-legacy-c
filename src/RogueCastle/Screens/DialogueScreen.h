// The dialogue box (DialogueScreen.cs) over the game: the speaker in gold,
// the text wrapped in the original DialogBox art, the blinking continue
// icon; A or B turns the page. A dialogue may end on a choice
// (SetDialogueChoice "ConfirmTest1": Confirmer / Annuler) picked with
// Up/Down. The pages come from Dialogue/DialogueData.h (DialogueManager.cs);
// a heroine gets the "_F" texts when they exist. The art is in the small
// uiExtraSpritesheet (tools/make_subsheet.py).
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"
#include "../Dialogue/DialogueData.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	class DialogueScreen
	{
	public:
		enum class Result { Open, Closed, Confirmed, Cancelled };

		~DialogueScreen();
		bool LoadContent(const char *assetsDir);
		// name: a DialogueManager key ("Meet Architect"); choice: a choice
		// dialogue shown after the last page ("ConfirmTest1"), or nullptr.
		// arg0/arg1 replace {0}/{1} in the pages (SetDialogue's params,
		// the donation box's gold cost).
		bool Open(const char *name, bool isFemale, const char *choice = nullptr,
			const char *arg0 = nullptr, const char *arg1 = nullptr);
		bool IsOpen() const { return m_entry != nullptr; }
		// The Dyslexia trait: the letters inside the words are shuffled
		// (TextObj.RandomizeSentence), once per page
		bool Dyslexia = false;
		Result Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // the 1320x720 screen at zoom 0.5
		DS2DEngine::SpriteObj *m_box = nullptr, *m_continue = nullptr, *m_plate = nullptr, *m_highlight = nullptr;
		const Dialogue::Entry *m_entry = nullptr, *m_choice = nullptr;
		bool m_female = false, m_choosing = false;
		int m_page = 0, m_selected = 0;
		int m_scrambledPage = -1;
		std::string m_scrambled;
		std::string m_arg0, m_arg1;   /* {0} and {1} in the pages */
		double m_time = 0;

		const char *Text(const char *id) const;
	};
}
