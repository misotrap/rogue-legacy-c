/* Port of RogueCastle/src/Game Objects/TerrainObj.cs (subset). */
#include "TerrainObj.h"
#include "../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

TerrainObj::TerrainObj(int width, int height)
{
	/* TerrainObj.cs constructor: CollisionTypeTag = WALL, IsCollidable,
	 * IsWeighted = false. The origin is the top-left corner (BlankObj has
	 * no anchor offset), so collision boxes start at (0, 0). */
	_width = width;
	_height = height;
	Anchor(Vector2(0, 0));
}

TerrainObj::~TerrainObj()
{
	for (CollisionBox *box : m_collisionBoxes)
		delete box;
}

void TerrainObj::Populate(int x, int y, int width, int height,
                          bool collidesTop, bool collidesBottom, bool collidesLeft, bool collidesRight)
{
	_position = Vector2((float) x, (float) y);
	_width = width;
	_height = height;
	m_collidesTop = collidesTop;
	m_collidesBottom = collidesBottom;
	m_collidesLeft = collidesLeft;
	m_collidesRight = collidesRight;

	/* TerrainObj.cs PopulateFromXMLReader: terrain + body boxes over the
	 * whole rectangle; fall-through platforms get half the height. */
	if (collidesTop && !collidesBottom && !collidesLeft && !collidesRight)
		_height = height / 2;

	m_collisionBoxes.clear();
	CollisionBox *terrainBox = new CollisionBox(0, 0, _width, _height, Consts::TERRAIN_HITBOX, this);
	terrainBox->PhysicsParent(this);
	m_collisionBoxes.push_back(terrainBox);
	CollisionBox *bodyBox = new CollisionBox(0, 0, _width, _height, Consts::BODY_HITBOX, this);
	bodyBox->PhysicsParent(this);
	m_collisionBoxes.push_back(bodyBox);
}

void TerrainObj::Draw(Camera2D *camera)
{
	(void) camera;
	/* Walls are plain coloured rectangles (AGENTS.md); the room draws
	 * them through the camera transform (RoomObj::Draw). */
}

} // namespace RogueCastle
