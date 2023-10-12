#include "stdafx.h"
#include <ShellAPI.h>
#include <string>
#include <unordered_map>

static bool testspawn_eventenabled = false;
static bool testspawn_posenabled = false;
static bool testspawn_charenabled = false;
static bool testspawn_levelenabled = false;

static int testspawn_eventid = 0;
static int testspawn_timeofday = TimesOfDay_Day;
static uint8_t testspawn_gamemode = GameModes_Trial;

static FunctionHook<void> LoadCharacter_t(LoadCharacter);

static const std::unordered_map<std::wstring, int16_t> level_name_ids_map = {
	{ L"hedgehoghammer",    LevelIDs_HedgehogHammer },
	{ L"emeraldcoast",      LevelIDs_EmeraldCoast },
	{ L"windyvalley",       LevelIDs_WindyValley },
	{ L"twinklepark",       LevelIDs_TwinklePark },
	{ L"speedhighway",      LevelIDs_SpeedHighway },
	{ L"redmountain",       LevelIDs_RedMountain },
	{ L"skydeck",           LevelIDs_SkyDeck },
	{ L"lostworld",         LevelIDs_LostWorld },
	{ L"icecap",            LevelIDs_IceCap },
	{ L"casinopolis",       LevelIDs_Casinopolis },
	{ L"finalegg",          LevelIDs_FinalEgg },
	{ L"hotshelter",        LevelIDs_HotShelter },
	{ L"chaos0",            LevelIDs_Chaos0 },
	{ L"chaos2",            LevelIDs_Chaos2 },
	{ L"chaos4",            LevelIDs_Chaos4 },
	{ L"chaos6",            LevelIDs_Chaos6 },
	{ L"perfectchaos",      LevelIDs_PerfectChaos },
	{ L"egghornet",         LevelIDs_EggHornet },
	{ L"eggwalker",         LevelIDs_EggWalker },
	{ L"eggviper",          LevelIDs_EggViper },
	{ L"zero",              LevelIDs_Zero },
	{ L"e101",              LevelIDs_E101 },
	{ L"e101r",             LevelIDs_E101R },
	{ L"stationsquare",     LevelIDs_StationSquare },
	{ L"eggcarrieroutside", LevelIDs_EggCarrierOutside },
	{ L"eggcarrierinside",  LevelIDs_EggCarrierInside },
	{ L"mysticruins",       LevelIDs_MysticRuins },
	{ L"past",              LevelIDs_Past },
	{ L"twinklecircuit",    LevelIDs_TwinkleCircuit },
	{ L"skychase1",         LevelIDs_SkyChase1 },
	{ L"skychase2",         LevelIDs_SkyChase2 },
	{ L"sandhill",          LevelIDs_SandHill },
	{ L"ssgarden",          LevelIDs_SSGarden },
	{ L"ecgarden",          LevelIDs_ECGarden },
	{ L"mrgarden",          LevelIDs_MRGarden },
	{ L"chaorace",          LevelIDs_ChaoRace }
};

static int16_t parse_level_id(const std::wstring& str)
{
	std::wstring lowercase = str;
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = level_name_ids_map.find(lowercase);

	if (it != level_name_ids_map.end())
		return it->second;

	return static_cast<int16_t>(std::stol(lowercase));
}

static const std::unordered_map<std::wstring, int16_t> character_name_ids_map = {
	{ L"sonic",      Characters_Sonic },
	{ L"eggman",     Characters_Eggman },
	{ L"tails",      Characters_Tails },
	{ L"knuckles",   Characters_Knuckles },
	{ L"tikal",      Characters_Tikal },
	{ L"amy",        Characters_Amy },
	{ L"gamma",      Characters_Gamma },
	{ L"big",        Characters_Big },
	{ L"metalsonic", Characters_MetalSonic }
};

static int16_t parse_character_id(const std::wstring& str)
{
	std::wstring lowercase = str;
	transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = character_name_ids_map.find(lowercase);

	if (it != character_name_ids_map.end())
		return it->second;

	return static_cast<int16_t>(std::stol(lowercase));
}

static void Obj_Icecap_r(task* o)
{
	if (o)
	{
		SetPlayerInitialPosition(playertwp[0]);
		o->exec = (TaskFuncPtr)Obj_Icecap;
		Obj_Icecap((ObjectMaster*)o);
	}
}

static int ForceTimeOfDay()
{
	return testspawn_timeofday;
}

static const std::unordered_map<std::wstring, int> time_ids_map = {
	{ L"day",     TimesOfDay_Day },
	{ L"evening", TimesOfDay_Evening },
	{ L"night",   TimesOfDay_Night },
};

static int parse_time_id(const std::wstring& str)
{
	std::wstring lowercase = str;
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = time_ids_map.find(lowercase);

	if (it != time_ids_map.end())
		return it->second;

	return std::stol(lowercase);
}

static const std::unordered_map<std::wstring, uint8_t> gamemode_name_map = {
	{ L"adventure",  GameModes_Adventure_Field },
	{ L"trial",     GameModes_Trial },
	{ L"stage",   GameModes_Adventure_ActionStg },
	{ L"mission",    GameModes_Mission },
};

static uint8_t parse_gamemode(const std::wstring& str)
{
	std::wstring lowercase = str;
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::towlower);

	const auto it = gamemode_name_map.find(lowercase);

	if (it != gamemode_name_map.end())
		return it->second;

	return static_cast<uint8_t>(std::stol(lowercase));
}

static void parse_save(const std::wstring str, const HelperFunctions& helperFunctions)
{
	// To normal chars
	std::string str_(str.begin(), str.end());

	// Insert leading zero if needed
	if (str_.size() < 2)
	{
		str_.insert(0, "0");
	}

	// Get path to save file
	std::string savepath = (std::string)helperFunctions.GetMainSavePath() + (std::string)"\\SonicDX" + str_ + (std::string)".snc";

	PrintDebug("Loading save: \"%s\"\n", savepath.c_str());

	FILE* file;

	// Open save file
	if (!fopen_s(&file, savepath.c_str(), "rb"))
	{
		fread_s(&SaveFile, sizeof(SaveFileData), sizeof(SaveFileData), 1, file);
		fclose(file);

		// Prevent save stuff from being overwritten
		WriteData(reinterpret_cast<uint8_t*>(0x425AF0), static_cast<uint8_t>(0xC3u)); // Lives
		WriteData<24>(reinterpret_cast<void*>(0x41330F), static_cast<uint8_t>(0x90u)); // EventFlags
		WriteData<5>(reinterpret_cast<void*>(0x0042D7CC), static_cast<uint8_t>(0x90u)); // CurrentCharacter

		LoadSave();
	}
}

static StartPosition gTestSpawnStartPos;
static FunctionHook<void, taskwk*> SetPlayerInitialPosition_t(SetPlayerInitialPosition);

static void SetPlayerInitialPosition_r(taskwk* twp)
{
	if (!CheckRestartLevel() && CurrentLevel == gTestSpawnStartPos.LevelID && CurrentAct == gTestSpawnStartPos.ActID)
	{
		twp->pos = gTestSpawnStartPos.Position;
		twp->ang.y = 0x4000 - gTestSpawnStartPos.YRot;
	}
	else
	{
		char charID = twp->counter.b[1];

		if (CurrentCharacter == Characters_Eggman || CurrentCharacter == Characters_Tikal || CurrentCharacter > Characters_MetalSonic)
		{
			twp->counter.b[1] = Characters_Sonic; //trick the game to make it thinks we are playing sonic to fix start pos
		}

		SetPlayerInitialPosition_t.Original(twp);

		twp->counter.b[1] = charID; //restore original charID
	}
}

static void TestSpawn_HookPosition(int level, int act, float x, float y, float z, Angle ang)
{
	if (!testspawn_posenabled)
	{
		SetPlayerInitialPosition_t.Hook(SetPlayerInitialPosition_r);
		gTestSpawnStartPos = { (int16_t)level, (int16_t)act, { x, y, z }, ang };
		testspawn_posenabled = true;
	}
}

struct CutsceneLevelData
{
	uint8_t level;
	uint8_t act;
	int character;
	int scene_select; // if -1: no story integration
	int16_t seqno;        // subscene
};

static const std::unordered_map<int, CutsceneLevelData> CutsceneList = {
	{ 0x001, { LevelIDs_StationSquare, 3, Characters_Sonic, 0, 0 } }, // Sonic intro
	{ 0x002, { LevelIDs_Chaos0, 0, Characters_Sonic, 0, 1 } }, // Sonic defeats Chaos 0
	{ 0x003, { LevelIDs_StationSquare, 4, Characters_Sonic, 1, 1 } }, // Sonic sees Tails crash
	{ 0x006, { LevelIDs_StationSquare, 4, Characters_Sonic, 1, 3 } }, // Sonic and Tails poolside
	{ 0x007, { LevelIDs_MysticRuins, 0, Characters_Sonic, 2, 1 } }, // Sonic faces off with the Egg Hornet
	{ 0x008, { LevelIDs_MysticRuins, 0, Characters_Sonic, 2, 3 } }, // Chaos 1 Emerges
	{ 0x009, { LevelIDs_StationSquare, 1, Characters_Sonic, 0, 6 } }, // Sonic and Tails are gassed
	{ 0x00B, { LevelIDs_MysticRuins, 0, Characters_Sonic, 5, 5 } }, // Chaos 4 Transformation
	{ 0x00C, { LevelIDs_MysticRuins, 0, Characters_Sonic, 5, 8 } }, // Sonic and Tails part ways with Knuckles
	{ 0x00D, { LevelIDs_MysticRuins, 0, Characters_Sonic, 5, 10 } }, // Tornado 1 Lift Off Cutscene
	{ 0x011, { LevelIDs_StationSquare, 4, Characters_Sonic, 6, 0 } }, // Sonic falling into Station Square
	{ 0x012, { LevelIDs_StationSquare, 1, Characters_Sonic, 6, 2 } }, // Amy finds Sonic
	{ 0x013, { LevelIDs_StationSquare, 3, Characters_Sonic, 6, 3 } }, // Amy and Sonic go to Twinkle Park
	{ 0x014, { LevelIDs_StationSquare, 5, Characters_Sonic, 7, 0 } }, // Sonic goes looking for Amy
	{ 0x015, { LevelIDs_StationSquare, 1, Characters_Sonic, 8, 0 } }, // Sonic finds Zero and Amy
	{ 0x016, { LevelIDs_MysticRuins, 0, Characters_Sonic, 9, 2 } }, // Zero transported to the Egg Carrier
	{ 0x017, { LevelIDs_RedMountain,  1, Characters_Sonic, 9, 3  } }, // Sonic and Tails on the Tornado 2
	{ 0x01A, { LevelIDs_EggCarrierOutside, 2, Characters_Sonic, 11, 2 } }, // Eggman takes Birdie's Emerald
	{ 0x01B, { LevelIDs_EggCarrierOutside, 2, Characters_Sonic, 11, 5 } }, // Sonic goes to put Eggman out of commission
	{ 0x01C, { LevelIDs_Chaos6, 0, Characters_Sonic, 11, 10 } }, // Sonic finds Chaos 6
	{ 0x01D, { LevelIDs_EggCarrierOutside, 0, Characters_Sonic, 11, 11 } }, // Sonic chases Eggman to the Mystic Ruins
	{ 0x01E, { LevelIDs_MysticRuins, 2, Characters_Sonic, 12, 1 } }, // Sonic prepares to enter Lost World
	{ 0x020, { LevelIDs_LostWorld,  2, Characters_Sonic, 12, 3 } }, // Sonic views the Perfect Chaos Mural
	{ 0x021, { LevelIDs_Past, 2, Characters_Sonic, 13, 0 } }, // Sonic enters the Past
	{ 0x022, { LevelIDs_Past, 2, Characters_Sonic, 13, 2 } }, // Sonic listens to Tikal in the Past
	{ 0x023, { LevelIDs_MysticRuins, 2, Characters_Sonic, 14, 1 } }, // Sonic sees Eggman heading to his base
	{ 0x024, { LevelIDs_EggViper, 0, Characters_Sonic, 14, 3 } }, // Sonic's Final Battle with Eggman
	{ 0x026, { LevelIDs_MysticRuins, 0, Characters_Sonic, 15, 0 } }, // Sonic's Outro
	{ 0x028, { LevelIDs_MysticRuins, 0, Characters_Sonic, 5, 2 } }, // Sonic vs. Knuckles
	{ 0x029, { LevelIDs_EggCarrierOutside, 0, Characters_Sonic, 10, 0 } }, // Tornado 2 lands on the Egg Carrier
	{ 0x02A, { LevelIDs_StationSquare, 1, Characters_Sonic, 4, 0 } }, // Sonic and Tails awaken after being gassed
	{ 0x02B, { LevelIDs_Chaos0, 0, Characters_Sonic, 0, 1 } }, // Sonic meets Chaos 0

	// Tails events
	{ 0x030, { LevelIDs_StationSquare, 3, Characters_Tails, 0, 0 } }, // Tails Intro
	{ 0x031, { LevelIDs_EmeraldCoast,  1, Characters_Tails, 0, 1 } }, // Tails is rescued by Sonic
	{ 0x032, { LevelIDs_StationSquare, 4, Characters_Tails, 0, 2 } }, // Tails and Sonic poolside
	{ 0x033, { LevelIDs_MysticRuins, 0, Characters_Tails, 1, 1 } }, // Tails faces off with Egg Hornet
	{ 0x034, { LevelIDs_MysticRuins, 0, Characters_Tails, 1, 3 } }, // Chaos 1 Emerges
	{ 0x035, { LevelIDs_StationSquare, 1, Characters_Tails, 2, 3 } }, // Tails and Sonic are gassed at Casinopolis
	{ 0x038, { LevelIDs_MysticRuins, 0, Characters_Tails, 4, 2 } }, // Tails vs. Knuckles
	{ 0x039, { LevelIDs_MysticRuins, 0, Characters_Tails, 4, 5 } }, // Chaos 4 Emerges
	{ 0x03A, { LevelIDs_MysticRuins, 0, Characters_Tails, 4, 8 } }, // Tails and Sonic follow Eggman after Chaos 4
	{ 0x03B, { LevelIDs_MysticRuins, 0, Characters_Tails, 4, 10 } }, // Tails and Sonic depart on the Tornado 1
	{ 0x03E, { LevelIDs_MysticRuins, 2, Characters_Tails, 5, 1 } }, // Tails' flashback
	{ 0x040, { LevelIDs_MysticRuins, 0, Characters_Tails, 5, 3 } }, // Tails wakes up from his dream
	{ 0x042, { LevelIDs_MysticRuins, 2, Characters_Tails, 5, 4 } }, // Tails chases Froggy
	{ 0x044, { LevelIDs_Past, 0, Characters_Tails, 6, 0 } }, // Tails enters the Past
	{ 0x045, { LevelIDs_Past, 0, Characters_Tails, 6, 3 } }, // Tails talks to Tikal
	{ 0x046, { LevelIDs_MysticRuins, 0, Characters_Tails, 7, 0 } }, // Tails returns and meets Big
	{ 0x047, { LevelIDs_MysticRuins, 0, Characters_Tails, 7, 2 } }, // The Tornado 2 takes flight
	{ 0x048, { LevelIDs_RedMountain,  1, Characters_Tails, 7, 3 } }, // Tails finds Sonic in Red Mountain
	{ 0x04B, { LevelIDs_EggCarrierOutside, 2, Characters_Tails, 9, 2 } }, // Tails faces off with Gamma
	{ 0x04C, { LevelIDs_EggCarrierOutside, 2, Characters_Tails, 9, 5 } }, // Tails departs the Egg Carrier with Amy
	{ 0x04D, { LevelIDs_StationSquare, 3, Characters_Tails, 10, 0 } }, // Eggman launches his missile attack
	{ 0x04E, { LevelIDs_StationSquare, 3, Characters_Tails, 10, 2 } }, // Tails follows Eggman after the missile
	{ 0x050, { LevelIDs_StationSquare, 1, Characters_Tails, 10, 4 } }, // Tails takes on the Egg Walker
	{ 0x051, { LevelIDs_StationSquare, 1, Characters_Tails, 10, 6 } }, // Egg Walker defeated, Station Square saved
	{ 0x052, { LevelIDs_MysticRuins, 0, Characters_Tails, 11, 0 } }, // Tails Outro
	{ 0x053, { LevelIDs_StationSquare, 0, Characters_Tails, -1, -1 } }, // Error
	{ 0x054, { LevelIDs_EggCarrierOutside, 0, Characters_Tails, 8, 0 } }, // Gonna land on the Egg Carrier
	{ 0x055, { LevelIDs_SandHill, 0, Characters_Tails, 5, 5 } }, // Cutscene with Froggy after Sand Hill
	{ 0x056, { LevelIDs_StationSquare, 1, Characters_Tails, 3, 0 } }, // Tails and Sonic awake after being gassed

	// Amy events
	{ 0x058, { LevelIDs_StationSquare, 0, Characters_Amy, 0, 1 } }, // Amy's pre-intro ??
	{ 0x060, { LevelIDs_StationSquare, 0, Characters_Amy, 0, 0 } }, // Amy's intro
	{ 0x061, { LevelIDs_StationSquare, 0, Characters_Amy, 1, 2 } }, // Amy meets Birdie
	{ 0x062, { LevelIDs_StationSquare, 1, Characters_Amy, 1, 4 } }, // Amy meets up with Sonic
	{ 0x063, { LevelIDs_StationSquare, 3, Characters_Amy, 1, 5 } }, // Amy and Sonic visit Twinkle Park
	{ 0x064, { LevelIDs_StationSquare, 3, Characters_Amy, 1, 8 } }, // Amy's kidnapped by Zero
	{ 0x065, { LevelIDs_EggCarrierInside, 3, Characters_Amy, 2, 1 } }, // Amy locked up, talking to Gamma
	{ 0x066, { LevelIDs_HotShelter, 1, Characters_Amy, 2, 6 } }, // Amy goes to the past after Hot Shelter
	{ 0x067, { LevelIDs_Past, 0, Characters_Amy, 3, 0 } }, // Amy enters the past
	{ 0x068, { LevelIDs_Past, 1, Characters_Amy, 3, 3 } }, // Amy meets Tikal
	{ 0x069, { LevelIDs_EggCarrierOutside, 2, Characters_Amy, 4, 2 } }, // Eggman takes Birdie's Emerald
	{ 0x06A, { LevelIDs_EggCarrierOutside, 2, Characters_Amy, 4, 3 } }, // Amy and Tails leave the Egg Carrier
	{ 0x06B, { LevelIDs_StationSquare, 0, Characters_Amy, -1, -1 } }, // Error
	{ 0x06C, { LevelIDs_EggCarrierOutside, 5, Characters_Amy, 4, 1 } }, // Amy returns to the present
	{ 0x06D, { LevelIDs_StationSquare, 3, Characters_Amy, 5, 0 } }, // Hunt to find Birdie's family
	{ 0x06E, { LevelIDs_MysticRuins, 2, Characters_Amy, 6, 1 } }, // Amy discovers the Egg Base
	{ 0x06F, { LevelIDs_FinalEgg, 0, Characters_Amy, 6, 3 } }, // Amy chased by Zero in Final Egg
	{ 0x070, { LevelIDs_MysticRuins, 3, Characters_Amy, 7, 0 } }, // Amy and Birdie head back to the Egg Carrier
	{ 0x071, { LevelIDs_EggCarrierOutside, 0, Characters_Amy, 8, 1 } }, // Zero confronts Amy
	{ 0x072, { LevelIDs_EggCarrierOutside, 0, Characters_Amy, 8, 4 } }, // Amy's Outro
	{ 0x075, { LevelIDs_StationSquare, 1, Characters_Amy, 1, 10 } }, // Amy's kidnapped to the Mystic Ruins

	// Knuckles events
	{ 0x080, { LevelIDs_MysticRuins, 1, Characters_Knuckles, 0, 0 } }, // Knuckles Intro
	{ 0x082, { LevelIDs_StationSquare, 3, Characters_Knuckles, 1, 0 } }, // Knuckles goes hunting for the Master Emerald
	{ 0x083, { LevelIDs_Casinopolis,  0, Characters_Knuckles, 1, 4 } }, // Knuckles enters the Past (from Casinopolis)
	{ 0x084, { LevelIDs_Past, 0, Characters_Knuckles, 2, 0 } }, // Knuckles while in the Past
	{ 0x085, { LevelIDs_Past, 0, Characters_Knuckles, 2, 2 } }, // Tikal's Crisis
	{ 0x086, { LevelIDs_StationSquare, 1, Characters_Knuckles, 3, 0 } }, // Knuckles returns from the Past
	{ 0x087, { LevelIDs_Chaos2, 0, Characters_Knuckles, 3, 3 } }, // Knuckles and Chaos 2 face off
	{ 0x088, { LevelIDs_Chaos2, 0, Characters_Knuckles, 3, 3 } }, // Eggman tricks Knuckles
	{ 0x089, { LevelIDs_MysticRuins, 0, Characters_Knuckles, 4, 1 } }, // Knuckles goes after Sonic
	{ 0x08A, { LevelIDs_MysticRuins, 0, Characters_Knuckles, 4, 6 } }, // Knuckles vs. Sonic
	{ 0x08B, { LevelIDs_MysticRuins, 0, Characters_Knuckles, 4, 9 } }, // Chaos 4 emerges
	{ 0x08C, { LevelIDs_MysticRuins, 0, Characters_Knuckles, 4, 12 } }, // Knuckles departs from Sonic and Tails
	{ 0x08D, { LevelIDs_LostWorld, 1, Characters_Knuckles, 4, 14 } }, // Knuckles goes to the Past (from Lost World)
	{ 0x08E, { LevelIDs_Past, 1, Characters_Knuckles, 5, 0 } }, // Knuckles back in the Past
	{ 0x08F, { LevelIDs_Past, 1, Characters_Knuckles, 5, 2 } }, // Tikal's crisis again
	{ 0x091, { LevelIDs_MysticRuins, 1, Characters_Knuckles, 6, 1 } }, // Knuckles restores most of the Master Emerald
	{ 0x092, { LevelIDs_MysticRuins, 2, Characters_Knuckles, 6, 4 } }, // Knuckles follows Gamma to Final Egg base
	{ 0x094, { LevelIDs_EggCarrierOutside, 0, Characters_Knuckles, 7, 0 } }, // Knuckles on the Egg Carrier
	{ 0x095, { LevelIDs_SkyDeck, 2, Characters_Knuckles, 7, 7 } }, // Knuckles finds the last missing piece
	{ 0x096, { LevelIDs_Past, 2, Characters_Knuckles, 8, 0 } }, // Knuckles travels back to the past one last time
	{ 0x097, { LevelIDs_Past, 2, Characters_Knuckles, 8, 2 } }, // The aftermath of Tikal's plight
	{ 0x098, { LevelIDs_EggCarrierOutside, 5, Characters_Knuckles, 9, 0 } }, // Knuckles returns to the present
	{ 0x099, { LevelIDs_Chaos6, 1, Characters_Knuckles, 9, 4 } }, // Knuckles fights Chaos 6
	{ 0x09A, { LevelIDs_EggCarrierOutside, 0, Characters_Knuckles, 9, 2 } }, // Knuckles has all collected the final shards
	{ 0x09B, { LevelIDs_Chaos6, 1, Characters_Knuckles, 9, 5 } }, // Knuckles defeats Chaos 6
	{ 0x09C, { LevelIDs_StationSquare, 0, Characters_Knuckles, -1, -1 } }, // Error
	{ 0x09D, { LevelIDs_MysticRuins, 1, Characters_Knuckles, 10, 0 } }, // Knuckles restores the Master Emerald
	{ 0x09F, { LevelIDs_MysticRuins, 1, Characters_Knuckles, 11, 2 } }, // Knuckles Outro
	{ 0x0A0, { LevelIDs_StationSquare, 4, Characters_Knuckles, 3, 2 } }, // Knuckles follows Eggman in Station Square hotel

	// Gamma events
	{ 0x0B0, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 0 } }, // Gamma Intro
	{ 0x0B1, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 2 } }, // Gamma Enters Final Egg
	{ 0x0B2, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 5 } }, // Gamma Exits Final Egg
	{ 0x0B3, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 4 } }, // Useless machine
	{ 0x0B4, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 7 } }, // Gamma's Fight with Beta
	{ 0x0B5, { LevelIDs_MysticRuins, 3, Characters_Gamma, 0, 9 } }, // Gamma defeats Beta
	{ 0x0B7, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 1, 1 } }, // The hunt for Froggy begins
	{ 0x0B8, { LevelIDs_EmeraldCoast, 0, Characters_Gamma, 2, 2 } }, // Gamma goes to the Past
	{ 0x0B9, { LevelIDs_Past, 1, Characters_Gamma, 3, 0 } }, // Gamma in the Past
	{ 0x0BA, { LevelIDs_Past, 1, Characters_Gamma, 3, 2 } }, // Gamma and Tikal meet
	{ 0x0BB, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 0 } }, // Gamma returns to the Egg Carrier
	{ 0x0BC, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 2 } }, // Gamma goes to the wrong room
	{ 0x0BD, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 3 } }, // Beta's new body being built
	{ 0x0BE, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 4 } }, // Gamma leaves Beta's room
	{ 0x0BF, { LevelIDs_EggCarrierInside, 3, Characters_Gamma, 4, 6 } }, // Gamma enters the jail, meets Amy
	{ 0x0C0, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 10 } }, // Gamma heading to the rear of the ship
	{ 0x0C1, { LevelIDs_EggCarrierOutside, 2, Characters_Gamma, 4, 12 } }, // Gamma emerges to fight Sonic
	{ 0x0C2, { LevelIDs_EggCarrierOutside, 2, Characters_Gamma, 4, 15 } }, // Gamma after the battle with Sonic
	{ 0x0C3, { LevelIDs_MysticRuins, 0, Characters_Gamma, 5, 1 } }, // Gamma's objectives changed
	{ 0x0C5, { LevelIDs_MysticRuins, 1, Characters_Gamma, 7, 0 } }, // Gamma remembers his brothers
	{ 0x0C7, { LevelIDs_EggCarrierOutside, 0, 6, 8, Characters_Gamma } }, // Gamma Outro

	// Big events
	{ 0x0D0, { LevelIDs_MysticRuins, 2, Characters_Big, 0, 0 } }, // Big Intro
	{ 0x0D1, { LevelIDs_StationSquare, 3, Characters_Big, 1, 0 } }, // Big goes searching for Froggy
	{ 0x0D2, { LevelIDs_StationSquare, 0, Characters_Big, 1, 2 } }, // Froggy heads into the sewers
	{ 0x0D3, { LevelIDs_MysticRuins, 0, Characters_Big, 2, 3 } }, // Big finds Froggy with Tails
	{ 0x0D4, { LevelIDs_EmeraldCoast,  2, Characters_Big, 3, 2 } }, // Big loses Froggy to Gamma
	{ 0x0D8, { LevelIDs_EggCarrierInside, 1, Characters_Big, 4, 1 } }, // Big enters Hot Shelter
	{ 0x0D9, { LevelIDs_HotShelter, 0, Characters_Big, 4, 2 } }, // Big spots Froggy inside the tanks
	{ 0x0DA, { LevelIDs_HotShelter, 0, Characters_Big, 4, 2 } }, // Big saves Froggy
	{ 0x0DB, { LevelIDs_Past, 1, Characters_Big, 5, 0 } }, // Big heads into the past
	{ 0x0DC, { LevelIDs_Past, 1, Characters_Big, 5, 2 } }, // Tikal talks to Big
	{ 0x0DD, { LevelIDs_EggCarrierInside, 1, Characters_Big, 6, 0 } }, // Big returns and is ready to leave the Egg Carrier
	{ 0x0DE, { LevelIDs_Chaos6, 0, Characters_Big, 6, 4 } }, // Chaos 6 takes Froggy
	{ 0x0DF, { LevelIDs_Chaos6, 0, Characters_Big, 6, 5 } }, // Sonic saves Froggy
	{ 0x0E0, { LevelIDs_EggCarrierOutside, 0, Characters_Big, 6, 7 } }, // Big finds the Tornado 2 and leaves
	{ 0x0E1, { LevelIDs_StationSquare, 0, Characters_Big, -1, -1 } }, // Error
	{ 0x0E2, { LevelIDs_MysticRuins, 2, Characters_Big, 7, 1  } }, // Big Outro
	{ 0x0E3, { LevelIDs_StationSquare, 3, Characters_Big, 3, 1 } }, // Big sees Froggy heading to the beach

	// Last Story
	{ 0x0F0, { LevelIDs_MysticRuins, 2, Characters_Sonic, 0, 0 } }, // Tornado 2 Flash scene
	{ 0x0F2, { LevelIDs_MysticRuins, 2, Characters_Sonic, 1, 2 } }, // Eggman heading to the Mystic Ruins base
	{ 0x0F3, { LevelIDs_MysticRuins, 1, Characters_Sonic, 1, 4 } }, // Knuckles at the Master Emerald
	{ 0x0F4, { LevelIDs_MysticRuins, 0, Characters_Sonic, 1, 6 } }, // Tails runs to Sonic
	{ 0x0F5, { LevelIDs_MysticRuins, 1, Characters_Sonic, 1, 7 } }, // Sonic and Tails find Eggman and Knuckles
	{ 0x0F6, { LevelIDs_Past, 2, Characters_Sonic, 2, 0 } }, // Sonic travels to the past
	{ 0x0F7, { LevelIDs_Past, 2, Characters_Sonic, 2, 1 } }, // Tikal pleads with her father
	{ 0x0F8, { LevelIDs_Past, 2, Characters_Sonic, 2, 4 } }, // Tikal seals Chaos
	{ 0x0F9, { LevelIDs_MysticRuins, 1, Characters_Sonic, 3, 0 } }, // Sonic returns to the present
	{ 0x0FA, { LevelIDs_MysticRuins, 2, Characters_Sonic, 3, 1 } }, // Sonic and Tails find the Tornado 2
	{ 0x0FB, { LevelIDs_Past, 2, Characters_Sonic, 2, 3} }, // Sonic checks on Tikal in the past
	{ 0x0FD, { LevelIDs_PerfectChaos, 0, Characters_Sonic, 3, 2 } }, // Perfect Chaos reveals himself
	{ 0x0FE, { LevelIDs_PerfectChaos, 0, Characters_Sonic, 3, 2 } }, // Last Story Outro
	{ 0x0FF, { LevelIDs_PerfectChaos, 0, Characters_Sonic, 3, 2 } }, // Everyone brings Sonic the emeralds

	// Additional Sonic events
	{ 0x100, { LevelIDs_EggCarrierOutside, 0, Characters_Sonic, 10, 1 } }, // Sonic and Tails after landing on the Egg Carrier
	{ 0x101, { LevelIDs_EggCarrierOutside, 0, Characters_Sonic, 10, 3 } }, // Sonic and Tails Sky Chase attack (don't get too many ideas)
	{ 0x102, { LevelIDs_EggCarrierOutside, 1, Characters_Sonic, 10, 5 } }, // The Egg Carrier Transforms
	{ 0x103, { LevelIDs_EggCarrierOutside, 1, Characters_Sonic, 10, 7 } }, // Sonic at the Sky Deck Entrance
	{ 0x104, { LevelIDs_EggCarrierInside, 1, Characters_Sonic, 11, 0 } }, // Sonic right after Sky Deck (Is that it?)
	{ 0x106, { LevelIDs_EggCarrierOutside, 2, Characters_Sonic, 11, 7 } }, // Sonic heading to transform the Egg Carrier
	{ 0x107, { LevelIDs_EggCarrierOutside, 3, Characters_Sonic, 11, 8 } }, // Emergency altert cancelled (Sonic)

	// Additional Tails events
	{ 0x110, { LevelIDs_EggCarrierOutside, 0, Characters_Tails, 8, 1 } }, // Tails and Sonic after landing on the Egg Carrier
	{ 0x111, { LevelIDs_EggCarrierOutside, 0, Characters_Tails, 8, 3 } }, // Tails' Sky Chase Attack
	{ 0x112, { LevelIDs_EggCarrierOutside, 1, Characters_Tails, 8, 5 } }, // The Egg Carrier Transforms
	{ 0x113, { LevelIDs_EggCarrierOutside, 1, Characters_Tails, 8, 7 } }, // Tails at the Sky Deck Entrance
	{ 0x114, { LevelIDs_EggCarrierInside, 1, Characters_Tails, 9, 0 } }, // Tails right after Sky Deck

	// Additional Knuckles events
	{ 0x120, { LevelIDs_EggCarrierOutside, 2, Characters_Knuckles, 7, 2 } }, // Egg Carrier Transforms 1
	{ 0x121, { LevelIDs_EggCarrierOutside, 2, Characters_Knuckles, 7, 4 } }, // Egg Carrier Transforms 2
	{ 0x122, { LevelIDs_EggCarrierOutside, 5, Characters_Knuckles, 7, 6 } }, // Knuckles sensing the emeralds on the Egg Carrier

	// Additional Amy events
	{ 0x130, { LevelIDs_EggCarrierInside, 2, Characters_Amy, 2, 3 } }, // Introduction to Hedgehog Hammer
	{ 0x131, { LevelIDs_EggCarrierInside, 2, Characters_Amy, 2, 4 } }, // Winning at Hedgehog Hammer

	// Additional Gamma events
	{ 0x140, { LevelIDs_EggCarrierInside, 1, Characters_Gamma, 4, 8 } }, // Gamma is told to find the Jet Booster
	{ 0x141, { LevelIDs_EggCarrierOutside, 0, Characters_Gamma, 8, 0 } }, // Gamma heads to Hot Shelter
	{ 0x142, { LevelIDs_EggCarrierOutside, 0, Characters_Gamma, 8, 3 } }, // Gamma rescues E-105

	// Additional Big events
	{ 0x150, { LevelIDs_EggCarrierOutside, 3, Characters_Big, 6, 1 } }, // Egg Carrier Transforms

	// Additional Last Story events
	{ 0x160, { LevelIDs_Past, 2, Characters_Sonic, 2, 2 } }, // The Echidna tribe faces Chaos

	// Upgrade Cutscenes
	{ 0x165, { LevelIDs_StationSquare, 4, Characters_Sonic, -1, -1 } }, // Sonic gets the Crystal Ring
	{ 0x166, { LevelIDs_StationSquare, 2, Characters_Sonic, 3, 2 } }, // Sonic gets the LSDash Shoe
	{ 0x167, { LevelIDs_MysticRuins, 1, Characters_Sonic, 9, 3 } }, // Sonic gets the Ancient Light
	{ 0x168, { LevelIDs_StationSquare, 3, Characters_Tails, -1, -1 } }, // Tails gets the Jet Anklet
	{ 0x169, { LevelIDs_Past, 0, Characters_Tails, 6, 2 } }, // Tails gets the Rhythm Badge
	{ 0x16A, { LevelIDs_MysticRuins, 2, Characters_Knuckles, -1, -1 } }, // Knuckles gets the Fighting Gloves
	{ 0x16B, { LevelIDs_MysticRuins, 0, Characters_Knuckles, 4, 5 } }, // Knuckles gets the Shovel Claw
	{ 0x16C, { LevelIDs_EggCarrierInside, 2, Characters_Amy, -1, -1 } }, // Amy gets the Long Hammer
	{ 0x16D, { LevelIDs_EggCarrierInside, 2, Characters_Amy, 2, 4 } }, // Amy gets the Warrior Feather
	{ 0x16E, { LevelIDs_EggCarrierInside, 4, Characters_Gamma, -1, -1 } }, // Gamma gets the Laser Blaster
	{ 0x16F, { LevelIDs_EggCarrierInside, 0, Characters_Gamma, 4, 9 } }, // Gamma gets the Jet Booster
	{ 0x170, { LevelIDs_MysticRuins, 2, Characters_Big, -1, -1 } }, // Big gets the PowerRod
	{ 0x171, { LevelIDs_MysticRuins, 1, Characters_Big, 2, 2 } }, // Big gets the Life Belt

	// Misc Events
	{ 0x176, { LevelIDs_StationSquare, 1, Characters_Sonic, 3, 5 } }, // Ice Stone appears (Sonic)
	{ 0x177, { LevelIDs_StationSquare, 1, Characters_Tails, 2, 2 } }, // Ice Stone appears (Tails)
	{ 0x178, { LevelIDs_StationSquare, 3, Characters_Big, 1, 7 } }, // Ice Stone appears (Big)
	{ 0x179, { LevelIDs_StationSquare, 3, Characters_Sonic, 7, 1 } }, // Employee Card appears
	{ 0x17A, { LevelIDs_MysticRuins, 0, Characters_Sonic, 5, 0 /*2,0 for Big*/ } }, // Passage to Angel Island opens (Sonic, Big)
	{ 0x17B, { LevelIDs_MysticRuins, 0, Characters_Tails, 4, 0 } }, // Passage to Angel Island opens (Tails)
	{ 0x17C, { LevelIDs_MysticRuins, 0, Characters_Gamma, 6, 0 } }, // Passage to Angel Island opens (Gamma)
	{ 0x180, { LevelIDs_RedMountain, 0, Characters_Sonic, 9, 3 } } // Egg Carrier in Red Mountain
};

static const CutsceneLevelData* GetCutsceneData(short cutscene)
{
	const auto it = CutsceneList.find(cutscene);

	if (it != CutsceneList.end())
	{
		return &it->second;
	}

	return nullptr;
}

static void SetLevelCleared(int level, int character)
{
	LevelClearCounts[43 * character + level] = 1;
}

static void __cdecl PATCH_EV0166(int n)
{
	EV_InitPlayer(n);
	EV_SetAction(EV_GetPlayer(n), SONIC_ACTIONS[1], &SONIC_TEXLIST, 0.5f, 1, 0); // Standing animation
}

static void __cdecl PATCH_EV00BA(int n)
{
	EV_InitPlayer(n);
	EV_SetAction(EV_GetPlayer(n), E102_ACTIONS[0], &E102_TEXLIST, 0.3f, 1, 0); // Standing animation
}

static void SetEventFlagsForCutscene(int eventID)
{
	switch (eventID)
	{
	case 0x0002: // Sonic defeats Chaos 0
		LevelCutscenes2[2].Cutscene = static_cast<int16_t>(eventID);
		break;
	case 0x0009: // Sonic and Tails gassed
		SetEventFlag((EventFlags)FLAG_SONIC_SS_ENTRANCE_CASINO);
		break;
	case 0x0014: // Sonic goes looking for Amy
		SetLevelCleared(LevelIDs_TwinklePark, Characters_Sonic);
		break;
	case 0x0015: // Sonic finds Zero and Amy
		SetLevelCleared(LevelIDs_SpeedHighway, Characters_Sonic);
		break;
	case 0x0020: // Sonic sees the mural
		WriteData(reinterpret_cast<uint8_t*>(0x7B0DA0), static_cast<uint8_t>(0xC3u)); // Lost World 3 end level object
		WriteData(reinterpret_cast<uint8_t*>(0x5E18B0), static_cast<uint8_t>(0xC3u)); // Level object that plays music
		break;
	case 0x0022: // Sonic listens to Tikal in the Past
		TestSpawn_HookPosition(LevelIDs_Past, 2, -2.77f, -48.86f, 674.9f, 0x9A80);
		break;
	case 0x0023: // Sonic sees Eggman heading to his base
		SetEventFlag((EventFlags)FLAG_SONIC_MR_APPEAR_FINALEGG);
		break;
	case 0x0024: // Egg Viper
		WriteData(reinterpret_cast<uint8_t*>(0x57C4A1), static_cast<uint8_t>(0x2u)); // Go into the wait for cutscene end action.
		break;
	case 0x0026: // Sonic's Outro
		SetLevelCleared(LevelIDs_FinalEgg, Characters_Sonic);
		break;
	case 0x0029: // Sonic and Tails land on the Egg Carrier
		SetEventFlag((EventFlags)FLAG_SONIC_EC_TORNADO2_LOST);
		break;
	case 0x0035: // Tails and Sonic gassed
		SetEventFlag((EventFlags)FLAG_MILES_SS_ENTRANCE_CASINO);
		break;
	case 0x0040: // Tails wakes up from his flashback
	case 0x0042: // Tails chases Froggy
		CutsceneFlagArray[0x003E] = 1;
		break;
	case 0x0050: // Egg Walker
	case 0x0051: // Egg Walker defeated
		SetLevelCleared(LevelIDs_SpeedHighway, Characters_Tails);
		break;
	case 0x0054: // Tails and Sonic land on the Egg Carrier
		SetEventFlag((EventFlags)FLAG_MILES_EC_TORNADO2_LOST);
		break;
	case 0x0055: // Tails saves Froggy in Sand Hill
		WriteData(reinterpret_cast<uint8_t*>(0x598040), static_cast<uint8_t>(0xC3u)); // Osfrog
		WriteData(reinterpret_cast<uint8_t*>(0x79E4C0), static_cast<uint8_t>(0xC3u)); // Plays level music
		WriteData<5>(reinterpret_cast<uint8_t*>(0x597BF3), static_cast<uint8_t>(0x90u)); // Snowboard
		break;
	case 0x006E: // Amy discovers Final Egg base
		SetEventFlag((EventFlags)FLAG_AMY_MR_APPEAR_FINALEGG); // Open Final Egg for Amy
		SetEventFlag((EventFlags)FLAG_AMY_MR_ENTRANCE_FINALEGG); // Open Final Egg for Amy
		break;
	case 0x0070: // Amy and Birdie head back to the Egg Carrier
		SetLevelCleared(LevelIDs_FinalEgg, Characters_Amy);
		break;
	case 0x0071: // Amy confronted by Zero
	case 0x0072: // Amy outro
		SetEventFlag((EventFlags)FLAG_AMY_EC_SINK); // Egg Carrier sunk in Amy's outro
		break;
	case 0x0083: // Knuckles goes to the Past from Casino
		WriteData(reinterpret_cast<uint8_t*>(0x7A1AA0), static_cast<uint8_t>(0xC3u)); // Remove Tikal hints
		WriteData(reinterpret_cast<uint8_t*>(0x476440), static_cast<uint8_t>(0xC3u)); // Remove Radar
		break;
	case 0x0086: // Knuckles returns from the Past to Station Square
		SetEventFlag((EventFlags)FLAG_KNUCKLES_SS_ENTRANCE_CASINO);
		break;
	case 0x0088: // Knuckles is tricked by Eggman
		WriteData(reinterpret_cast<uint8_t*>(0x54DF00), static_cast<uint8_t>(0xC3u)); // Don't load Chaos 2
		LevelCutscenes2[3].Cutscene = static_cast<int16_t>(eventID);
		break;
	case 0x008D: // Knuckles goes to the Past from Lost World
		WriteData<1>(reinterpret_cast<uint8_t*>(0x5E18B0), static_cast<uint8_t>(0xC3u)); // Level object that plays music
		WriteData(reinterpret_cast<uint8_t*>(0x7A1AA0), static_cast<uint8_t>(0xC3u)); // Remove Tikal hints
		WriteData(reinterpret_cast<uint8_t*>(0x476440), static_cast<uint8_t>(0xC3u)); // Remove Radar
		break;
	case 0x0092: // Knuckles follows Gamma to Final Egg
		SetEventFlag((EventFlags)FLAG_KNUCKLES_MR_APPEAR_FINALEGG); // Open Final Egg for Knuckles
		break;
	case 0x0095: // Knuckles finds the last missing piece in Sky Deck
		WriteData<5>(reinterpret_cast<void*>(0x5EF6D0), static_cast<uint8_t>(0x90u)); // Remove Sky Deck music
		WriteData(reinterpret_cast<uint8_t*>(0x450370), static_cast<uint8_t>(0xC3u)); // Remove Rings
		WriteData(reinterpret_cast<uint8_t*>(0x7A1AA0), static_cast<uint8_t>(0xC3u)); // Remove Tikal hints
		WriteData(reinterpret_cast<uint8_t*>(0x476440), static_cast<uint8_t>(0xC3u)); // Remove Radar
		break;
	case 0x009B: // Knuckles defeats Chaos 6
		WriteData(reinterpret_cast<uint8_t*>(0x559FC0), static_cast<uint8_t>(0xC3u)); // Don't load Chaos 6
		LevelCutscenes2[5].Cutscene = static_cast<int16_t>(eventID);
		break;
	case 0x00B3: // Useless machine
		SetEventFlag((EventFlags)FLAG_E102_MR_FREEPASS); // Open Final Egg for useless machine
		SetEventFlag((EventFlags)FLAG_E102_CLEAR_BEACH); // Open Final Egg for useless machine
		SetEventFlag((EventFlags)FLAG_E102_CLEAR_FINALEGG); // Open Final Egg for useless machine
		SetEventFlag((EventFlags)FLAG_E102_MR_APPEAR_FINALEGG); // Open Final Egg for useless machine
		break;
	case 0x00B8: // Gamma goes to the Past
		WriteData(reinterpret_cast<uint8_t*>(0x61CA90), static_cast<uint8_t>(0xC3u)); // Remove Emerald Coast music
		WriteData(reinterpret_cast<uint8_t*>(0x4AD140), static_cast<uint8_t>(0xC3u)); // Remove Kikis
		WriteData(reinterpret_cast<uint8_t*>(0x4FA320), static_cast<uint8_t>(0xC3u)); // Remove OFrog
		break;
	case 0x00C0: // Gamma heading to the rear of the ship
	case 0x00C1: // Gamma vs Sonic
	case 0x00C2: // Gamma after fighting Sonic
	case 0x00C3: // Gamma's objectives changed
		SetEventFlag((EventFlags)FLAG_E102_EC_BOOSTER); // Cutscenes where Gamma appears with the Jet Booster
		break;
	case 0x00BA: // Gamma meets Tikal
		WriteCall(reinterpret_cast<void*>(0x67DD88), PATCH_EV00BA);
		break;
	case 0x00C5: // Gamma remembers his brothers
		SetLevelCleared(LevelIDs_RedMountain, Characters_Gamma);
		SetEventFlag((EventFlags)FLAG_E102_MR_ENTRANCE_MOUNTAIN);
		break;
	case 0x00C7: // Gamma outro
		SetEventFlag((EventFlags)FLAG_E102_EC_SINK);
		break;
	case 0x00D3: // Big finds Froggy with Tails
		SetLevelCleared(LevelIDs_IceCap, Characters_Big);
		break;
	case 0x00D4: // Big loses Froggy to Gamma
		WriteData(reinterpret_cast<uint8_t*>(0x61CA90), static_cast<uint8_t>(0xC3u)); // Remove Emerald Coast music
		break;
	case 0x00DA: // Big saves Froggy in Hot Shelter
		CutsceneFlagArray[217] = 1;
		break;
	case 0x00DF: // Sonic saves Froggy
		LevelCutscenes2[6].Cutscene = static_cast<int16_t>(eventID);
		break;
	case 0x0103: // Sonic at the Sky Deck entrance
	case 0x0113: // Tails at the Sky Deck entrance
		TestSpawn_HookPosition(LevelIDs_EggCarrierOutside, 1, 0.0f, 740.0f, 362.0f, 0);
		break;
	case 0x0122: // Knuckles sensing the emeralds on the Egg Carrier
		TestSpawn_HookPosition(LevelIDs_EggCarrierOutside, 5, 80.0f, -70.0f, 0.0f, 0xC000);
		WriteData(reinterpret_cast<uint8_t*>(0x51DC30), static_cast<uint8_t>(0xC3u)); // Remove pool water
		SetEventFlag((EventFlags)FLAG_KNUCKLES_EC_PALMSWITCH);
		break;
	case 0x0131: // Winning at Hedgehog Hammer
		TestSpawn_HookPosition(LevelIDs_EggCarrierInside, 2, 0.0f, 12.8f, 0.0f, 0x8000);
		break;
	case 0x0140: // Gamma is told to find the Jet Booster
		TestSpawn_HookPosition(LevelIDs_EggCarrierInside, 1, -47.0f, 0.0f, 172.0f, 0x6000);
		break;
	case 0x0141: // Gamma heads to Hot Shelter
	case 0x0142: // Gamma rescues E-105
		SetEventFlag((EventFlags)FLAG_E102_EC_SINK);
		break;
	case 0x0165: // Sonic gets the Crystal Ring
		SetEventFlag((EventFlags)FLAG_SONIC_SS_CRYSTALRING);
		break;
	case 0x0166: // Sonic gets the Light Speed Shoes
		WriteCall(reinterpret_cast<void*>(0x652F5A), PATCH_EV0166);
		SetEventFlag((EventFlags)FLAG_SONIC_SS_LIGHTSHOOSE);
		break;
	case 0x0167: // Sonic gets the Ancient Light
		SetEventFlag((EventFlags)FLAG_SONIC_MR_ANCIENT_LIGHT);
		break;
	case 0x0168: // Tails gets the Jet Anklet
		SetEventFlag((EventFlags)FLAG_MILES_SS_JETANKLET);
		break;
	case 0x0169: // Tails gets the Rhythm Badge
		SetEventFlag((EventFlags)FLAG_MILES_MR_RHYTHMBROOCH);
		break;
	case 0x016A: // Knuckles gets the Fighting Gloves
		SetEventFlag((EventFlags)FLAG_KNUCKLES_MR_FIGHTINGGROVE);
		break;
	case 0x016B: // Knuckles gets the Shovel Claw
		SetEventFlag((EventFlags)FLAG_KNUCKLES_MR_SHOVELCLAW);
		break;
	case 0x016C: // Amy gets the Long Hammer
		SetEventFlag((EventFlags)FLAG_AMY_MR_FIGHTERSFEATHER);
		SetEventFlag((EventFlags)FLAG_AMY_EC_LONGHAMMER);
		break;
	case 0x016D: // Amy gets the Warrior Feather
		SetEventFlag((EventFlags)FLAG_AMY_MR_FIGHTERSFEATHER);
		break;
	case 0x016E: // Gamma gets the Laser Blaster
		TestSpawn_HookPosition(LevelIDs_EggCarrierInside, 4, 35.0f, 56.72f, 0.0f, 0x8000);
		SetEventFlag((EventFlags)FLAG_E102_EC_TYPE3LASER);
		break;
	case 0x016F: // Gamma gets the Jet Booster
		SetEventFlag((EventFlags)FLAG_E102_EC_BOOSTER);
		break;
	case 0x0170: // Big gets the Power Rod
		SetEventFlag((EventFlags)FLAG_BIG_MR_POWERROD);
		break;
	case 0x0171: // Big gets the Life Belt
		SetEventFlag((EventFlags)FLAG_BIG_MR_LIFEBELT);
		break;
	case 0x0177: // Ice Stone appears (Tails)
		SetEventFlag((EventFlags)FLAG_MILES_SS_ICESTONE);
		SetEventFlag((EventFlags)FLAG_MILES_SS_ENTRANCE_CASINO);
		break;
	case 0x017C: // Angel Island opens (Gamma)
		SetLevelCleared(LevelIDs_WindyValley, Characters_Gamma);
		break;
	case 0x00F0: // Super Sonic cutscenes
	case 0x00F2:
	case 0x00F3:
	case 0x00F4:
	case 0x00F5:
	case 0x00F6:
	case 0x00F7:
	case 0x00F8:
	case 0x00F9:
	case 0x00FA:
	case 0x00FB:
	case 0x0160:
		LastStoryFlag = 1;
		break;
	case 0x00FD: // Perfect Chaos level cutscenes
	case 0x00FE:
	case 0x00FF:
		LevelCutscenes2[7].Cutscene = static_cast<int16_t>(eventID);
		LastStoryFlag = 1;
		break;
	}
}

static Trampoline* CheckStandaloneEvent_t = nullptr;

static void __cdecl CustomEventTask(task* tp)
{
	// Wait a few frames for the level to be entirely set up
	int delay = 1;
	switch (testspawn_eventid)
	{
	case 0x006F: // Amy chased by Zero in Final Egg
		delay = 0;
		break;
	case 0x0083: // Knuckles taken to the Past from Casino
		delay = 2;
		break;
	}
	if (++tp->awp->work.sl[0] > delay)
	{
		// Don't load the event if it's already playing, override if another event is playing.
		if (!(EV_MainThread_ptr && CurrentCutsceneID == testspawn_eventid))
		{
			tp->exec = (void(__cdecl*)(task*))0x42CAC0;
			SoundManager_Delete2();
			LoadCutscene(testspawn_eventid);
			LoadEVThread();
		}
		else
		{
			FreeTask(tp);
		}

		// Remove demo/level event hook
		delete CheckStandaloneEvent_t;
	}
}

static signed int __cdecl CheckStandaloneEvent_r()
{
	CreateElementalTask(8, LEV_0, CustomEventTask);
	return 1;
}

static void __cdecl ForceEventMode()
{
	auto data = GetCutsceneData(testspawn_eventid);

	// If we have data for the event
	if (data != nullptr)
	{
		SetupCharacter(testspawn_charenabled ? CurrentCharacter : data->character);
		SetEventFlagsForCutscene(testspawn_eventid);
		if (!testspawn_levelenabled) SetLevelAndAct(data->level, data->act);

		// If the event has story integration
		if (data->scene_select != -1)
		{
			SeqSetPlayer(data->character); // Get story information
			pCurSectionList = &pCurSectionList[data->scene_select]; // Get scene information
			pCurSection = pCurSectionList->psec; // Set current scene manager functions
			pCurSequence->seqno = data->seqno; // Set current subscene of the scene
			pCurSequence->destination = -1; // Force story progression

			// If in adventure field then run the event as a story event
			if (GetLevelType() == 1)
			{
				if (testspawn_eventid != 0x0080) // Exclude Knuckles' intro because it plays twice
					StoryEventMode = 2; // Force story event to play
				StoryEventID = testspawn_eventid;
				GameMode = GameModes_Adventure_Field;
				return;
			}
		}
	}

	// Otherwise run the requested event as a standalone event
	CheckStandaloneEvent_t = new Trampoline(0x413A10, 0x413A15, CheckStandaloneEvent_r); // Hook the demo/level event check
	GameMode = GetLevelType() == 1 ? GameModes_Adventure_Field : GameModes_Adventure_ActionStg;
}

static const auto loc_40C95F = reinterpret_cast<const void*>(0x0040C95F);

__declspec(naked) void ForceEventMode_asm()
{
	__asm
	{
		call ForceEventMode
		jmp loc_40C95F
	}
}

static void DisableMusic()
{
	Music_Enabled = false;
}

static void DisableVoice()
{
	EnableVoice = false;
}

static void DisableSound()
{
	// RET. Prevents the SoundQueue from running.
	WriteData(reinterpret_cast<uint8_t*>(0x004250D0), static_cast<uint8_t>(0xC3u));
}

static void LoadCharacter_r()
{
	if (CurrentCharacter == Characters_Tikal || CurrentCharacter == Characters_Eggman)
	{
		ClearPlayerArrays();
		ObjectMaster* player = nullptr;
		bool isTikal = CurrentCharacter == Characters_Tikal;
		ObjectFuncPtr char_main = isTikal ? Tikal_Main : Eggman_Main;
		player = LoadObject((LoadObj)(LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), 1, char_main);
		player->Data1->CharID = (char)CurrentCharacter;
		player->Data1->CharIndex = 0;
		EntityData1Ptrs[0] = player->Data1;
		EntityData2Ptrs[0] = (EntityData2*)player->Data2;
		MovePlayerToStartPoint(player->Data1);
		return;
	}

	return LoadCharacter_t.Original();
}

void ProcessTestSpawn(const HelperFunctions& helperFunctions)
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	bool GmEdited = false;

	for (int i = 1; i < argc; i++)
	{
		if (!wcscmp(argv[i], L"--movie") || !wcscmp(argv[i], L"-m"))
		{
			WriteData(reinterpret_cast<GameModes*>(0x0040C10C), (GameModes)MD_MOVIE);
			WriteData<6>((char*)0x00413C13, 0x90u); // Prevent movie ID override
			movie_num = _wtoi(argv[++i]);
			PrintDebug("Loading movie: %d\n", movie_num);
			break;
		}
		else if (!wcscmp(argv[i], L"--level") || !wcscmp(argv[i], L"-l"))
		{
			CurrentLevel = parse_level_id(argv[++i]);
			PrintDebug("Loading level: %d\n", CurrentLevel);
			testspawn_levelenabled = true;
		}
		else if (!wcscmp(argv[i], L"--act") || !wcscmp(argv[i], L"-a"))
		{
			CurrentAct = _wtoi(argv[++i]);
			PrintDebug("Loading act: %d\n", CurrentAct);
			testspawn_levelenabled = true;
		}
		else if (!wcscmp(argv[i], L"--character") || !wcscmp(argv[i], L"-c"))
		{
			int16_t character_id = parse_character_id(argv[++i]);

			if (character_id == Characters_MetalSonic)
			{
				MetalSonicFlag = 1;
				character_id = 0;
			}

			if (character_id == Characters_Tikal || character_id == Characters_Eggman || character_id > Characters_MetalSonic)
			{
				LoadCharacter_t.Hook(LoadCharacter_r);
				SetPlayerInitialPosition_t.Hook(SetPlayerInitialPosition_r);
			}

			CurrentCharacter = character_id;

			// NOP. Prevents CurrentCharacter from being overwritten at initialization.
			WriteData<5>(reinterpret_cast<void*>(0x00415007), static_cast<uint8_t>(0x90u));
			//load all characters animations + fix Tikal and Eggman infinite loop
			WriteJump(LoadPlayerMotionData, (void*)0x5034A0);
			PrintDebug("Loading character: %d\n", CurrentCharacter);
			testspawn_charenabled = true;
		}
		else if (!wcscmp(argv[i], L"--time") || !wcscmp(argv[i], L"-t"))
		{
			testspawn_timeofday = parse_time_id(argv[++i]);
			WriteJump(GetTimeOfDay, ForceTimeOfDay);
		}
		else if (!wcscmp(argv[i], L"--gamemode") || !wcscmp(argv[i], L"-g"))
		{
			testspawn_gamemode = parse_gamemode(argv[++i]);
			GmEdited = true;
		}
		else if (!wcscmp(argv[i], L"--save") || !wcscmp(argv[i], L"-s"))
		{
			parse_save(argv[++i], helperFunctions);
		}
		else if (!wcscmp(argv[i], L"--position") || !wcscmp(argv[i], L"-p"))
		{
			if (!testspawn_levelenabled)
			{
				MessageBoxA(nullptr, "Insufficient arguments for parameter: --position.\n"
					"Either --level or --act must be specified before --position.",
					"Insufficient arguments", MB_OK);

				continue;
			}

			if (i + 3 >= argc)
			{
				MessageBoxA(nullptr, "Insufficient arguments for parameter: --position.\n"
					"All 3 components (X, Y, Z) of the spawn position must be provided. Default spawn point will be used.",
					"Insufficient arguments", MB_OK);

				continue;
			}

			// Casinopolis start positions are hardcoded, remove the needed one
			if (CurrentLevel == LevelIDs_Casinopolis)
			{
				switch (CurrentAct)
				{
				case 0:
					WriteData<5>(reinterpret_cast<void*>(0x5C0D67), static_cast<uint8_t>(0x90u));
					break;
				case 1:
					WriteData<5>(reinterpret_cast<void*>(0x5C0E19), static_cast<uint8_t>(0x90u));
					break;
				case 2:
					WriteData<5>(reinterpret_cast<void*>(0x5C0E77), static_cast<uint8_t>(0x90u));
					break;
				case 3:
					WriteData<5>(reinterpret_cast<void*>(0x5C0EF1), static_cast<uint8_t>(0x90u));
					break;
				}
			}
			else if (IsLevelChaoGarden())
			{
				WriteData((uint8_t*)0x715350, static_cast<uint8_t>(0xC3u)); // Remove the chao world start position task
			}

			const float x = std::stof(argv[++i]);
			const float y = std::stof(argv[++i]);
			const float z = std::stof(argv[++i]);

			TestSpawn_HookPosition(CurrentLevel, CurrentAct, x, y, z, 0);
		}
		else if (!wcscmp(argv[i], L"--rotation") || !wcscmp(argv[i], L"-r"))
		{
			if (!testspawn_posenabled)
			{
				MessageBoxA(nullptr, "Insufficient arguments for parameter: --rotation.\n"
					"--position must be specified before --rotation.",
					"Insufficient arguments", MB_OK);

				continue;
			}

			gTestSpawnStartPos.YRot = _wtoi(argv[++i]);
		}
		else if (!wcscmp(argv[i], L"--event") || !wcscmp(argv[i], L"-e"))
		{
			testspawn_eventenabled = true;
			testspawn_eventid = _wtoi(argv[++i]);
			PrintDebug("Loading event: EV%04x (%d)\n", testspawn_eventid, testspawn_eventid);

			// NOP. Prevents story sequence manager to be reset.
			WriteData<5>(reinterpret_cast<void*>(0x413884), static_cast<uint8_t>(0x90u));
		}
		else if (!wcscmp(argv[i], L"--no-music"))
		{
			DisableMusic();
		}
		else if (!wcscmp(argv[i], L"--no-voice"))
		{
			DisableVoice();
		}
		else if (!wcscmp(argv[i], L"--no-sound"))
		{
			DisableSound();
		}
		else if (!wcscmp(argv[i], L"--no-audio"))
		{
			DisableMusic();
			DisableVoice();
			DisableSound();
		}
	}

	if (CurrentLevel == LevelIDs_MysticRuins)
	{
		WriteData<1>((int*)0x52F140, 0xC3); //remove NPC Char load function to fix crash
	}

	if (!GmEdited && (CurrentLevel >= LevelIDs_StationSquare && CurrentLevel <= LevelIDs_Past))
	{
		testspawn_gamemode = GameModes_Adventure_Field;
	}

	if (CurrentLevel == LevelIDs_IceCap)
	{
		WriteData<2>(reinterpret_cast<void*>(0x004149EC), 0x90i8);
		WriteData<2>(reinterpret_cast<void*>(0x0041497F), 0x90i8);
		WriteData<2>(reinterpret_cast<void*>(0x00414A70), 0x90i8);
		RoundMasterList[LevelIDs_IceCap] = Obj_Icecap_r;
	}

	LocalFree(argv);
}

void ApplyTestSpawn()
{
	if (testspawn_eventenabled)
	{
		WriteJump(reinterpret_cast<void*>(0x0040C106), ForceEventMode_asm);
	}
	else if (testspawn_levelenabled)
	{
		WriteData(reinterpret_cast<GameModes*>(0x0040C10C), (GameModes)testspawn_gamemode);
	}
}