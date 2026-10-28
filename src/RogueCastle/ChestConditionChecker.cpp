/* Port of RogueCastle/src/ChestConditionChecker.cs. */
#include "ChestConditionChecker.h"
#include "GameObjects/FairyChestObj.h"
#include "GameObjects/Player/PlayerObj.h"
#include "GameObjects/EnemyObjs/EnemyObj.h"
#include "Screens/ProceduralLevelScreen.h"
#include "GameObjects/RoomObjs/RoomObj.h"
#include "Types/ChestConditionType.h"
#include <cmath>

namespace RogueCastle
{

using namespace DS2DEngine;

static float Distance(Vector2 a, Vector2 b)
{
	float dx = a.X - b.X, dy = a.Y - b.Y;
	return sqrtf(dx * dx + dy * dy);
}

void ChestConditionChecker::SetConditionState(FairyChestObj *chest, PlayerObj *player,
                                              ProceduralLevelScreen *level)
{
	/* ChestConditionChecker.cs line 16 (SetConditionState). */
	if (chest == nullptr || player == nullptr || level == nullptr)
		return;
	const int distanceCheck = 100;
	Vector2 chestPos(chest->X(), chest->Y());
	Vector2 playerPos(player->X(), player->Y());
	float distance = Distance(chestPos, playerPos);

	switch (chest->ConditionType())
	{
	case ChestConditionType::InvisibleChest:
	case ChestConditionType::None:
		if (distance < distanceCheck)
			chest->SetChestUnlocked();
		break;
	case ChestConditionType::KillAllEnemies:
		if (level->CurrentRoom() != nullptr && level->CurrentRoom()->ActiveEnemies() <= 0)
			chest->SetChestUnlocked();
		break;
	case ChestConditionType::DontLook:
	{
		SpriteEffects flipCheck = SpriteEffects::None;
		if (chest->X() < player->X())   /* the chest is to the left */
			flipCheck = SpriteEffects::FlipHorizontally;
		if (distance < 375 && distance > distanceCheck && player->Flip() == flipCheck)
			chest->SetChestFailed();
		else if (distance < distanceCheck)
			chest->SetChestUnlocked();
		break;
	}
	case ChestConditionType::NoJumping:
		if (distance < 10000 && player->IsJumping() && player->AccelerationY() < 0
		    && distance > distanceCheck)
			chest->SetChestFailed();
		else if (distance < distanceCheck)
			chest->SetChestUnlocked();
		break;
	case ChestConditionType::NoFloor:
		/* Not used by the game (ChestConditionChecker.cs line 54). */
		if (player->IsTouchingGround() && distance > distanceCheck && distance < 1000)
			chest->SetChestFailed();
		else if (distance < distanceCheck)
			chest->SetChestUnlocked();
		break;
	case ChestConditionType::NoAttackingEnemies:
		if (level->CurrentRoom() != nullptr)
		{
			for (const EnemyObj *enemy : level->CurrentRoom()->EnemyList())
			{
				if (enemy->CurrentHealth < enemy->MaxHealth)
				{
					chest->SetChestFailed();
					break;
				}
			}
			if (distance < distanceCheck && chest->State == STATE_LOCKED)
				chest->SetChestUnlocked();
		}
		break;
	case ChestConditionType::ReachIn5Seconds:
		if (distance > distanceCheck && chest->Timer <= 0)
			chest->SetChestFailed();
		else if (distance < distanceCheck && chest->Timer > 0)
			chest->SetChestUnlocked();
		break;
	case ChestConditionType::TakeNoDamage:
		if (player->State == PlayerObj::STATE_HURT)
			chest->SetChestFailed();
		else if (distance < distanceCheck)
			chest->SetChestUnlocked();
		break;
	default:
		break;
	}
}

} // namespace RogueCastle
