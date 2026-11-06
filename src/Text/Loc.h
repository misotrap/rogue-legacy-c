// Translated texts (LocaleBuilder.getResourceString): assets/lang/<code>.txt
// made by tools/convert_loc.py. English is loaded too, as the fallback for
// texts a language lacks. LocIDs.h gives the IDs of classes, traits, spells.
#pragma once

#include "LocIDs.h"

#include <string>

namespace Text
{
	namespace Loc
	{
		// e.g. Load("assets", "fr"); returns false if the language is missing
		// (English is used then)
		bool Load(const char *assetsDir, const char *language);
		// The text for an ID ("LOC_ID_CLASS_NAME_1_MALE"); the ID itself if unknown
		const char *Get(const char *id);
		// Get() with {0}, {1}... replaced by the arguments (string.Format)
		std::string Format(const char *id, const std::string &a0 = "", const std::string &a1 = "", const std::string &a2 = "");

		// The languages of the original game: their codes (assets/lang/<code>.txt)
		// and the LOC_ID_OPTIONS_LANGUAGE_* name of each, English first
		constexpr int LANGUAGE_COUNT = 8;
		extern const char *const LANGUAGES[LANGUAGE_COUNT];
		extern const char *const LANGUAGE_NAMES[LANGUAGE_COUNT];
		// The code loaded last ("en" before any Load)
		const char *Language();
		// Game.ChangeBitmapLanguage: the variant of a picture with text in it
		// for the current language ("LineageTitleText_Sprite" -> "..._FR";
		// English keeps the plain name)
		std::string Sprite(const char *baseName);
		// The few texts of the port itself (the console's buttons...),
		// which the original has no translation for
		enum class Port { UpDown, LeftRight, TeleporterHelp, CategoryHelp, Critical };
		const char *Get(Port text);
	}
}
