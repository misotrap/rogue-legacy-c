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
						into[line.substr(0, tab)] = value;
					}
					line.clear();
				}
				fclose(f);
				return true;
			}
		}

		bool Load(const char *assetsDir, const char *language)
		{
			char path[512];
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
	}
}
