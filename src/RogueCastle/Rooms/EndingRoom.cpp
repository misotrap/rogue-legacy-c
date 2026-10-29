/* Port of RogueCastle/src/Game Objects/RoomObjs/EndingRoomObj.cs. */
#include "EndingRoom.h"
#include "../../Audio/Audio.h"
#include "../../gfx.h"
#include "../../platform.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/EnemyObjs/EnemyObj_BallAndChain.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../Levels/RoomData.h"
#include "../Levels/LevelParser.h"
#include "../Screens/EnemyNames.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../Types/EnemyType.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace RogueCastle
{

namespace
{
	const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
	const uint32_t OUTLINE = 0xFFB58E27u;   // the C# text outline colour
	/* World (1320x720) to screen coordinates through the camera. */
	float SX(float x, Vector2 cam) { return (x - cam.X) * 0.5f + SCREEN_W / 2; }
	float SY(float y, Vector2 cam) { return (y - cam.Y) * 0.5f + SCREEN_H / 2; }
	float Clamp01(float t) { return t < 0 ? 0 : t > 1 ? 1 : t; }
	float QuadInOut(float t)
	{
		t = Clamp01(t);
		return t < 0.5f ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t);
	}

	DS2DEngine::SpriteObj *Make(const char *name)
	{
		DS2DEngine::SpriteObj *s = new DS2DEngine::SpriteObj(name);
		s->ForceDraw(true);
		return s;
	}

	const char *EnemyLocID(const std::string &name)
	{
		for (const EnemyNameLoc &e : ENEMY_NAMES)
			if (name == e.name) return e.locID;
		return nullptr;
	}
}

EndingRoom::~EndingRoom()
{
	delete m_room;
	delete m_bg;
	for (Exhibit &e : m_exhibits)
	{
		delete e.frame;
		delete e.plaque;
	}
}

bool EndingRoom::LoadContent(const char *assetsDir)
{
	/* Junicode: the names (FontSize 12) and the slain counts (10). The
	 * UI and enemy sheets load in Open(), with the template. */
	return m_font.Load(assetsDir, "Junicode");
}

void EndingRoom::Open(const char *assetsDir, Save::GameState &save)
{
	/* A fresh gallery for every ending. */
	delete m_room;
	m_room = nullptr;
	for (Exhibit &e : m_exhibits)
	{
		delete e.frame;
		delete e.plaque;
	}
	m_exhibits.clear();
	m_waypoints.clear();
	m_waypointIndex = 1;
	m_waypointTime = 0;
	m_continueShown = false;
	m_continueTimer = 0;
	m_continueOpacity = 0;

	/* CreateEndingRoom: the "Ending" template, moved to (0,0) (the
	 * parser stores the objects relative to the room already). */
	std::vector<Levels::RoomTemplate> templates;
	char path[512];
	snprintf(path, sizeof(path), "%s/rooms.txt", assetsDir);
	if (!Levels::LoadRooms(path, templates))
		return;
	int templ = -1;
	for (size_t i = 0; i < templates.size(); i++)
		if (templates[i].name == "Ending") { templ = (int) i; break; }
	if (templ < 0)
		return;
	const Levels::RoomTemplate &t = templates[templ];

	/* The waypoints (m_cameraPosList): the EnemyOrbObj with IsWaypoint,
	 * their Name attribute is the index. */
	for (const Levels::ObjData &o : t.objs)
	{
		if (o.type == "EnemyOrbObj" && o.isWaypoint)
		{
			int index = atoi(o.name.c_str());
			if (index < 0) continue;
			if (index >= (int) m_waypoints.size())
				m_waypoints.resize(index + 1, Vector2(0, 0));
			m_waypoints[index] = Vector2(o.x, o.y);
		}
	}
	while (!m_waypoints.empty() && m_waypoints.back() == Vector2(0, 0))
		m_waypoints.pop_back();
	if (m_waypoints.empty())
		m_waypoints.push_back(Vector2(t.width * 0.5f, t.height * 0.5f));

	Levels::PlacedRoom p;
	p.templ = templ;
	p.levelType = Levels::LevelType::NONE;
	p.x = 0;
	p.y = 0;
	p.width = t.width;
	p.height = t.height;
	p.name = t.name;
	p.tag = t.tag;
	p.objs = t.objs;

	SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "enemyFinalSpritesheetBig");
	SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "enemyFinal2Spritesheet");
	SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "parallaxBGSpritesheet");   // LineageScreen*
	SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "portraitSpritesheet");     // GiantPortrait
	if (m_bg == nullptr && SpriteSystem::SpriteLibrary::ContainsSprite("LineageScreenBG_Sprite"))
		m_bg = Make("LineageScreenBG_Sprite");

	m_room = new RoomObj();
	/* The hero stays hidden: no target, no AI (PauseEnemy would stop
	 * the animation, so IsPaused is set directly). */
	m_room->LoadRoom(p, nullptr);
	for (EnemyObj *enemy : m_room->EnemyList())
	{
		if (enemy == nullptr) continue;
		enemy->IsPaused = true;
		enemy->DisableAllWeightFlag = true;
		enemy->IsCollidableFlag = false;

		/* The special cases of LoadContent (lines 185-272) and
		 * FixMiniboss. */
		const bool miniboss = enemy->DifficultyType == EnemyObj::MINIBOSS;
		const char *nameID = nullptr;
		switch (enemy->Type)
		{
		case EnemyType::Eyeball:
			if (miniboss)
				enemy->ChangeSprite("EnemyEyeballBossEye_Character");
			break;
		case EnemyType::Blob:
			if (miniboss)
			{
				enemy->ChangeSprite("EnemyBlobBossIdle_Character");
				if (enemy->NumChildren() > 2)
				{
					enemy->GetChildAt(0)->TextureColor(Color(255, 255, 255));
					enemy->GetChildAt(1)->TextureColor(Color(0, 0, 0));
					enemy->GetChildAt(2)->TextureColor(Color(135, 206, 250));
					enemy->GetChildAt(2)->Opacity(0.8f);
				}
			}
			break;
		case EnemyType::Fireball:
			if (miniboss)
				enemy->ChangeSprite("EnemyGhostBossIdle_Character");
			break;
		case EnemyType::Fairy:
			if (miniboss)
				enemy->ChangeSprite("EnemyFairyGhostBossIdle_Character");
			break;
		case EnemyType::Skeleton:
			if (miniboss)
				nameID = enemy->Flip() == SpriteEffects::None
				         ? "LOC_ID_ENEMY_NAME_108" : "LOC_ID_ENEMY_NAME_111";
			break;
		case EnemyType::Zombie:
			enemy->ChangeSprite("EnemyZombieWalk_Character");
			break;
		case EnemyType::BallAndChain:
			static_cast<EnemyObj_BallAndChain *>(enemy)->HideBalls();
			break;
		case EnemyType::LastBoss:
			if (enemy->DifficultyType == EnemyObj::ADVANCED)
				enemy->ChangeSprite("EnemyLastBossIdle_Character");
			break;
		default:
			break;
		}

		enemy->PlayAnimation(true);
		enemy->UpdateCollisionBoxes();

		/* The frame around it (a giant portrait for the minibosses). */
		DS2DEngine::SpriteObj *frame =
			Make(miniboss && enemy->Type != EnemyType::Portrait
			         ? "GiantPortrait_Sprite" : "LineageScreenFrame_Sprite");
		if (miniboss && enemy->Type == EnemyType::Portrait)
			frame->Visible(false);   // the portrait does not fit a frame
		float fw = frame->Width() > 0 ? frame->Width() : 1;
		float fh = frame->Height() > 0 ? frame->Height() : 1;
		float sx = (enemy->Width() + 150) / fw, sy = (enemy->Height() + 150) / fh;
		if (sx < 1) sx = 1;
		if (sy < 1) sy = 1;
		frame->Scale(Vector2(sx, sy));
		frame->Position(Vector2(enemy->X(),
		                        (float) enemy->Bounds().Top() + enemy->Height() * 0.5f));

		/* The name and the slain count under it. */
		Exhibit ex;
		ex.enemy = enemy;
		ex.frame = frame;
		ex.x = frame->X();
		ex.nameY = (float) frame->Bounds().Bottom() + 40;
		ex.slainY = (float) frame->Bounds().Bottom() + 80;
		const char *loc = nameID != nullptr ? nameID : EnemyLocID(enemy->Name);
		ex.name = loc != nullptr ? Text::Loc::Get(loc) : enemy->Name;
		int type = enemy->Type;
		if (type >= 0 && type < Save::ENEMY_TYPES)
			ex.slain = std::string(Text::Loc::Get("LOC_ID_ENDING_ROOM_OBJ_1")) + ": "
			           + std::to_string(save.enemiesKilled[type][(int) enemy->DifficultyType]);
		/* The plaque behind the slain count. */
		ex.plaque = Make("LineageScreenPlaque1Long_Sprite");
		ex.plaque->Scale(Vector2(1.8f, 1.8f));
		ex.plaque->Position(Vector2(ex.x, ex.slainY));
		m_exhibits.push_back(ex);
	}

	/* OnEnter: the camera starts at waypoint 0 and moves to the next
	 * one every m_waypointSpeed seconds. */
	m_camera.Position(m_waypoints[0]);
	m_camera.Zoom(0.5f);
	Audio::PlayMusic("EndSongDrums");
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "ending: %d waypoints, %d exhibits\n",
		        (int) m_waypoints.size(), (int) m_exhibits.size());
}

bool EndingRoom::Update(uint32_t pressed, double dt)
{
	if (!IsOpen())
		return true;
	m_waypointTime += dt;

	/* Update: any button shows the continue text; a press while it is
	 * shown leaves for the credits. */
	if (pressed != 0)
	{
		if (m_continueShown)
			return true;
		m_continueShown = true;
		m_continueTimer = 0;
	}
	if (m_continueShown)
	{
		m_continueTimer += dt;
		m_continueOpacity = Clamp01((float) m_continueTimer / 0.5f);
		if (m_continueOpacity >= 1 && m_continueTimer >= 4)   // HideContinueText
		{
			m_continueShown = false;
			m_continueTimer = 0;
		}
	}
	else if (m_continueOpacity > 0)
		m_continueOpacity = Clamp01(m_continueOpacity - (float) dt / 0.5f);

	/* ChangeWaypoints: the tween to the next waypoint lasts 1.5 s
	 * (Quad.EaseInOut), then the camera dwells m_waypointSpeed (5 s);
	 * the last one hands over to the credits. */
	if (m_waypointTime >= 1.5 + 5)
	{
		m_waypointIndex++;
		m_waypointTime = 0;
		if (m_waypointIndex >= m_waypoints.size())
			return true;   // DisplayScreen(ScreenType.Credits)
	}
	if (m_waypointIndex >= m_waypoints.size())
		return true;

	const Vector2 &from =
		m_waypointIndex > 0 ? m_waypoints[m_waypointIndex - 1] : m_waypoints[0];
	const Vector2 &to = m_waypoints[m_waypointIndex];
	if (m_waypointTime < 1.5)
	{
		float k = QuadInOut((float) (m_waypointTime / 1.5));
		m_camera.Position(Vector2(from.X + (to.X - from.X) * k,
		                          from.Y + (to.Y - from.Y) * k));
	}
	else
		m_camera.Position(to);

	/* The gallery animates (RoomObj::Update plays the enemies). */
	GameTime gameTime(dt);
	m_room->Update(&gameTime);
	return false;
}

void EndingRoom::Draw(uint32_t *frame)
{
	if (!IsOpen())
		return;
	gfx_begin(frame);
	/* The ending mask: everything behind the gallery is black. */
	gfx_fill(0, 0, SCREEN_W, SCREEN_H, BLACK);

	Vector2 cam = m_camera.Position();
	/* The lineage background, repeated at 0.7 opacity (BackgroundObj
	 * SetRepeated); like LineageScreen the tiles overlap by 2 px so no
	 * seam shows. */
	if (m_bg != nullptr)
	{
		m_bg->Opacity(0.7f);
		float tw = m_bg->Width() - 2, th = m_bg->Height() - 2;
		Rectangle view = m_camera.Bounds();
		float x0 = floorf(view.X / tw) * tw - tw, y0 = floorf(view.Y / th) * th - th;
		for (float y = y0; y < view.Y + view.Height + th; y += th)
			for (float x = x0; x < view.X + view.Width + tw; x += tw)
			{
				m_bg->Position(Vector2(x, y));
				m_bg->Draw(&m_camera);
			}
	}

	/* The frames, the plaques, then the enemies (the room draws them). */
	for (const Exhibit &e : m_exhibits)
	{
		e.frame->Draw(&m_camera);
		e.plaque->Draw(&m_camera);
	}
	m_room->Draw(&m_camera);

	/* The names and the slain counts (Junicode 12 and 10, golden
	 * outline, centred under the frames). */
	for (const Exhibit &e : m_exhibits)
	{
		if (!e.name.empty())
			m_font.DrawOutlined(frame, e.name.c_str(), SX(e.x, cam), SY(e.nameY, cam),
			                    0.3f, WHITE, OUTLINE, 1, Text::Align::Centre);
		if (!e.slain.empty())
			m_font.DrawOutlined(frame, e.slain.c_str(), SX(e.x, cam), SY(e.slainY, cam),
			                    0.25f, WHITE, OUTLINE, 1, Text::Align::Centre);
	}

	/* The continue hint, bottom right (m_continueText). */
	if (m_continueOpacity > 0.01f)
	{
		std::string hint = Text::Loc::Get("LOC_ID_ENDING_ROOM_OBJ_2_NEW");
		size_t input = hint.find("[Input:0]");
		if (input != std::string::npos)
			hint.replace(input, 9, "(A)");
		uint32_t alpha = (uint32_t) (m_continueOpacity * 255) << 24;
		float w = 0, h = 0;
		m_font.Measure(hint.c_str(), w, h);
		m_font.Draw(frame, hint.c_str(), SCREEN_W - 10 - w * 0.25f,
		            SCREEN_H - 24 - h * 0.25f, 0.25f, alpha | 0xFFFFFFu,
		            Text::Align::Right);
	}
}

} // namespace RogueCastle
