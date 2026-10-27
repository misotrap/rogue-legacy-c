/* Parser for the .spr files (see tools/convert_assets.py for the format).
 * Semantics ported from SpriteLibrary.cs: 'A' records append to an ordered
 * frame list (the XML 'frame' attribute is a label), 'R' hitboxes attach to
 * the last frame added, containers become CharacterData lists.
 */
#include "SpriteLibrary.h"
#include <cstdio>

namespace SpriteSystem
{

std::unordered_map<std::string, SpritesheetObj *> SpriteLibrary::m_spriteDict;
std::unordered_map<std::string, std::vector<CharacterData> *> SpriteLibrary::m_charDataDict;
std::unordered_map<std::string, Texture *> SpriteLibrary::m_textureDict;
std::unordered_map<std::string, std::string> SpriteLibrary::m_sheetOfSprite;
std::unordered_map<std::string, int> SpriteLibrary::m_indexDict;
std::vector<std::string> SpriteLibrary::m_names;
std::vector<CharacterData> SpriteLibrary::m_emptyCharData;

SpritesheetObj *SpriteLibrary::GetOrAdd(const std::string &name, const std::string &sheet)
{
	auto it = m_spriteDict.find(name);
	if (it != m_spriteDict.end())
		return it->second;
	SpritesheetObj *obj = new SpritesheetObj(name, sheet);
	m_spriteDict[name] = obj;
	m_sheetOfSprite[name] = sheet;
	m_indexDict[name] = (int) m_names.size();
	m_names.push_back(name);
	return obj;
}

bool SpriteLibrary::LoadSheet(const char *dir, const char *sheet)
{
	char path[512];
	char line[1024];
	Texture *tex;
	FILE *f;
	std::string sheetName = sheet;
	std::string lastFrameName;
	SpritesheetObj *lastFrameSheet = nullptr;

	if (m_textureDict.count(sheetName) != 0)
		return true; /* already loaded */

	tex = new Texture();
	snprintf(path, sizeof(path), "%s/%s.tex", dir, sheet);
	if (!gfx_load_texture(tex, path))
	{
		fprintf(stderr, "cannot load %s\n", path);
		delete tex;
		return false;
	}
	m_textureDict[sheetName] = tex;

	snprintf(path, sizeof(path), "%s/%s.spr", dir, sheet);
	f = fopen(path, "r");
	if (f == NULL)
	{
		fprintf(stderr, "cannot load %s\n", path);
		return false;
	}
	while (fgets(line, sizeof(line), f))
	{
		char name[128], other[128];
		int a, b, c, d, e, g, h;
		float fx, fy, fr;
		if (line[0] == 'I' && sscanf(line, "I\t%127s\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
		                            name, &a, &b, &c, &d, &e, &g, &h) == 8)
		{
			SpritesheetObj *sheetObj = GetOrAdd(name, sheetName);
			sheetObj->AddImageData(a, Vector2((float) b, (float) c),
			                       Vector2((float) g, (float) h), d, e);
			sheetObj->SortImageData();
		}
		else if (line[0] == 'A' && sscanf(line, "A\t%127s\t%d\t%d\t%d", name, &a, &b, &c) == 4)
		{
			SpritesheetObj *sheetObj = GetOrAdd(name, sheetName);
			/* The label (6th field, may be empty) is used by the C#
			 * PlayAnimation("Start", "End") logic. */
			std::string label;
			int tabs = 0;
			const char *t = line;
			while (*t != '\0' && tabs < 5)
			{
				if (*t == '\t')
					tabs++;
				t++;
			}
			if (tabs == 5)
			{
				label = t;
				while (!label.empty() && (label.back() == '\n' || label.back() == '\r'))
					label.pop_back();
			}
			sheetObj->AddFrameData(a, b, c, label);
			lastFrameName = name;
			lastFrameSheet = sheetObj;
		}
		else if (line[0] == 'R' && sscanf(line, "R\t%127s\t%d\t%d\t%d\t%d\t%d\t%f\t%d",
		                                 name, &a, &b, &c, &d, &e, &fr, &g) == 8)
		{
			/* AddHitboxToLastFrameData: attach to the previous A record. */
			if (lastFrameSheet != nullptr && lastFrameName == name)
				lastFrameSheet->AddHitboxToLastFrameData(b, c, d, e, fr, g);
		}
		else if (line[0] == 'C' && sscanf(line, "C\t%127s\t%127s\t%f\t%f", name, other, &fx, &fy) == 4)
		{
			std::vector<CharacterData> *list = m_charDataDict.count(name) ? m_charDataDict[name] : nullptr;
			if (list == nullptr)
				list = m_charDataDict[name] = new std::vector<CharacterData>();
			CharacterData cd;
			cd.Child = other;
			cd.ChildX = (int) fx;
			cd.ChildY = (int) fy;
			list->push_back(cd);
		}
	}
	fclose(f);
	return true;
}

void SpriteLibrary::FreeSheet(const char *sheet)
{
	std::string sheetName = sheet;
	Texture *tex = m_textureDict.count(sheetName) ? m_textureDict[sheetName] : nullptr;
	if (tex == nullptr)
		return;
	gfx_free_texture(tex);
	delete tex;
	m_textureDict.erase(sheetName);
	/* Sprites of the sheet keep their parsed data; only the pixel memory
	 * matters (tens of MB per sheet). */
}

bool SpriteLibrary::ContainsSprite(const std::string &name)
{
	return m_spriteDict.count(name) != 0;
}

bool SpriteLibrary::ContainsCharacter(const std::string &name)
{
	return m_charDataDict.count(name) != 0;
}

const Texture *SpriteLibrary::GetSprite(const std::string &name)
{
	auto sheetIt = m_sheetOfSprite.find(name);
	if (sheetIt == m_sheetOfSprite.end())
		return nullptr;
	auto texIt = m_textureDict.find(sheetIt->second);
	return texIt == m_textureDict.end() ? nullptr : texIt->second;
}

const std::string &SpriteLibrary::GetSpritesheetName(const std::string &name)
{
	static std::string empty;
	auto it = m_sheetOfSprite.find(name);
	return it == m_sheetOfSprite.end() ? empty : it->second;
}

const std::vector<ImageData> *SpriteLibrary::GetImageDataList(const std::string &name)
{
	auto it = m_spriteDict.find(name);
	return it == m_spriteDict.end() ? nullptr : &it->second->GetImageDataList();
}

const std::vector<FrameData> *SpriteLibrary::GetFrameDataList(const std::string &name)
{
	auto it = m_spriteDict.find(name);
	return it == m_spriteDict.end() ? nullptr : &it->second->GetFrameDataList();
}

const std::vector<CharacterData> &SpriteLibrary::GetCharData(const std::string &name)
{
	auto it = m_charDataDict.find(name);
	return it == m_charDataDict.end() ? m_emptyCharData : *it->second;
}

int SpriteLibrary::GetSpriteIndex(const std::string &name)
{
	auto it = m_indexDict.find(name);
	return it == m_indexDict.end() ? -1 : it->second;
}

const std::string &SpriteLibrary::IndexToName(int index)
{
	static std::string empty;
	return (index >= 0 && index < (int) m_names.size()) ? m_names[index] : empty;
}

int SpriteLibrary::NumSprites()
{
	return (int) m_names.size();
}

} // namespace SpriteSystem
