#include "CollisionMath.h"

namespace DS2DEngine
{

Vector2 CollisionMath::CalculateMTD(Rectangle left, Rectangle right)
{
	Vector2 result(0, 0);
	float difference = 0.0f;
	float minimumTranslationDistance = 0.0f;
	int axis = 0, side = 0;

	/* Left (CollisionMath.cs line 445). */
	difference = (float) (left.Right() - right.Left());
	if (difference < 0.0f)
		return Vector2(0, 0);
	minimumTranslationDistance = difference;
	axis = 0;
	side = -1;

	/* Right */
	difference = (float) (right.Right() - left.Left());
	if (difference < 0.0f)
		return Vector2(0, 0);
	if (difference < minimumTranslationDistance)
	{
		minimumTranslationDistance = difference;
		axis = 0;
		side = 1;
	}

	/* Down */
	difference = (float) (left.Bottom() - right.Top());
	if (difference < 0.0f)
		return Vector2(0, 0);
	if (difference < minimumTranslationDistance)
	{
		minimumTranslationDistance = difference;
		axis = 1;
		side = -1;
	}

	/* Up */
	difference = (float) (right.Bottom() - left.Top());
	if (difference < 0.0f)
		return Vector2(0, 0);
	if (difference < minimumTranslationDistance)
	{
		minimumTranslationDistance = difference;
		axis = 1;
		side = 1;
	}

	if (axis == 1)
		result.Y = (float) side * minimumTranslationDistance;
	else
		result.X = (float) side * minimumTranslationDistance;

	return result;
}

} // namespace DS2DEngine
