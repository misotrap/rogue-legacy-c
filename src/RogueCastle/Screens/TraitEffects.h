// The traits that change how the castle looks (ProceduralLevelScreen.Draw
// and VirtualScreen.Draw), applied to the finished frame:
//   World pass, before the HUD: the darkness around the hero (the dungeon,
//   Glaucoma: ShadowIntensity 0.7 minus the 12x LightSource), the blur of
//   NearSighted (away from the hero) and FarSighted (near him).
//   Screen pass, after the HUD: Nostalgic's film grain and sepia (HSV
//   saturation 0.2, brightness 0.1, tinted 180,150,80), ColorBlind's grey.
//   (Vertigo's upside-down screen is in main.cpp, GameFlow::VertigoActive.)
// The Glasses (special item) cancel all but the darkness.
#pragma once

#include "../Lineage/Lineage.h"

#include <cstdint>
#include <vector>

namespace DS2DEngine { class Camera2D; class SpriteObj; }

namespace RogueCastle
{
	class TraitEffects
	{
	public:
		~TraitEffects();
		// hero: the traits; glasses: SpecialItem == Glasses; px, py: the
		// hero on screen
		void World(uint32_t *frame, const Lineage::Heir &hero, bool glasses, bool dungeon, float px, float py);
		void Screen(uint32_t *frame, const Lineage::Heir &hero, bool glasses, DS2DEngine::Camera2D *camera, double dt);

	private:
		std::vector<uint32_t> m_blur;
		DS2DEngine::SpriteObj *m_grain = nullptr;
	};
}
