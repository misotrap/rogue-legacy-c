#include "StartingRoom.h"

#include "../GameObjects/Player/PlayerObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../platform.h"

#include <cmath>

using DS2DEngine::GameObj;
using DS2DEngine::ObjContainer;
using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const float FLOOR = 720 - 60;   // the courtyard floor (one tile up)

		// Puts the object's bottom on the floor: 720 - 60 - (Bounds.Bottom - Y) - lift
		void OnFloor(GameObj *o, float x, float lift)
		{
			o->Position(Vector2(x, 0));
			float below = o->Bounds().Bottom() - o->Y();
			o->Y(FLOOR - below - lift);
		}

		ObjContainer *Npc(const char *name, float scale, float x, float lift)
		{
			ObjContainer *c = new ObjContainer(name);
			c->ForceDraw(true);
			c->Flip(SpriteEffects::FlipHorizontally);
			c->Scale(Vector2(scale, scale));
			c->AnimationDelay(1 / 10.0f);
			c->PlayAnimation(true);
			OnFloor(c, x, lift);
			return c;
		}

		SpriteObj *Prop(const char *name, float scale)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			s->Scale(Vector2(scale, scale));
			s->TextureColor(Color(200, 200, 200));
			return s;
		}

		bool Has(const Skills::SkillTree &skills, Skills::SkillType t) { return skills.ModifierAmount(t) > 0; }

		// The player stands next to the NPC (the original's talk zones are
		// 100 px either side of the NPC bounds)
		bool Near(PlayerObj *player, GameObj *npc)
		{
			Rectangle b = npc->Bounds();
			float px = player->X();
			return px > b.Left() - 100 && px < b.Right() + 100;
		}
	}

	StartingRoom::~StartingRoom()
	{
		delete m_blacksmith; delete m_enchantress; delete m_architect;
		delete m_board; delete m_tent; delete m_gear; delete m_arrow;
	}

	bool StartingRoom::LoadContent(const char *assetsDir)
	{
		using SpriteSystem::SpriteLibrary;
		bool ok = true;
		if (!SpriteLibrary::ContainsCharacter("Blacksmith_Character"))
			ok = SpriteLibrary::LoadSheet(assetsDir, "blacksmithUISpritesheet") && ok;
		if (!SpriteLibrary::ContainsSprite("StartRoomGypsyTent_Sprite"))
			ok = SpriteLibrary::LoadSheet(assetsDir, "startingRoomSpritesheet") && ok;
		if (!SpriteLibrary::ContainsSprite("UpArrowBubble_Sprite"))
			ok = SpriteLibrary::LoadSheet(assetsDir, "uiExtraSpritesheet") && ok;

		m_blacksmith = Npc("Blacksmith_Character", 2.5f, 700, 1);
		m_board = Prop("StartRoomBlacksmithBoard_Sprite", 2);
		m_board->Position(Vector2(m_blacksmith->X() - m_board->Width() / 2 - 35,
			m_blacksmith->Bounds().Bottom() - m_board->Height() - 1));

		m_enchantress = Npc("Enchantress_Character", 2, 1150, 2);
		m_tent = Prop("StartRoomGypsyTent_Sprite", 1.5f);
		m_tent->Position(Vector2(m_enchantress->X() - m_tent->Width() / 2 + 5,
			m_enchantress->Bounds().Bottom() - m_tent->Height()));

		m_architect = Npc("ArchitectIdle_Character", 2, 1550, 2);
		m_gear = Prop("ArchitectGear_Sprite", 2);
		m_gear->Position(Vector2(m_architect->X() + 30, m_architect->Bounds().Bottom() - 1));

		m_arrow = new SpriteObj("UpArrowBubble_Sprite");
		m_arrow->ForceDraw(true);
		m_arrow->Scale(Vector2(2, 2));
		m_arrow->Flip(SpriteEffects::FlipHorizontally);
		return ok;
	}

	void StartingRoom::PlaceAtLeftEdge(PlayerObj *player)
	{
		// StartingRoomObj.OnEnter: Position = (0, 720 - 60 - (Bounds.Bottom - Y))
		float below = player->Bounds().Bottom() - player->Y();
		player->X(60);
		player->Y(FLOOR - below);
		player->UpdateCollisionBoxes();
	}

	StartingRoom::Action StartingRoom::Update(PlayerObj *player, const Skills::SkillTree &skills, uint32_t pressed, double dt)
	{
		m_time += dt;
		m_near = nullptr;
		if (player == nullptr) return Action::None;
		// Leaving on the left: the manor (StartingRoomObj.Update)
		if (player->X() < 0) return Action::Manor;

		if (Has(skills, Skills::SkillType::Smithy) && Near(player, m_blacksmith)) m_near = m_blacksmith;
		else if (Has(skills, Skills::SkillType::Enchanter) && Near(player, m_enchantress)) m_near = m_enchantress;
		else if (Has(skills, Skills::SkillType::Architect) && Near(player, m_architect)) m_near = m_architect;
		if (m_near != nullptr && (pressed & BTN_UP))
			return m_near == m_blacksmith ? Action::Blacksmith : m_near == m_enchantress ? Action::Enchantress : Action::Architect;
		return Action::None;
	}

	void StartingRoom::Draw(DS2DEngine::Camera2D *camera, const Skills::SkillTree &skills)
	{
		if (Has(skills, Skills::SkillType::Smithy))
		{
			m_board->Draw(camera);
			m_blacksmith->Draw(camera);
		}
		if (Has(skills, Skills::SkillType::Enchanter))
		{
			m_tent->Draw(camera);
			m_enchantress->Draw(camera);
		}
		if (Has(skills, Skills::SkillType::Architect))
		{
			m_gear->Draw(camera);
			m_architect->Draw(camera);
		}
		if (m_near != nullptr)
		{
			// The talk arrow bobs over the NPC (sin(time * 20) * 2), above
			// the blacksmith's board so it does not sink into it
			Rectangle b = m_near->Bounds();
			float top = (float) b.Top();
			if (m_near == m_blacksmith && m_board->Bounds().Top() < top) top = (float) m_board->Bounds().Top();
			m_arrow->Position(Vector2(b.X + b.Width / 2.0f,
				top - 40 + (float) std::sin(m_time * 20) * 2));
			m_arrow->Draw(camera);
		}
	}
}
