/* Port of DS2DEngine/src/CollisionMath.cs (subset: axis-aligned rectangles;
 * rotated walls are still TODO, they barely occur in room templates).
 */
#ifndef DS2D_COLLISION_MATH_H
#define DS2D_COLLISION_MATH_H

#include "Types.h"

namespace DS2DEngine
{

class CollisionMath
{
public:
	/* Minimum translation to move 'left' out of 'right' (CollisionMath.cs
	 * line 445). result.Y < 0: 'right' is below, > 0: above; result.X > 0:
	 * push right, < 0 push left. Zero when they do not intersect. */
	static Vector2 CalculateMTD(Rectangle left, Rectangle right);

	static bool Intersects(Rectangle a, Rectangle b)
	{
		return a.Right() > b.Left() && a.Left() < b.Right() &&
		       a.Bottom() > b.Top() && a.Top() < b.Bottom();
	}

	static bool Contains(Rectangle a, int x, int y)
	{
		return x >= a.Left() && x < a.Right() && y >= a.Top() && y < a.Bottom();
	}
};

} // namespace DS2DEngine

#endif
