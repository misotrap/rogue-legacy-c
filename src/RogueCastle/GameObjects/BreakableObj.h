/* Port of RogueCastle/src/Game Objects/BreakableObj.cs: a destructible
 * room object (chairs, barrels, urns...). Solid like terrain until the
 * player's weapon touches it, then it breaks (frame 2, all parts visible)
 * and stops colliding. Item drops come with the lineage milestone.
 */
#ifndef ROGUE_CASTLE_BREAKABLE_OBJ_H
#include <vector>
#define ROGUE_CASTLE_BREAKABLE_OBJ_H

#include "../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include <string>

namespace RogueCastle
{

class BreakableObj : public DS2DEngine::PhysicsObjContainer
{
public:
	explicit BreakableObj(const std::string &spriteName);

	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

	void Break();
	bool Broken = false;
	/* The pieces fly off and fade (Break's tweens, 0.3 s) */
	void Draw(DS2DEngine::Camera2D *camera) override;
	bool DropItem = true;
	bool HitBySpellsOnly = false;
	/* The room object's Name attribute ("Health"/"Mana" force a drop,
	 * BreakableObj.cs line 80). */
	std::string Name;
	void SetLevelScreen(class ProceduralLevelScreen *levelScreen)
	{
		m_levelScreen = levelScreen;
	}

private:
	bool m_wasWeighted = true;
	struct Piece { float x, y, dx, rotation; };
	std::vector<Piece> m_pieces;
	float m_breakTime = -1;
	class ProceduralLevelScreen *m_levelScreen = nullptr;
};

} // namespace RogueCastle

#endif
