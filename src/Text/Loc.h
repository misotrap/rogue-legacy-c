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
	}
}
