// Saved game: everything that lasts between sessions (PlayerStats.cs, the
// family tree, the manor, the blacksmith and runes, the current castle).
//
// The original (SaveGameManager.cs) writes several binary files. This port
// writes one text file of "key value" lines: readable, easy to debug, and
// tolerant of new fields (unknown keys are ignored, missing ones keep their
// default). Writing goes to a temporary file that then replaces the save in
// one rename, keeping the previous save as .bak: switching the console off
// in the middle of a save cannot destroy it.
#pragma once

#include "../Equipment/Equipment.h"
#include "../Lineage/Lineage.h"
#include "../Skills/SkillTree.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Save
{
	// FamilyTreeNode (Types/LineageTypes.cs): one dead ancestor
	struct FamilyTreeNode
	{
		std::string name, romanNumeral;
		uint8_t age = 0, childAge = 0, classType = 0;
		uint8_t headPiece = 1, chestPiece = 1, shoulderPiece = 1;
		int numEnemiesBeaten = 0;
		bool beatenABoss = false, isFemale = false;
		uint8_t trait1 = 0, trait2 = 0;
	};

	// Scalar fields of PlayerStats.cs, kept with their C# names.
	// X(type, name, default)
#define SAVE_PLAYER_FIELDS(X) \
	X(int, Gold, 0) \
	X(int, CurrentHealth, 0) \
	X(int, CurrentMana, 0) \
	X(int, SpecialItem, 0) \
	X(int, DiaryEntry, 0) \
	X(int, BonusHealth, 0) \
	X(int, BonusStrength, 0) \
	X(int, BonusMana, 0) \
	X(int, BonusDefense, 0) \
	X(int, BonusWeight, 0) \
	X(int, BonusMagic, 0) \
	X(int, LichHealth, 0) \
	X(int, LichMana, 0) \
	X(float, LichHealthMod, 1.0f) \
	X(bool, NewBossBeaten, false) \
	X(bool, EyeballBossBeaten, false) \
	X(bool, FairyBossBeaten, false) \
	X(bool, FireballBossBeaten, false) \
	X(bool, BlobBossBeaten, false) \
	X(bool, LastbossBeaten, false) \
	X(int, TimesCastleBeaten, 0) \
	X(int, NumEnemiesBeaten, 0) \
	X(bool, TutorialComplete, false) \
	X(bool, CharacterFound, false) \
	X(bool, LoadStartingRoom, true) \
	X(bool, LockCastle, false) \
	X(bool, SpokeToBlacksmith, false) \
	X(bool, SpokeToEnchantress, false) \
	X(bool, SpokeToArchitect, false) \
	X(bool, SpokeToTollCollector, false) \
	X(bool, IsDead, false) \
	X(bool, FinalDoorOpened, false) \
	X(bool, RerolledChildren, false) \
	X(int, TimesDead, 0) \
	X(bool, HasArchitectFee, false) \
	X(bool, ReadLastDiary, false) \
	X(bool, SpokenToLastBoss, false) \
	X(bool, HardcoreMode, false) \
	X(float, TotalHoursPlayed, 0.0f) \
	X(bool, HasProsopagnosia, false) \
	X(bool, ChallengeEyeballUnlocked, false) \
	X(bool, ChallengeFireballUnlocked, false) \
	X(bool, ChallengeBlobUnlocked, false) \
	X(bool, ChallengeSkullUnlocked, false) \
	X(bool, ChallengeLastBossUnlocked, false) \
	X(bool, ChallengeEyeballBeaten, false) \
	X(bool, ChallengeFireballBeaten, false) \
	X(bool, ChallengeBlobBeaten, false) \
	X(bool, ChallengeSkullBeaten, false) \
	X(bool, ChallengeLastBossBeaten, false) \
	X(int, ChallengeEyeballTimesUpgraded, 0) \
	X(int, ChallengeFireballTimesUpgraded, 0) \
	X(int, ChallengeBlobTimesUpgraded, 0) \
	X(int, ChallengeSkullTimesUpgraded, 0) \
	X(int, ChallengeLastBossTimesUpgraded, 0) \
	X(bool, ArchitectUsed, false)

	constexpr int ENEMY_TYPES = 34;      // EnemyType.Total
	constexpr int ENEMY_DIFFICULTIES = 4; // basic, advanced, expert, miniboss

	struct GameState
	{
#define SAVE_DECLARE(type, name, def) type name = def;
		SAVE_PLAYER_FIELDS(SAVE_DECLARE)
#undef SAVE_DECLARE

		// The current hero (PlayerStats: PlayerName, Class, Spell, Traits,
		// Age, ChildAge, IsFemale, HeadPiece...)
		Lineage::Heir hero;
		// Heirs offered after the last death (PlayerStats.CurrentBranches)
		std::vector<Lineage::Heir> currentBranches;
		// Dead ancestors (PlayerStats.FamilyTreeArray)
		std::vector<FamilyTreeNode> familyTree;
		// Bestiary (PlayerStats.EnemiesKilledList): kills per type and difficulty
		int enemiesKilled[ENEMY_TYPES][ENEMY_DIFFICULTIES] = {};

		// The current castle: regenerated from its seed, plus what was seen
		bool castleExists = false;
		uint32_t castleSeed = 0;
		std::vector<uint8_t> roomsVisited;  // one flag per room of the castle
		std::vector<uint8_t> chestsOpened;  // up to the game: one flag per chest

		Skills::SkillTree skills;
		Equipment::EquipmentSystem equipment;

		// Names of every ancestor (for Lineage::Generator roman numerals)
		std::vector<std::string> FamilyTreeNames() const;
	};

	// Format version written in the file (bump when meanings change)
	constexpr int SAVE_VERSION = 1;

	bool Write(const GameState &state, const char *path);
	// Returns false if neither the file nor its .bak can be read; then
	// 'state' is a new game.
	bool Read(GameState &state, const char *path);
}
