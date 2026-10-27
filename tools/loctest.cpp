// Translation check (no window): ./loctest [language]
#include "../src/Text/Loc.h"

#include <cstdio>

int main(int argc, char **argv)
{
	const char *lang = argc > 1 ? argv[1] : "fr";
	if (!Text::Loc::Load("assets", lang)) printf("no %s texts, English used\n", lang);
	for (int c : { ClassType::Knight, ClassType::Wizard2, ClassType::Lich, ClassType::Dragon })
		printf("class %2d: %s / %s\n", c, Text::Loc::Get(LocIDs::ClassType_ToStringID(c, false)),
			Text::Loc::Get(LocIDs::ClassType_ToStringID(c, true)));
	printf("trait: %s - %s\n", Text::Loc::Get(LocIDs::TraitType_ToStringID(TraitType::Vertigo)),
		Text::Loc::Get(LocIDs::TraitType_DescriptionID(TraitType::Vertigo, false)));
	printf("spell: %s - %s\n", Text::Loc::Get(LocIDs::SpellType_ToStringID(SpellType::Axe)),
		Text::Loc::Get(LocIDs::SpellType_DescriptionID(SpellType::Axe)));
	return 0;
}
