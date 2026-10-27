#include "SaveGame.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

namespace Save
{
	std::vector<std::string> GameState::FamilyTreeNames() const
	{
		std::vector<std::string> names;
		for (const FamilyTreeNode &n : familyTree) names.push_back(n.name);
		return names;
	}

	namespace
	{
		const char *const MAGIC = "RogueLegacyC-save";

		// Names never contain '|' (it separates fields) or line breaks
		std::string Clean(const std::string &s)
		{
			std::string out;
			for (char c : s) out += (c == '|' || c == '\n' || c == '\r') ? ' ' : c;
			return out;
		}

		void WriteValue(FILE *f, const char *key, int v) { fprintf(f, "%s %d\n", key, v); }
		void WriteValue(FILE *f, const char *key, float v) { fprintf(f, "%s %.6g\n", key, v); }
		void WriteValue(FILE *f, const char *key, bool v) { fprintf(f, "%s %d\n", key, v ? 1 : 0); }

		void ReadValue(const char *text, int &v) { v = atoi(text); }
		void ReadValue(const char *text, float &v) { v = (float) atof(text); }
		void ReadValue(const char *text, bool &v) { v = atoi(text) != 0; }

		void WriteHeir(FILE *f, const char *key, const Lineage::Heir &h)
		{
			fprintf(f, "%s %s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d\n", key, Clean(h.name).c_str(), Clean(h.romanNumeral).c_str(),
				h.isFemale ? 1 : 0, h.classType, h.spell, h.trait1, h.trait2, h.age, h.childAge, h.headPiece, h.chestPiece, h.shoulderPiece);
		}

		// Splits "a|b|c" into fields (in place)
		int SplitBars(char *s, char **fields, int max)
		{
			int n = 0;
			char *p = s;
			while (n < max)
			{
				fields[n++] = p;
				p = strchr(p, '|');
				if (p == nullptr) break;
				*p++ = '\0';
			}
			return n;
		}

		bool ReadHeir(char *text, Lineage::Heir &h)
		{
			char *f[12];
			if (SplitBars(text, f, 12) < 12) return false;
			h.name = f[0];
			h.romanNumeral = f[1];
			h.isFemale = atoi(f[2]) != 0;
			h.classType = (uint8_t) atoi(f[3]);
			h.spell = (uint8_t) atoi(f[4]);
			h.trait1 = (uint8_t) atoi(f[5]);
			h.trait2 = (uint8_t) atoi(f[6]);
			h.age = (uint8_t) atoi(f[7]);
			h.childAge = (uint8_t) atoi(f[8]);
			h.headPiece = (uint8_t) atoi(f[9]);
			h.chestPiece = (uint8_t) atoi(f[10]);
			h.shoulderPiece = (uint8_t) atoi(f[11]);
			return true;
		}

		std::string Flags(const std::vector<uint8_t> &v)
		{
			std::string s;
			for (uint8_t b : v) s += b ? '1' : '0';
			return s.empty() ? "-" : s;
		}

		std::vector<uint8_t> ParseFlags(const char *s)
		{
			std::vector<uint8_t> v;
			if (strcmp(s, "-") == 0) return v;
			for (; *s == '0' || *s == '1'; s++) v.push_back(*s == '1');
			return v;
		}

		bool ReadFile(GameState &st, const char *path)
		{
			FILE *f = fopen(path, "r");
			if (f == nullptr) return false;
			char line[4096];
			if (fgets(line, sizeof(line), f) == nullptr || strncmp(line, MAGIC, strlen(MAGIC)) != 0)
			{
				fclose(f);
				return false;
			}
			GameState fresh;
			st = fresh;
			bool complete = false;
			while (fgets(line, sizeof(line), f))
			{
				size_t len = strlen(line);
				while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
				char *space = strchr(line, ' ');
				const char *key = line;
				char *value = (char *) "";
				if (space != nullptr)
				{
					*space = '\0';
					value = space + 1;
				}
				if (strcmp(key, "end") == 0) { complete = true; break; }
#define SAVE_READ(type, name, def) else if (strcmp(key, #name) == 0) ReadValue(value, st.name);
				if (false) {}
				SAVE_PLAYER_FIELDS(SAVE_READ)
#undef SAVE_READ
				else if (strcmp(key, "hero") == 0) ReadHeir(value, st.hero);
				else if (strcmp(key, "branch") == 0)
				{
					Lineage::Heir h;
					if (ReadHeir(value, h)) st.currentBranches.push_back(h);
				}
				else if (strcmp(key, "family") == 0)
				{
					char *v[13];
					if (SplitBars(value, v, 13) == 13)
					{
						FamilyTreeNode n;
						n.name = v[0];
						n.romanNumeral = v[1];
						n.age = (uint8_t) atoi(v[2]);
						n.childAge = (uint8_t) atoi(v[3]);
						n.classType = (uint8_t) atoi(v[4]);
						n.headPiece = (uint8_t) atoi(v[5]);
						n.chestPiece = (uint8_t) atoi(v[6]);
						n.shoulderPiece = (uint8_t) atoi(v[7]);
						n.numEnemiesBeaten = atoi(v[8]);
						n.beatenABoss = atoi(v[9]) != 0;
						n.isFemale = atoi(v[10]) != 0;
						n.trait1 = (uint8_t) atoi(v[11]);
						n.trait2 = (uint8_t) atoi(v[12]);
						st.familyTree.push_back(n);
					}
				}
				else if (strcmp(key, "kills") == 0)
				{
					int t, d, c;
					if (sscanf(value, "%d %d %d", &t, &d, &c) == 3 && t >= 0 && t < ENEMY_TYPES && d >= 0 && d < ENEMY_DIFFICULTIES)
						st.enemiesKilled[t][d] = c;
				}
				else if (strcmp(key, "castle") == 0)
				{
					st.castleExists = true;
					st.castleSeed = (uint32_t) strtoul(value, nullptr, 10);
				}
				else if (strcmp(key, "visited") == 0) st.roomsVisited = ParseFlags(value);
				else if (strcmp(key, "chests") == 0) st.chestsOpened = ParseFlags(value);
				else if (strcmp(key, "skill") == 0)
				{
					// skill <SkillType value> <level> <visible>
					int type, level, visible;
					if (sscanf(value, "%d %d %d", &type, &level, &visible) == 3)
						for (int i = 0; i < Skills::SKILL_COUNT; i++)
							if ((int) Skills::SKILLS[i].type == type)
							{
								st.skills.levels[i] = (uint8_t) level;
								st.skills.visible[i] = (uint8_t) (visible != 0);
							}
				}
				else if (strcmp(key, "playerLevel") == 0) st.skills.playerLevel = atoi(value);
				else if (strcmp(key, "blueprint") == 0 || strcmp(key, "rune") == 0)
				{
					int c, i, s;
					bool isRune = key[0] == 'r';
					int maxI = isRune ? Equipment::ABILITIES : Equipment::BASE_TYPES;
					if (sscanf(value, "%d %d %d", &c, &i, &s) == 3 && c >= 0 && c < Equipment::CATEGORIES && i >= 0 && i < maxI)
						(isRune ? st.equipment.rune[c][i] : st.equipment.blueprint[c][i]) = (uint8_t) s;
				}
				else if (strcmp(key, "equipped") == 0 || strcmp(key, "equippedRune") == 0)
				{
					int c, i;
					if (sscanf(value, "%d %d", &c, &i) == 2 && c >= 0 && c < Equipment::CATEGORIES)
						(strcmp(key, "equipped") == 0 ? st.equipment.equipped[c] : st.equipment.equippedRune[c]) = (int8_t) i;
				}
				else if (strcmp(key, "runesPurchased") == 0) st.equipment.totalRunesPurchased = atoi(value);
				// Unknown keys (from newer versions) are ignored
			}
			fclose(f);
			if (!complete) st = fresh; // truncated file: do not half-load it
			return complete;
		}
	}

	bool Write(const GameState &st, const char *path)
	{
		std::string tmp = std::string(path) + ".tmp", bak = std::string(path) + ".bak";
		FILE *f = fopen(tmp.c_str(), "w");
		if (f == nullptr) return false;
		fprintf(f, "%s %d\n", MAGIC, SAVE_VERSION);
#define SAVE_WRITE(type, name, def) WriteValue(f, #name, st.name);
		SAVE_PLAYER_FIELDS(SAVE_WRITE)
#undef SAVE_WRITE
		WriteHeir(f, "hero", st.hero);
		for (const Lineage::Heir &h : st.currentBranches) WriteHeir(f, "branch", h);
		for (const FamilyTreeNode &n : st.familyTree)
			fprintf(f, "family %s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d\n", Clean(n.name).c_str(), Clean(n.romanNumeral).c_str(),
				n.age, n.childAge, n.classType, n.headPiece, n.chestPiece, n.shoulderPiece, n.numEnemiesBeaten,
				n.beatenABoss ? 1 : 0, n.isFemale ? 1 : 0, n.trait1, n.trait2);
		for (int t = 0; t < ENEMY_TYPES; t++)
			for (int d = 0; d < ENEMY_DIFFICULTIES; d++)
				if (st.enemiesKilled[t][d] != 0) fprintf(f, "kills %d %d %d\n", t, d, st.enemiesKilled[t][d]);
		if (st.castleExists)
		{
			fprintf(f, "castle %u\n", st.castleSeed);
			fprintf(f, "visited %s\n", Flags(st.roomsVisited).c_str());
			fprintf(f, "chests %s\n", Flags(st.chestsOpened).c_str());
		}
		for (int i = 0; i < Skills::SKILL_COUNT; i++)
			fprintf(f, "skill %d %d %d\n", (int) Skills::SKILLS[i].type, st.skills.levels[i], st.skills.visible[i]);
		fprintf(f, "playerLevel %d\n", st.skills.playerLevel);
		for (int c = 0; c < Equipment::CATEGORIES; c++)
		{
			for (int i = 0; i < Equipment::BASE_TYPES; i++)
				if (st.equipment.blueprint[c][i] != 0) fprintf(f, "blueprint %d %d %d\n", c, i, st.equipment.blueprint[c][i]);
			for (int i = 0; i < Equipment::ABILITIES; i++)
				if (st.equipment.rune[c][i] != 0) fprintf(f, "rune %d %d %d\n", c, i, st.equipment.rune[c][i]);
			fprintf(f, "equipped %d %d\n", c, st.equipment.equipped[c]);
			fprintf(f, "equippedRune %d %d\n", c, st.equipment.equippedRune[c]);
		}
		fprintf(f, "runesPurchased %d\n", st.equipment.totalRunesPurchased);
		fprintf(f, "end\n");
		bool ok = fflush(f) == 0;
		ok = ok && fsync(fileno(f)) == 0;
		ok = (fclose(f) == 0) && ok;
		if (!ok)
		{
			remove(tmp.c_str());
			return false;
		}
		// Keep the previous save, then swap the new one in
		remove(bak.c_str());
		rename(path, bak.c_str());
		if (rename(tmp.c_str(), path) != 0) return false;
		sync();
		return true;
	}

	bool Read(GameState &st, const char *path)
	{
		if (ReadFile(st, path)) return true;
		std::string bak = std::string(path) + ".bak";
		if (ReadFile(st, bak.c_str())) return true;
		st = GameState();
		return false;
	}
}
