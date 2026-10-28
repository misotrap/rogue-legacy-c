// Sound (DS2DEngine SoundManager.cs, subset): plays the cues converted by
// tools/convert_audio.cpp (assets/audio/sfx/<Cue>.snd, assets/audio/music/
// <Cue>.snd: mono int16 at 22050 Hz). Sound effects are loaded on their
// first use and kept; the music track is loaded whole when it starts.
// Mixing runs in the platform's audio thread (plat_audio_open). Without a
// sound device or without the converted files, every call does nothing.
#pragma once

namespace Audio
{
	bool Init(const char *assetsDir);
	void Quit();

	// SoundManager.PlaySound(name): one of the names at random, like the
	// original's PlaySound("Attack1", "Attack2", "Attack3")
	void PlaySound(const char *cue);
	void PlaySound(const char *cue1, const char *cue2);
	void PlaySound(const char *cue1, const char *cue2, const char *cue3);
	// SoundManager.PlayMusic: changing to the same song keeps it playing
	void PlayMusic(const char *cue, bool loop = true);
	void StopMusic();

	void SetVolumes(float sfx, float music);   // 0..1 (GameConfig)
}
