#include "GetItemScreen.h"
#include "../Types/SpecialItemType.h"

#include "../../Audio/Audio.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../Text/LocIDs.h"
#include "../../platform.h"
#include "../EVs/GameEV.h"
#include "../GameObjects/ItemDropObj.h"

#include <cmath>
#include <cstdlib>
#include <deque>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace GetItem
	{
		static std::deque<Reward> s_queue;
		void Push(const Reward &reward) { s_queue.push_back(reward); }
		bool Pop(Reward &reward)
		{
			if (s_queue.empty()) return false;
			reward = s_queue.front();
			s_queue.pop_front();
			return true;
		}
	}

	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const float END_Y = 720 / 2.0f + 30;   // ItemSpinAnimation3

		float SX(float x) { return (x - 660) * 0.5f + SCREEN_W / 2; }
		float SY(float y) { return (y - 360) * 0.5f + SCREEN_H / 2; }
		float Clamp01(float t) { return t < 0 ? 0 : t > 1 ? 1 : t; }
		float BackOut(float t) { t = Clamp01(t) - 1; return t * t * (2.70158f * t + 1.70158f) + 1; }
		float QuadOut(float t) { t = Clamp01(t); return t * (2 - t); }

		SpriteObj *Make(const char *name)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			return s;
		}

		const char *StatSprite(int type)
		{
			switch (type)
			{
			case ItemDropType::Stat_MaxHealth: return "Heart_Sprite";
			case ItemDropType::Stat_MaxMana: return "ManaCrystal_Sprite";
			case ItemDropType::Stat_Magic: return "MagicBook_Sprite";
			case ItemDropType::Stat_Defense: return "Shield_Sprite";
			case ItemDropType::Stat_Weight: return "Backpack_Sprite";
			default: return "Sword_Sprite";
			}
		}

		std::string StatText(int type, int timesBeaten)
		{
			const char *id = "LOC_ID_GET_ITEM_SCREEN_4";
			int amount = GameEV::ITEM_STAT_STRENGTH_AMOUNT;
			switch (type)
			{
			case ItemDropType::Stat_MaxHealth: id = "LOC_ID_GET_ITEM_SCREEN_2"; amount = GameEV::ITEM_STAT_MAXHP_AMOUNT; break;
			case ItemDropType::Stat_MaxMana: id = "LOC_ID_GET_ITEM_SCREEN_3"; amount = GameEV::ITEM_STAT_MAXMP_AMOUNT; break;
			case ItemDropType::Stat_Magic: id = "LOC_ID_GET_ITEM_SCREEN_5"; amount = GameEV::ITEM_STAT_MAGIC_AMOUNT; break;
			case ItemDropType::Stat_Defense: id = "LOC_ID_GET_ITEM_SCREEN_6"; amount = GameEV::ITEM_STAT_ARMOR_AMOUNT; break;
			case ItemDropType::Stat_Weight: id = "LOC_ID_GET_ITEM_SCREEN_7"; amount = GameEV::ITEM_STAT_WEIGHT_AMOUNT; break;
			}
			return std::string(Text::Loc::Get(id)) + ": +" + std::to_string(amount * (1 + timesBeaten));
		}
	}

	GetItemScreen::~GetItemScreen()
	{
		delete m_bg; delete m_found;
		for (SpriteObj *s : m_items) delete s;
		for (SpriteObj *s : m_particles) delete s;
	}

	bool GetItemScreen::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("BlueprintFoundBG_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "uiExtraSpritesheet") && ok;
		m_bg = Make("BlueprintFoundBG_Sprite");
		// The translated "found" texts are on the language sheet
		if (!SpriteSystem::SpriteLibrary::ContainsSprite(Text::Loc::Sprite("BlueprintFoundText_Sprite").c_str()))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "languageSpritesheet") && ok;
		m_found = Make(Text::Loc::Sprite("BlueprintFoundText_Sprite").c_str());   // Game.ChangeBitmapLanguage
		for (SpriteObj *&s : m_items) s = Make("BlueprintIcon_Sprite");
		for (SpriteObj *&s : m_particles) s = Make("LevelUpParticleFX_Sprite");
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		return ok;
	}

	static const char *SpecialItemLocID(int item)
{
	static char id[40];
	snprintf(id, sizeof(id), "LOC_ID_SPECIAL_ITEM_TYPE_%d", item);
	return id;
}

/* SpellType.Icon (line 110). */
static const char *SpellIcon(int spell)
{
	switch (spell)
	{
	case SpellType::Dagger: return "DaggerIcon_Sprite";
	case SpellType::Axe: return "AxeIcon_Sprite";
	case SpellType::TimeBomb: return "TimeBombIcon_Sprite";
	case SpellType::TimeStop: return "TimeStopIcon_Sprite";
	case SpellType::Nuke: return "NukeIcon_Sprite";
	case SpellType::Translocator: return "TranslocatorIcon_Sprite";
	case SpellType::Displacer: return "DisplacerIcon_Sprite";
	case SpellType::Boomerang: return "BoomerangIcon_Sprite";
	case SpellType::DualBlades: return "DualBladesIcon_Sprite";
	case SpellType::Close: return "CloseIcon_Sprite";
	case SpellType::DamageShield: return "DamageShieldIcon_Sprite";
	case SpellType::Bounce: return "BounceIcon_Sprite";
	case SpellType::DragonFire: return "DragonFireIcon_Sprite";
	case SpellType::RapidDagger: return "RapidDaggerIcon_Sprite";
	default: return "DaggerIcon_Sprite";
	}
}

void GetItemScreen::Open(const GetItem::Reward &reward, float camX, float camY, int timesCastleBeaten)
	{
		m_reward = reward;
		m_time = 0;
		m_exitTime = -1;
		m_rotation = 0;
		// The chest on the level screen, in 1320x720 screen units
		m_startX = reward.worldX - camX + 660;
		m_startY = reward.worldY - camY + 360;
		m_count = 1;
		m_texts[1].clear();
		m_texts[2].clear();
		switch (reward.type)
		{
		case GetItem::Blueprint:
			m_items[0]->ChangeSprite("BlueprintIcon_Sprite");
			m_found->ChangeSprite(Text::Loc::Sprite("BlueprintFoundText_Sprite").c_str());
			m_texts[0] = Text::Loc::Format("LOC_ID_EQUIPMENT_BASE_FORMAT",
				Text::Loc::Get(LocIDs::EquipmentBaseType_ToStringID(reward.y)),
				Text::Loc::Get(LocIDs::EquipmentCategoryType_ToStringID(reward.x)));
			break;
		case GetItem::Rune:
			m_items[0]->ChangeSprite("RuneIcon_Sprite");
			m_found->ChangeSprite(Text::Loc::Sprite("RuneFoundText_Sprite").c_str());
			m_texts[0] = Text::Loc::Format("LOC_ID_RUNE_BASE_FORMAT",
				Text::Loc::Get(LocIDs::EquipmentAbilityType_ToStringID(reward.y)),
				Text::Loc::Get("LOC_ID_ENCHANTRESS_SCREEN_13"));
			m_texts[1] = std::string("(") + Text::Loc::Get(LocIDs::EquipmentCategoryType_ToStringID2(reward.x)) + ")";
			break;
		case GetItem::Spell:
			/* GetItemScreen.cs line 204: the spell's icon and name */
			m_items[0]->ChangeSprite(SpellIcon(reward.x));
			m_found->ChangeSprite(Text::Loc::Sprite("SpellFoundText_Sprite").c_str());
			m_texts[0] = Text::Loc::Get(LocIDs::SpellType_ToStringID(reward.x));
			break;
		case GetItem::SpecialItem:
			/* GetItemScreen.cs line 199: the icon and name of the item */
			m_items[0]->ChangeSprite(SpecialItemType::SpriteName((uint8_t) reward.x));
			m_found->ChangeSprite(Text::Loc::Sprite("ItemFoundText_Sprite").c_str());
			m_texts[0] = Text::Loc::Get(SpecialItemLocID(reward.x));
			break;
		default:
			m_found->ChangeSprite(Text::Loc::Sprite("StatFoundText_Sprite").c_str());
			m_items[0]->ChangeSprite(StatSprite(reward.x));
			m_texts[0] = StatText(reward.x, timesCastleBeaten);
			if (reward.type == GetItem::TripStatDrop)
			{
				m_count = 3;
				m_items[1]->ChangeSprite(StatSprite(reward.stat2));
				m_items[2]->ChangeSprite(StatSprite(reward.stat3));
				m_texts[1] = StatText(reward.stat2, timesCastleBeaten);
				m_texts[2] = StatText(reward.stat3, timesCastleBeaten);
			}
			break;
		}
		for (int i = 0; i < PARTICLES; i++)
		{
			m_particleX[i] = 660 + rand() % 201 - 100;
			m_particleY[i] = 360 + rand() % 101 - 50;
			m_particleDelay[i] = (rand() % 1000) / 2000.0f;
		}
		Audio::PlaySound("GetItemBuildupStinger");
	}

	bool GetItemScreen::Update(uint32_t pressed, double dt)
	{
		double before = m_time;
		m_time += dt;
		if (before < 0.9 && m_time >= 0.9) Audio::PlaySound("GetItemStinger3");
		if (m_reward.type == GetItem::Blueprint || m_reward.type == GetItem::Rune)
			m_rotation = m_time < 0.9 ? m_rotation + 1200 * (float) dt : -25;
		if (m_exitTime < 0)
		{
			// UnlockControls: 0.5 s after the item lands
			if (m_time > 1.4 && (pressed & (BTN_A | BTN_B | BTN_X | BTN_Y | BTN_START)))
				m_exitTime = m_time;
			return true;
		}
		return m_time - m_exitTime < 0.45;
	}

	void GetItemScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		const float t = (float) m_time;
		const float land = t - 0.9f;   // ItemSpinAnimation3
		// The exit: everything shrinks back in 0.4 s
		const float out = m_exitTime < 0 ? 1 : 1 - Clamp01((float) (m_time - m_exitTime) / 0.4f);

		// BackBufferOpacity: up to 50% black
		int dim = (int) (256 * (1 - 0.5f * Clamp01(land / 0.5f) * out));
		if (dim < 256)
			for (size_t i = 0; i < (size_t) SCREEN_W * SCREEN_H; i++)
			{
				uint32_t c = frame[i];
				frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * dim >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * dim >> 8) & 0xFF00u);
			}

		if (land >= 0)
		{
			float k = BackOut(land / 0.5f) * out;
			m_bg->Position(Vector2(660, 360 - 170 + 30));
			m_bg->Scale(Vector2(k, k));
			m_bg->Draw(&m_camera);
			for (int i = 0; i < PARTICLES; i++)
			{
				float p = land - m_particleDelay[i];
				if (p < 0 || p > 0.7f) continue;
				float s = 0.1f + 1.9f * Clamp01(p / 0.5f);
				m_particles[i]->Position(Vector2(m_particleX[i], m_particleY[i]));
				m_particles[i]->Scale(Vector2(s, s));
				m_particles[i]->Opacity(Clamp01(p / 0.2f) * (1 - Clamp01((p - 0.5f) / 0.2f)));
				m_particles[i]->Draw(&m_camera);
			}
			float f = BackOut((land - 0.05f) / 0.5f) * out;
			m_found->Position(Vector2(660, 360 - 170));
			m_found->Scale(Vector2(f, f));
			m_found->Draw(&m_camera);
		}

		// The item(s): rise 150 px, shrink, then grow in the middle
		const float offsetX[3] = { 0, 50, -50 }, endX[3] = { 660, 660 + 170, 660 - 170 };
		for (int i = 0; i < m_count; i++)
		{
			SpriteObj *s = m_items[i];
			s->Scale(Vector2(1, 1));
			float h = s->Height() > 0 ? s->Height() : 1;
			float small = 35 / h, big = 130 / h;
			float riseX = m_startX + offsetX[i] * BackOut(t / 0.5f), riseY = m_startY - 150 * BackOut(t / 0.5f);
			float x = riseX, y = riseY, scale = small;
			if (t >= 0.7f && land < 0) scale = small + (0.1f - small) * QuadOut((t - 0.7f) / 0.2f);
			if (land >= 0)
			{
				float a = Clamp01(land / 0.2f);
				scale = 0.1f + (big - 0.1f) * a;
				x = riseX + (endX[i] - riseX) * a;
				y = riseY + (END_Y - riseY) * a;
			}
			scale *= out;
			s->Position(Vector2(x, y));
			s->Scale(Vector2(scale, scale));
			s->Rotation(i == 0 ? m_rotation : 0);
			s->Draw(&m_camera);
		}

		// The names, growing in under the item(s)
		if (land >= 0)
		{
			float k = BackOut(land / 0.3f) * out * 0.36f;
			if (k > 0.01f)
			{
				float y = END_Y + 70 + (m_count == 3 ? 50 : 0);
				m_font.DrawOutlined(frame, m_texts[0].c_str(), SX(660), SY(y), k, WHITE, BLACK, 1, Text::Align::Centre);
				if (m_count == 3)
				{
					m_font.DrawOutlined(frame, m_texts[1].c_str(), SX(660), SY(y + 50), k, WHITE, BLACK, 1, Text::Align::Centre);
					m_font.DrawOutlined(frame, m_texts[2].c_str(), SX(660), SY(END_Y + 70), k, WHITE, BLACK, 1, Text::Align::Centre);
				}
				else if (!m_texts[1].empty())
					m_font.DrawOutlined(frame, m_texts[1].c_str(), SX(660), SY(y + 30), k, WHITE, BLACK, 1, Text::Align::Centre);
			}
		}

		// "[Input:0] Continuer", bottom right, once the controls unlock
		if (land > 0 && out > 0.5f)
		{
			std::string go = Text::Loc::Get("LOC_ID_GET_ITEM_SCREEN_1_NEW");
			size_t at = go.find("[Input:0]");
			if (at != std::string::npos) go.replace(at, 9, "A");
			uint32_t a = (uint32_t) (255 * Clamp01(land / 0.3f));
			m_font.DrawOutlined(frame, go.c_str(), SX(1290), SY(720 - 100), 0.24f, (a << 24) | 0xFFFFFFu, BLACK, 1, Text::Align::Right);
		}
	}
}
