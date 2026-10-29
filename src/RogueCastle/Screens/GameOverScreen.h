/* Port of RogueCastle/src/Screens/GameOverScreen.cs: the death cinematic.
 * Timeline (GameOverScreen.OnEnter, t = 0 at the death):
 *   0.0-0.5 s  the frozen castle frame fades to black
 *   1.0 s      the spotlight lights the body (+ Player_Death_Spotlight)
 *   1.0-2.0 s  the camera closes in on the body (Zoom 1, Quad.EaseInOut)
 *   1.2 s      the GameOverStinger
 *   2.0 s      the death animation (RunDeathAnimation1) with its per-frame
 *              sounds (SwordTwirl 2, SwordLand 9, BodyFall 14), and the
 *              dialogue plate: the slain text, a game hint, the parting words
 *   3.0 s      the ghost rises (+ Player_Ghost), swaying, gone at 4.5 s
 *   4.0 s      DropStats: the enemies killed in the run drop in a line
 *              (0.05 s apart, Enemy_Kill_Plant each), the heir's framed
 *              portrait fades in, the continue text
 */
#ifndef ROGUE_CASTLE_GAME_OVER_SCREEN_H
#define ROGUE_CASTLE_GAME_OVER_SCREEN_H

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../Text/Font.h"
#include "../Lineage/Lineage.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace RogueCastle
{
	class EnemyObj;

	class GameOverScreen
	{
	public:
		// How the dead hero looked: each part's tint and visibility, the class
		// piece ("Shield_Sprite"...), and where it was in the screen's 1320x720
		struct HeroLook
		{
			std::vector<Color> colours;
			std::vector<bool> visible;
			std::string extraSuffix;
			bool flip = false;
			float scale = 2, x = 660, y = 360, feet = 60;   // feet: Bounds.Bottom - Y
		};

		~GameOverScreen() { Close(); }
		// lastFrame: the castle frame at the moment of death (SCREEN_W*SCREEN_H)
		// kills: EnemiesKilledInRun (type, difficulty), the DropStats parade
		bool Open(const char *assetsDir, const std::vector<uint32_t> &lastFrame,
		          const Lineage::Heir &hero, const HeroLook &look,
		          const std::vector<std::pair<uint8_t, int>> &kills,
		          int killerKind = 0, const std::string &killerName = "");
		void Close();
		// true once A is pressed after the sequence
		bool Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		void Dress(DS2DEngine::ObjContainer *hero, const char *sprite);
		DS2DEngine::ObjContainer *MakeHero(const char *sprite, float scale);

		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // starts at the 1320x720 view, closes to Zoom 1
		DS2DEngine::SpriteObj *m_spotlight = nullptr, *m_ghost = nullptr, *m_plate = nullptr;
		DS2DEngine::SpriteObj *m_frame = nullptr, *m_plaque = nullptr;   // the heir's card
		DS2DEngine::ObjContainer *m_hero = nullptr;      // the body, lit, over the dark
		DS2DEngine::ObjContainer *m_portrait = nullptr;   // the heir's card portrait
		std::vector<EnemyObj *> m_parade;   // the killed enemies (owned)
		std::vector<double> m_paradeAt;     // when each one appears
		HeroLook m_look;
		Lineage::Heir m_heir;
		std::string m_slain, m_hint, m_partingTitle, m_continue, m_classLine, m_traitLine;
		std::vector<uint32_t> m_lastFrame;
		double m_time = 0;
		bool m_open = false, m_female = false;
		bool m_deathSound[16] = {};   // the per-frame sounds, by frame index
		double m_ghostDrift = 0;      // the sway integration (GameOverScreen.cs line 528)
	};
}

#endif
