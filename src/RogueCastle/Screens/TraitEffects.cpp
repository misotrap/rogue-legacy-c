#include "TraitEffects.h"

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../platform.h"
#include "../Types/TraitType.h"

#include <cmath>
#include <cstring>

namespace RogueCastle
{
	namespace
	{
		// The castle view: world 1320x720 at zoom 0.5, rows 60-419
		const int TOP = 60, BOTTOM = 420;

		inline int R(uint32_t c) { return (c >> 16) & 0xFF; }
		inline int G(uint32_t c) { return (c >> 8) & 0xFF; }
		inline int B(uint32_t c) { return c & 0xFF; }
		inline uint32_t RGB(int r, int g, int b)
		{
			r = r < 0 ? 0 : r > 255 ? 255 : r;
			g = g < 0 ? 0 : g > 255 ? 255 : g;
			b = b < 0 ? 0 : b > 255 ? 255 : b;
			return 0xFF000000u | (uint32_t) r << 16 | (uint32_t) g << 8 | (uint32_t) b;
		}
		inline uint32_t Mix(uint32_t a, uint32_t b, int t)   // t: 0..256 towards b
		{
			return RGB(R(a) + ((R(b) - R(a)) * t >> 8), G(a) + ((G(b) - G(a)) * t >> 8), B(a) + ((B(b) - B(a)) * t >> 8));
		}
	}

	TraitEffects::~TraitEffects()
	{
		delete m_grain;
	}

	void TraitEffects::World(uint32_t *frame, const Lineage::Heir &hero, bool glasses, bool dungeon, float px, float py)
	{
		bool near = !glasses && hero.HasTrait(TraitType::NearSighted);
		bool far = !glasses && !near && hero.HasTrait(TraitType::FarSighted);
		if (near || far)
		{
			// A 5x5 box blur of the view (GaussianBlur), mixed in by the
			// trait aura: the LightSource (41 px radius) at scale 15 or 8
			m_blur.resize((size_t) SCREEN_W * SCREEN_H);
			uint32_t *tmp = m_blur.data();
			for (int y = TOP; y < BOTTOM; y++)
			{
				const uint32_t *row = frame + y * SCREEN_W;
				int sr = 0, sg = 0, sb = 0;
				for (int x = -2; x <= 2; x++)
				{
					uint32_t c = row[x < 0 ? 0 : x];
					sr += R(c); sg += G(c); sb += B(c);
				}
				for (int x = 0; x < SCREEN_W; x++)
				{
					tmp[y * SCREEN_W + x] = RGB(sr / 5, sg / 5, sb / 5);
					uint32_t out = row[x - 2 < 0 ? 0 : x - 2], in = row[x + 3 >= SCREEN_W ? SCREEN_W - 1 : x + 3];
					sr += R(in) - R(out); sg += G(in) - G(out); sb += B(in) - B(out);
				}
			}
			float radius = 41 * (near ? 15 : 8) * 0.5f;
			for (int x = 0; x < SCREEN_W; x++)
			{
				int sr = 0, sg = 0, sb = 0;
				for (int y = TOP - 2; y <= TOP + 2; y++)
				{
					uint32_t c = tmp[(y < TOP ? TOP : y) * SCREEN_W + x];
					sr += R(c); sg += G(c); sb += B(c);
				}
				for (int y = TOP; y < BOTTOM; y++)
				{
					float dx = x - px, dy = y - py;
					float d = sqrtf(dx * dx + dy * dy) / radius;
					float w = near ? d : 1 - d;   // InvertMask for the myopia
					w = w < 0 ? 0 : w > 1 ? 1 : w;
					uint32_t &dst = frame[y * SCREEN_W + x];
					dst = Mix(dst, RGB(sr / 5, sg / 5, sb / 5), (int) (w * 256));
					int yo = y - 2 < TOP ? TOP : y - 2, yi = y + 3 >= BOTTOM ? BOTTOM - 1 : y + 3;
					uint32_t out = tmp[yo * SCREEN_W + x], in = tmp[yi * SCREEN_W + x];
					sr += R(in) - R(out); sg += G(in) - G(out); sb += B(in) - B(out);
				}
			}
		}

		if (dungeon || hero.HasTrait(TraitType::Glaucoma))
		{
			// ShadowFX: black at 0.7 minus the hero's light (LightSource x12)
			float radius = 41 * 12 * 0.5f;
			for (int y = TOP; y < BOTTOM; y++)
				for (int x = 0; x < SCREEN_W; x++)
				{
					float dx = x - px, dy = y - py;
					float light = 1 - sqrtf(dx * dx + dy * dy) / radius;
					float shadow = 0.7f - (light < 0 ? 0 : light);
					if (shadow <= 0) continue;
					uint32_t &c = frame[y * SCREEN_W + x];
					int keep = (int) ((1 - shadow) * 256);
					c = RGB(R(c) * keep >> 8, G(c) * keep >> 8, B(c) * keep >> 8);
				}
		}
	}

	void TraitEffects::Screen(uint32_t *frame, const Lineage::Heir &hero, bool glasses, DS2DEngine::Camera2D *camera, double dt)
	{
		if (glasses) return;
		if (hero.HasTrait(TraitType::ColorBlind))
		{
			// HSVEffect, saturation 0: the grey of each pixel
			for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
			{
				uint32_t c = frame[i];
				int l = (R(c) * 77 + G(c) * 150 + B(c) * 29) >> 8;
				frame[i] = RGB(l, l, l);
			}
		}
		else if (hero.HasTrait(TraitType::Nostalgic))
		{
			// The film grain over the view, then saturation 0.2, brightness
			// 0.1 and the sepia tint (180, 150, 80)
			if (m_grain == nullptr && SpriteSystem::SpriteLibrary::ContainsSprite("FilmGrain_Sprite"))
			{
				m_grain = new DS2DEngine::SpriteObj("FilmGrain_Sprite");
				m_grain->Scale(Vector2(2.015f, 2.05f));
				m_grain->ForceDraw(true);
				m_grain->AnimationDelay(1 / 30.0f);
				m_grain->PlayAnimation(true);
			}
			if (m_grain != nullptr)
			{
				m_grain->Position(Vector2(camera->X() - 660 - 5, camera->Y() - 360 - 5));
				gfx_begin(frame);
				m_grain->Draw(camera);
			}
			(void) dt;
			for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
			{
				uint32_t c = frame[i];
				int l = (R(c) * 77 + G(c) * 150 + B(c) * 29) >> 8;
				int r = l + (R(c) - l) / 5 + 25, g = l + (G(c) - l) / 5 + 25, b = l + (B(c) - l) / 5 + 25;
				frame[i] = RGB(r * 180 / 255, g * 150 / 255, b * 80 / 255);
			}
		}
	}
}
