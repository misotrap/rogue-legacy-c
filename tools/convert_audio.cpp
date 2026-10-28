// Converts the XACT audio of your own Steam copy (Content/Audio) into one
// small file per cue for the port. Runs once, on the computer:
//
//   c++ -O2 -std=c++17 -o convert_audio tools/convert_audio.cpp
//   ./convert_audio "<.../Content/Audio>" assets/audio
//
// SFXSoundBank.xsb + SFXWaveBank.xwb (16-bit PCM)  -> assets/audio/sfx/<Cue>.snd
// MusicSoundBank.xsb + MusicWaveBank.xwb (MS-ADPCM) -> assets/audio/music/<Cue>.snd
//
// .snd: "RLS1", uint32 rate (22050), uint32 frames, uint32 loop (1 = the
// cue loops forever), then frames * int16 mono samples (little endian).
// Mono 22050 Hz keeps the files small (the console has one speaker).
//
// Sound bank layout (FACT, content version 46): all of Rogue Legacy's cues
// are simple cues -> one sound; a simple sound names its wave directly, a
// complex sound has one track whose first event (PlayWave) names it.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace
{
	const int OUT_RATE = 22050;

	std::vector<uint8_t> ReadFile(const std::string &path)
	{
		std::vector<uint8_t> d;
		FILE *f = fopen(path.c_str(), "rb");
		if (f == nullptr) return d;
		fseek(f, 0, SEEK_END);
		d.resize((size_t) ftell(f));
		fseek(f, 0, SEEK_SET);
		if (fread(d.data(), 1, d.size(), f) != d.size()) d.clear();
		fclose(f);
		return d;
	}

	template <typename T> T Get(const std::vector<uint8_t> &d, size_t at)
	{
		T v;
		memcpy(&v, d.data() + at, sizeof(T));
		return v;
	}

	struct Cue { std::string name; int wave; bool loop; };

	// SoundBank header (after "SDBK"): see FACT_INTERNAL_ParseSoundBank
	std::vector<Cue> ParseSoundBank(const std::vector<uint8_t> &d)
	{
		std::vector<Cue> cues;
		if (d.size() < 0x60 || memcmp(d.data(), "SDBK", 4) != 0) return cues;
		uint16_t numSimple = Get<uint16_t>(d, 19), numTotal = Get<uint16_t>(d, 25);
		uint32_t simpleOff = Get<uint32_t>(d, 34), nameIdxOff = Get<uint32_t>(d, 66);
		if (numSimple != numTotal) fprintf(stderr, "warning: %d complex cues skipped\n", numTotal - numSimple);
		for (int i = 0; i < numSimple; i++)
		{
			uint32_t nameOff = Get<uint32_t>(d, nameIdxOff + i * 6);
			uint32_t so = Get<uint32_t>(d, simpleOff + i * 5 + 1);
			uint8_t flags = d[so];
			size_t p = so + 9;
			Cue c;
			c.name = (const char *) d.data() + nameOff;
			c.loop = false;
			if (flags & 1)
			{
				p += 1;   // number of tracks (always 1 here)
				if (flags & 0x0E) p += Get<uint16_t>(d, p);   // RPC table
				if (flags & 0x10) p += Get<uint16_t>(d, p);   // DSP presets
				uint32_t events = Get<uint32_t>(d, p + 1);
				// event: count, u32 type|time, u16, u8 0xff, u8 flags, u16 wave, u8 bank, u8 loops
				c.wave = Get<uint16_t>(d, events + 9);
				c.loop = d[events + 12] == 0xff;
			}
			else c.wave = Get<uint16_t>(d, p);
			cues.push_back(c);
		}
		return cues;
	}

	struct Wave { int tag, channels, rate, bits, blockAlign; uint32_t offset, length; };

	std::vector<Wave> ParseWaveBank(const std::vector<uint8_t> &d, uint32_t &dataOffset)
	{
		std::vector<Wave> waves;
		if (d.size() < 52 || memcmp(d.data(), "WBND", 4) != 0) return waves;
		uint32_t bankOff = Get<uint32_t>(d, 12), metaOff = Get<uint32_t>(d, 20);
		dataOffset = Get<uint32_t>(d, 44);
		uint32_t count = Get<uint32_t>(d, bankOff + 4), stride = Get<uint32_t>(d, bankOff + 72);
		for (uint32_t i = 0; i < count; i++)
		{
			size_t e = metaOff + i * stride;
			uint32_t fmt = Get<uint32_t>(d, e + 4);
			Wave w;
			w.tag = fmt & 3;
			w.channels = (fmt >> 2) & 7;
			w.rate = (fmt >> 5) & 0x3ffff;
			w.blockAlign = (fmt >> 23) & 0xff;
			w.bits = fmt >> 31;
			w.offset = Get<uint32_t>(d, e + 8);
			w.length = Get<uint32_t>(d, e + 12);
			waves.push_back(w);
		}
		return waves;
	}

	// MS-ADPCM (WAVE_FORMAT_ADPCM), interleaved channels
	void DecodeAdpcm(const uint8_t *src, size_t len, int channels, int blockAlign, std::vector<float> &out)
	{
		static const int adapt[16] = { 230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230 };
		static const int coef1[7] = { 256, 512, 0, 192, 240, 460, 392 };
		static const int coef2[7] = { 0, -256, 0, 64, 0, -208, -232 };
		for (size_t b = 0; b + blockAlign <= len; b += blockAlign)
		{
			const uint8_t *p = src + b;
			int pred[2], delta[2], s1[2], s2[2];
			for (int c = 0; c < channels; c++) pred[c] = p[c] > 6 ? 6 : p[c];
			p += channels;
			for (int c = 0; c < channels; c++, p += 2) delta[c] = (int16_t) (p[0] | p[1] << 8);
			for (int c = 0; c < channels; c++, p += 2) s1[c] = (int16_t) (p[0] | p[1] << 8);
			for (int c = 0; c < channels; c++, p += 2) s2[c] = (int16_t) (p[0] | p[1] << 8);
			float mix = 0;
			for (int c = 0; c < channels; c++) mix += s2[c];
			out.push_back(mix / channels);
			mix = 0;
			for (int c = 0; c < channels; c++) mix += s1[c];
			out.push_back(mix / channels);
			const uint8_t *end = src + b + blockAlign;
			int c = 0;
			float acc = 0;
			for (; p < end; p++)
			{
				for (int half = 0; half < 2; half++)
				{
					int nib = half == 0 ? p[0] >> 4 : p[0] & 15;
					int signedNib = nib >= 8 ? nib - 16 : nib;
					int predicted = (s1[c] * coef1[pred[c]] + s2[c] * coef2[pred[c]]) / 256 + signedNib * delta[c];
					if (predicted > 32767) predicted = 32767;
					if (predicted < -32768) predicted = -32768;
					s2[c] = s1[c];
					s1[c] = predicted;
					delta[c] = delta[c] * adapt[nib] / 256;
					if (delta[c] < 16) delta[c] = 16;
					acc += predicted;
					if (++c == channels)
					{
						out.push_back(acc / channels);
						acc = 0;
						c = 0;
					}
				}
			}
		}
	}

	// Mono float samples at 'rate' -> int16 at OUT_RATE (linear)
	std::vector<int16_t> Resample(const std::vector<float> &in, int rate)
	{
		std::vector<int16_t> out;
		if (in.empty()) return out;
		double step = (double) rate / OUT_RATE;
		size_t n = (size_t) (in.size() / step);
		out.reserve(n);
		for (size_t i = 0; i < n; i++)
		{
			double pos = i * step;
			size_t a = (size_t) pos;
			double t = pos - a;
			float v = a + 1 < in.size() ? (float) (in[a] * (1 - t) + in[a + 1] * t) : in[a];
			if (v > 32767) v = 32767;
			if (v < -32768) v = -32768;
			out.push_back((int16_t) lrintf(v));
		}
		return out;
	}

	bool WriteSnd(const std::string &path, const std::vector<int16_t> &pcm, bool loop)
	{
		FILE *f = fopen(path.c_str(), "wb");
		if (f == nullptr) return false;
		uint32_t head[3] = { (uint32_t) OUT_RATE, (uint32_t) pcm.size(), loop ? 1u : 0u };
		fwrite("RLS1", 1, 4, f);
		fwrite(head, 4, 3, f);
		fwrite(pcm.data(), 2, pcm.size(), f);
		fclose(f);
		return true;
	}

	int ConvertBank(const std::string &in, const char *xsb, const char *xwb, const std::string &outDir)
	{
		std::vector<uint8_t> sb = ReadFile(in + "/" + xsb), wb = ReadFile(in + "/" + xwb);
		uint32_t dataOffset = 0;
		std::vector<Cue> cues = ParseSoundBank(sb);
		std::vector<Wave> waves = ParseWaveBank(wb, dataOffset);
		if (cues.empty() || waves.empty())
		{
			fprintf(stderr, "cannot read %s / %s\n", xsb, xwb);
			return -1;
		}
		mkdir(outDir.c_str(), 0755);
		int n = 0;
		for (const Cue &c : cues)
		{
			if (c.wave < 0 || c.wave >= (int) waves.size()) continue;
			const Wave &w = waves[c.wave];
			const uint8_t *src = wb.data() + dataOffset + w.offset;
			if (dataOffset + w.offset + w.length > wb.size() || w.channels < 1) continue;
			std::vector<float> mono;
			if (w.tag == 0)   // PCM, 16 bit (8 bit unsigned when bits == 0)
			{
				int bytes = w.bits ? 2 : 1;
				size_t frames = w.length / (bytes * w.channels);
				mono.reserve(frames);
				for (size_t i = 0; i < frames; i++)
				{
					float acc = 0;
					for (int ch = 0; ch < w.channels; ch++)
					{
						const uint8_t *s = src + (i * w.channels + ch) * bytes;
						acc += bytes == 2 ? (float) (int16_t) (s[0] | s[1] << 8) : (float) ((s[0] - 128) << 8);
					}
					mono.push_back(acc / w.channels);
				}
			}
			else if (w.tag == 2)   // MS-ADPCM: block align = (value + 22) * channels
				DecodeAdpcm(src, w.length, w.channels, (w.blockAlign + 22) * w.channels, mono);
			else
			{
				fprintf(stderr, "skipped %s: format %d\n", c.name.c_str(), w.tag);
				continue;
			}
			if (WriteSnd(outDir + "/" + c.name + ".snd", Resample(mono, w.rate), c.loop)) n++;
		}
		return n;
	}
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s <Content/Audio> <assets/audio>\n", argv[0]);
		return 1;
	}
	std::string in = argv[1], out = argv[2];
	mkdir(out.c_str(), 0755);
	int sfx = ConvertBank(in, "SFXSoundBank.xsb", "SFXWaveBank.xwb", out + "/sfx");
	int music = ConvertBank(in, "MusicSoundBank.xsb", "MusicWaveBank.xwb", out + "/music");
	printf("sfx: %d cues, music: %d cues\n", sfx, music);
	return sfx > 0 && music > 0 ? 0 : 1;
}
