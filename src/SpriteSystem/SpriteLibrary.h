/* Port of SpriteSystem/SpriteLibrary.cs: the registry of every sprite,
 * animation and character. Loads the .tex/.spr files produced by
 * tools/convert_assets.py.
 *
 * C# is a static class -> all methods are static here. Missing names return
 * null instead of throwing (the port builds with -fno-exceptions).
 */
#ifndef SPRITE_SYSTEM_SPRITE_LIBRARY_H
#define SPRITE_SYSTEM_SPRITE_LIBRARY_H

#include "../gfx.h"
#include "SpritesheetObj.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace SpriteSystem
{

class SpriteLibrary
{
public:
	/* Loads assets/<sheet>.tex + .spr; returns false on failure. Can be
	 * called once per sheet; keep only the sheets the scene needs. */
	static bool LoadSheet(const char *dir, const char *sheet);
	static void FreeSheet(const char *sheet);

	static bool ContainsSprite(const std::string &name);
	static bool ContainsCharacter(const std::string &name);
	static const Texture *GetSprite(const std::string &name);
	static const std::string &GetSpritesheetName(const std::string &name);
	static const std::vector<ImageData> *GetImageDataList(const std::string &name);
	static const std::vector<FrameData> *GetFrameDataList(const std::string &name);
	/* CharacterData list of a container name (empty when unknown). */
	static const std::vector<CharacterData> &GetCharData(const std::string &name);
	static const std::string &GetSpriteName(int index);   /* index -> name */

	/* The .spr parser assigns each sprite an integer handle, used by
	 * SpriteObj (the C# code passes names around; handles are the port's
	 * equivalent, resolved once at load). */
	static int GetSpriteIndex(const std::string &name);
	static const std::string &IndexToName(int index);
	static int NumSprites();

private:
	static SpritesheetObj *GetOrAdd(const std::string &name, const std::string &sheet);

	static std::unordered_map<std::string, SpritesheetObj *> m_spriteDict;
	static std::unordered_map<std::string, std::vector<CharacterData> *> m_charDataDict;
	static std::unordered_map<std::string, Texture *> m_textureDict;      /* sheet -> tex */
	static std::unordered_map<std::string, std::string> m_sheetOfSprite;  /* sprite -> sheet */
	static std::unordered_map<std::string, int> m_indexDict;             /* sprite -> index */
	static std::vector<std::string> m_names;                               /* index -> name */
	static std::vector<CharacterData> m_emptyCharData;
};

} // namespace SpriteSystem

#endif
