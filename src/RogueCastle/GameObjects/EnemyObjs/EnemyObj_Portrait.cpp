/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Portrait.cs. */
#include "EnemyObj_Portrait.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include <cstdlib>
#include <string>
#include "../../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_Portrait::EnemyObj_Portrait(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyPortrait_Character", target, difficulty)
{
	Type = EnemyType::Portrait;
	/* A random painting in the frame (EnemyObj_Portrait.cs constructor) */
	/* The painting covers the frame, which keeps the hit boxes (the C#
	 * swaps the sprite and adds the boxes by hand, InitializeLogic) */
	m_picture = "FramePicture" + std::to_string(1 + rand() % 16) + "_Sprite";
	m_face = new SpriteObj(m_picture);
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Portrait.cs line 22), so they collapse into one. */
void EnemyObj_Portrait::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Portrait_Miniboss_Name;
		MaxHealth = EnemyEV::Portrait_Miniboss_MaxHealth;
		Damage = EnemyEV::Portrait_Miniboss_Damage;
		XPValue = EnemyEV::Portrait_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Miniboss_DropChance;
		Speed = EnemyEV::Portrait_Miniboss_Speed;
		TurnSpeed = EnemyEV::Portrait_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Miniboss_Jump;
		CooldownTime = EnemyEV::Portrait_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Miniboss_Scale.X, EnemyEV::Portrait_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Miniboss_ProjectileScale.X, EnemyEV::Portrait_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Miniboss_Tint.R, EnemyEV::Portrait_Miniboss_Tint.G, EnemyEV::Portrait_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Miniboss_KnockBack.X, EnemyEV::Portrait_Miniboss_KnockBack.Y);
		/* InitializeLogic: the giant portrait and its painting */
		if (NumChildren() > 0 && !m_giant)
		{
			m_giant = true;
			GetChildAt(0)->ChangeSprite("GiantPortrait_Sprite");
			Scale(Vector2(2, 2));
			SpriteObj *pic = new SpriteObj("Portrait" + std::to_string((rand() % 8)) + "_Sprite");
			pic->Scale(Vector2(0.5f, 0.5f));   // OverrideParentScale
			AddChild(pic);
		}
		break;
	case EXPERT:
		Name = EnemyEV::Portrait_Expert_Name;
		MaxHealth = EnemyEV::Portrait_Expert_MaxHealth;
		Damage = EnemyEV::Portrait_Expert_Damage;
		XPValue = EnemyEV::Portrait_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Expert_DropChance;
		Speed = EnemyEV::Portrait_Expert_Speed;
		TurnSpeed = EnemyEV::Portrait_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Expert_Jump;
		CooldownTime = EnemyEV::Portrait_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Expert_Scale.X, EnemyEV::Portrait_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Expert_ProjectileScale.X, EnemyEV::Portrait_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Expert_Tint.R, EnemyEV::Portrait_Expert_Tint.G, EnemyEV::Portrait_Expert_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Expert_KnockBack.X, EnemyEV::Portrait_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Portrait_Advanced_Name;
		MaxHealth = EnemyEV::Portrait_Advanced_MaxHealth;
		Damage = EnemyEV::Portrait_Advanced_Damage;
		XPValue = EnemyEV::Portrait_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Advanced_DropChance;
		Speed = EnemyEV::Portrait_Advanced_Speed;
		TurnSpeed = EnemyEV::Portrait_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Advanced_Jump;
		CooldownTime = EnemyEV::Portrait_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Advanced_Scale.X, EnemyEV::Portrait_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Advanced_ProjectileScale.X, EnemyEV::Portrait_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Advanced_Tint.R, EnemyEV::Portrait_Advanced_Tint.G, EnemyEV::Portrait_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Advanced_KnockBack.X, EnemyEV::Portrait_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Portrait_Basic_Name;
		MaxHealth = EnemyEV::Portrait_Basic_MaxHealth;
		Damage = EnemyEV::Portrait_Basic_Damage;
		XPValue = EnemyEV::Portrait_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Basic_DropChance;
		Speed = EnemyEV::Portrait_Basic_Speed;
		TurnSpeed = EnemyEV::Portrait_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Basic_Jump;
		CooldownTime = EnemyEV::Portrait_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Basic_Scale.X, EnemyEV::Portrait_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Basic_ProjectileScale.X, EnemyEV::Portrait_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Basic_Tint.R, EnemyEV::Portrait_Basic_Tint.G, EnemyEV::Portrait_Basic_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Basic_KnockBack.X, EnemyEV::Portrait_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Portrait::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Portrait::FireSpread()
{
	/* ThrowAdvancedProjectiles (90/-90), ThrowExpertProjectiles (the
	 * cross), ThrowProjectiles (the miniboss's diagonals) */
	static const float advanced[2] = { 90, -90 };
	static const float cross[4] = { 0, 90, -90, 180 };
	static const float diagonals[4] = { 135, 45, -45, -135 };
	const float *angles = DifficultyType == MINIBOSS ? diagonals : DifficultyType == EXPERT ? cross : advanced;
	int count = DifficultyType == ADVANCED ? 2 : 4;
	Audio::PlaySound("FairyAttack1");
	for (int i = 0; i < count; i++)
	{
		float rad = angles[i] * PI / 180.0f;
		m_levelScreen->FireProjectile("GhostProjectile_Sprite", Position(),
		                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
		                              (int) Damage, ProjectileScale, false, this);
	}
}

void EnemyObj_Portrait::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 1;

	switch (action)
	{
	case ACTION_SHAKE:
		/* basicWarningLS: shake for a second, rest a second */
		CurrentSpeed(0);
		m_shake = true;
		m_actionTimer = 1.0f;
		break;
	case ACTION_CHASE:
		/* moveTowards*LS: fly at the hero, then (above basic) fire */
		CurrentSpeed(Speed);
		m_actionTimer = DifficultyType == MINIBOSS ? 1.25f : DifficultyType == BASIC ? 1.0f : 1.75f;
		break;
	default:
		break;
	}
}

void EnemyObj_Portrait::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_SHAKE:
		if (m_actionTimer <= 0)
		{
			if (m_shake)
			{
				m_shake = false;
				m_actionTimer = 1.0f;
			}
			else
				m_actionPhase = 1;
		}
		break;
	case ACTION_CHASE:
		{
			float dx = m_target->X() - X();
			float dy = m_target->Y() - Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
		}
		if (m_actionTimer <= 0 && m_actionPhase == 0)
		{
			if (DifficultyType != BASIC)
				FireSpread();
			m_actionPhase = 1;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Portrait::ChasePlayer()
{
	/* Hit, touched or approached: the portrait wakes up for good */
	if (m_chasing)
		return;
	m_actionActive = false;
	m_shake = false;
	m_chasing = true;
}

void EnemyObj_Portrait::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	ChasePlayer();
	EnemyObj::HitEnemy(damage, collisionPt, isPlayer);
}

void EnemyObj_Portrait::UpdateEnemy(GameTime *gameTime)
{
	float dt = (float) gameTime->ElapsedGameTime;
	m_time += dt;
	if (!IsKilled && Bounds().Intersects(m_target->Bounds()))
		ChasePlayer();   // CollisionResponse with the player

	if (!m_chasing)
	{
		if (DifficultyType != MINIBOSS)
			Rotation(m_shake ? sinf(m_time * 15) * 2 : 0);
	}
	else
	{
		// Spinning, with its true face (ChangePortrait)
		Rotation(Rotation() + (DifficultyType == MINIBOSS ? 420 : 600) * dt);
		std::string face = "EnemyPortrait" + std::to_string((int) DifficultyType) + "_Sprite";
		if (m_face != nullptr && m_face->SpriteName() != face)
		{
			Audio::PlaySound("FinalBoss_St2_BlockLaugh");
			m_face->ChangeSprite(face);
			if (DifficultyType == MINIBOSS && NumChildren() > 1)
				GetChildAt(1)->Visible(false);
		}
	}
	UpdateAction();
}

void EnemyObj_Portrait::RunBasicLogic()
{
	/* EnemyObj_Portrait.cs line 283: close by, it chases; in sight, it
	 * shakes a warning; once chasing, it keeps flying at the hero */
	if (m_chasing)
	{
		StartAction(ACTION_CHASE);
		return;
	}
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
		ChasePlayer();
		break;
	case STATE_ENGAGE:
		StartAction(ACTION_SHAKE);
		break;
	default:
		break;
	}
}

EnemyObj_Portrait::~EnemyObj_Portrait()
{
	delete m_face;
}

void EnemyObj_Portrait::Draw(Camera2D *camera)
{
	EnemyObj::Draw(camera);
	if (m_face == nullptr || !Visible() || IsKilled || DifficultyType == MINIBOSS)
		return;
	m_face->Position(Position());
	m_face->Scale(Scale());
	m_face->Rotation(Rotation());
	m_face->Flip(Flip());
	m_face->Opacity(Opacity());
	m_face->Draw(camera);
}

} // namespace RogueCastle
