#include "Loc.h"

#include <cstdio>
#include <cstring>
#include <unordered_map>

namespace Text
{
	namespace Loc
	{
		namespace
		{
			std::unordered_map<std::string, std::string> texts, fallback;

			bool LoadFile(const char *path, std::unordered_map<std::string, std::string> &into)
			{
				FILE *f = fopen(path, "r");
				if (f == nullptr) return false;
				into.clear();
				std::string line;
				char buf[2048];
				while (fgets(buf, sizeof(buf), f))
				{
					line += buf;
					if (line.empty() || line.back() != '\n') continue; // very long line: keep reading
					line.pop_back();
					size_t tab = line.find('\t');
					if (tab != std::string::npos)
					{
						std::string value;
						for (size_t i = tab + 1; i < line.size(); i++)
						{
							if (line[i] == '\\' && i + 1 < line.size())
							{
								char c = line[++i];
								value += c == 'n' ? '\n' : c;
							}
							else
								value += line[i];
						}
						// Some texts carry their line break doubly escaped
						// ("\\n"): it shows as "\n" once unescaped
						for (size_t at = value.find("\\n"); at != std::string::npos; at = value.find("\\n", at))
							value.replace(at, 2, "\n");
						into[line.substr(0, tab)] = value;
					}
					line.clear();
				}
				fclose(f);
				return true;
			}
		}

		std::string current = "en";

		bool Load(const char *assetsDir, const char *language)
		{
			char path[512];
			texts.clear();
			current = language;
			snprintf(path, sizeof(path), "%s/lang/en.txt", assetsDir);
			LoadFile(path, fallback);
			snprintf(path, sizeof(path), "%s/lang/%s.txt", assetsDir, language);
			return LoadFile(path, texts);
		}

		const char *Get(const char *id)
		{
			auto it = texts.find(id);
			if (it != texts.end()) return it->second.c_str();
			it = fallback.find(id);
			if (it != fallback.end()) return it->second.c_str();
			return id;
		}

		std::string Format(const char *id, const std::string &a0, const std::string &a1, const std::string &a2)
		{
			std::string s = Get(id), out;
			const std::string *args[3] = { &a0, &a1, &a2 };
			for (size_t i = 0; i < s.size(); i++)
			{
				if (s[i] == '{' && i + 2 < s.size() && s[i + 2] == '}' && s[i + 1] >= '0' && s[i + 1] <= '2')
				{
					out += *args[s[i + 1] - '0'];
					i += 2;
				}
				else
					out += s[i];
			}
			return out;
		}
	
		const char *const LANGUAGES[LANGUAGE_COUNT] = { "en", "fr", "de", "es", "pl", "pt", "ru", "zh" };
		const char *const LANGUAGE_NAMES[LANGUAGE_COUNT] = {
			"LOC_ID_OPTIONS_LANGUAGE_ENGLISH", "LOC_ID_OPTIONS_LANGUAGE_FRENCH", "LOC_ID_OPTIONS_LANGUAGE_GERMAN",
			"LOC_ID_OPTIONS_LANGUAGE_SPANISH", "LOC_ID_OPTIONS_LANGUAGE_POLISH", "LOC_ID_OPTIONS_LANGUAGE_PORTUGUESE",
			"LOC_ID_OPTIONS_LANGUAGE_RUSSIAN", "LOC_ID_OPTIONS_LANGUAGE_CHINESE" };

		const char *Language() { return current.c_str(); }

		std::string Sprite(const char *baseName)
		{
			// The suffixes of the language sheet's pictures
			static const char *const SUFFIXES[LANGUAGE_COUNT] = { "", "_FR", "_DE", "_SP", "_PO", "_BR", "_RU", "_ZH" };
			for (int i = 0; i < LANGUAGE_COUNT; i++)
				if (current == LANGUAGES[i]) return std::string(baseName) + SUFFIXES[i];
			return baseName;
		}
	
		const char *Get(Port text)
		{
			// en, fr, de, es, pl, pt, ru, zh (the order of LANGUAGES)
			static const char *const TEXTS[5][LANGUAGE_COUNT] = {
				{ "Up/Down", "Haut/Bas", "Oben/Unten", "Arriba/Abajo", "Góra/Dół", "Cima/Baixo", "Вверх/Вниз", "上/下" },
				{ "Left/Right", "Gauche/Droite", "Links/Rechts", "Izquierda/Derecha", "Lewo/Prawo", "Esquerda/Direita", "Влево/Вправо", "左/右" },
				{ "Left/Right: choose, A: travel", "Gauche/Droite : choisir, A : voyager", "Links/Rechts: wählen, A: reisen",
				  "Izquierda/Derecha: elegir, A: viajar", "Lewo/Prawo: wybierz, A: podróżuj", "Esquerda/Direita: escolher, A: viajar",
				  "Влево/Вправо: выбор, A: перемещение", "左/右：选择，A：传送" },
				{ "L1/R1: category", "L1/R1 : catégorie", "L1/R1: Kategorie", "L1/R1: categoría", "L1/R1: kategoria",
				  "L1/R1: categoria", "L1/R1: категория", "L1/R1：类别" },
				{ "Critical!", "Critique !", "Kritisch!", "¡Crítico!", "Krytyk!", "Crítico!", "Крит!", "暴击！" },
			};
			int language = 0;
			for (int i = 0; i < LANGUAGE_COUNT; i++)
				if (current == LANGUAGES[i]) language = i;
			return TEXTS[(int) text][language];
		}
	}
}
