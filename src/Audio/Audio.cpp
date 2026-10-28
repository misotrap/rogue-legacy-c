#include "Audio.h"

#include "../platform.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace Audio
{
	namespace
	{
		const int RATE = 22050;
		const int VOICES = 12;

		struct Clip
		{
			std::vector<int16_t> pcm;
			bool loop = false;
		};

		struct Voice
		{
			const Clip *clip = nullptr;
			size_t pos = 0;
		};

		std::string g_dir;
		bool g_open = false;
		std::map<std::string, Clip *> g_sfx;   // loaded on first use, kept
		Clip *g_music = nullptr;                // owned, swapped under the lock
		std::string g_musicName;
		size_t g_musicPos = 0;
		Voice g_voices[VOICES];
		int g_nextVoice = 0;
		int g_sfxGain = 256, g_musicGain = 160;   // /256

		Clip *Load(const std::string &path)
		{
			FILE *f = fopen(path.c_str(), "rb");
			if (f == nullptr) return nullptr;
			char magic[4];
			uint32_t head[3];
			Clip *c = nullptr;
			if (fread(magic, 1, 4, f) == 4 && memcmp(magic, "RLS1", 4) == 0 && fread(head, 4, 3, f) == 3)
			{
				c = new Clip();
				c->loop = head[2] != 0;
				c->pcm.resize(head[1]);
				if (fread(c->pcm.data(), 2, c->pcm.size(), f) != c->pcm.size())
				{
					delete c;
					c = nullptr;
				}
			}
			fclose(f);
			return c;
		}

		// The audio thread: music + every voice, clamped
		void Mix(int16_t *out, int frames)
		{
			for (int i = 0; i < frames; i++)
			{
				int acc = 0;
				if (g_music != nullptr && !g_music->pcm.empty())
				{
					if (g_musicPos >= g_music->pcm.size() && g_music->loop) g_musicPos = 0;
					if (g_musicPos < g_music->pcm.size())
						acc += g_music->pcm[g_musicPos++] * g_musicGain >> 8;
				}
				for (Voice &v : g_voices)
				{
					if (v.clip == nullptr) continue;
					if (v.pos >= v.clip->pcm.size())
					{
						v.clip = nullptr;
						continue;
					}
					acc += v.clip->pcm[v.pos++] * g_sfxGain >> 8;
				}
				out[i] = (int16_t) (acc > 32767 ? 32767 : acc < -32768 ? -32768 : acc);
			}
		}
	}

	bool Init(const char *assetsDir)
	{
		g_dir = std::string(assetsDir) + "/audio";
		g_open = plat_audio_open(RATE, Mix) != 0;
		if (!g_open) fprintf(stderr, "audio: no sound device\n");
		return g_open;
	}

	void Quit()
	{
		if (g_open)
		{
			plat_audio_lock();
			for (Voice &v : g_voices) v.clip = nullptr;
			g_music = nullptr;
			plat_audio_unlock();
		}
		for (auto &it : g_sfx) delete it.second;
		g_sfx.clear();
	}

	void PlaySound(const char *cue)
	{
		if (!g_open || cue == nullptr) return;
		auto it = g_sfx.find(cue);
		Clip *clip;
		if (it == g_sfx.end())
		{
			clip = Load(g_dir + "/sfx/" + cue + ".snd");
			g_sfx[cue] = clip;   // nullptr too: a missing cue is not retried
		}
		else clip = it->second;
		if (clip == nullptr) return;
		plat_audio_lock();
		// a free voice, else the oldest one started
		int slot = -1;
		for (int i = 0; i < VOICES && slot < 0; i++)
			if (g_voices[i].clip == nullptr) slot = i;
		if (slot < 0) slot = g_nextVoice = (g_nextVoice + 1) % VOICES;
		g_voices[slot].clip = clip;
		g_voices[slot].pos = 0;
		plat_audio_unlock();
	}

	void PlaySound(const char *cue1, const char *cue2)
	{
		PlaySound(rand() % 2 == 0 ? cue1 : cue2);
	}

	void PlaySound(const char *cue1, const char *cue2, const char *cue3)
	{
		int r = rand() % 3;
		PlaySound(r == 0 ? cue1 : r == 1 ? cue2 : cue3);
	}

	void PlayMusic(const char *cue, bool loop)
	{
		if (!g_open || cue == nullptr || g_musicName == cue) return;
		Clip *clip = Load(g_dir + "/music/" + cue + ".snd");
		if (clip != nullptr) clip->loop = loop;
		plat_audio_lock();
		Clip *old = g_music;
		g_music = clip;
		g_musicPos = 0;
		plat_audio_unlock();
		delete old;
		g_musicName = cue;
	}

	void StopMusic()
	{
		if (!g_open) return;
		plat_audio_lock();
		Clip *old = g_music;
		g_music = nullptr;
		plat_audio_unlock();
		delete old;
		g_musicName.clear();
	}

	void SetVolumes(float sfx, float music)
	{
		plat_audio_lock();
		g_sfxGain = (int) (sfx * 256);
		g_musicGain = (int) (music * 256);
		plat_audio_unlock();
	}
}
