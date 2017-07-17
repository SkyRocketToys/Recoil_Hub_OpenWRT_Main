//.cpp file code:

#include <math.h>
#include "GameState.h"
#include "Vector3.h"


GameState::HotGenVec3::HotGenVec3()
{
	X = 0.0f;
	Y = 0.0f;
	Z = 0.0f;
}

GameState::HotGenVec3::HotGenVec3(float x, float y, float z)
{
	X = x;
	Y = y;
	Z = z;
}

void GameState::HotGenVec3::SetToZero()
{
	X = 0;
	Y = 0;
	Z = 0;
}

GameState::HotGenVec3 GameState::HotGenVec3::Subtract(HotGenVec3 fromVec, HotGenVec3 subVec)
{
	HotGenVec3 newVec = HotGenVec3();
	newVec.X = fromVec.X - subVec.X;
	newVec.Y = fromVec.Y - subVec.Y;
	newVec.Z = fromVec.Z - subVec.Z;
	return newVec;
}

GameState::HotGenVec3 GameState::HotGenVec3::Add(HotGenVec3 vecA, HotGenVec3 vecB)
{
	HotGenVec3 newVec = HotGenVec3();
	newVec.X = vecA.X + vecB.X;
	newVec.Y = vecA.Y + vecB.Y;
	newVec.Z = vecA.Z + vecB.Z;
	return newVec;
}

Vector3 *GameState::HotGenVec3::ToVector3()
{
	Vector3 *newVec = new Vector3(static_cast<float>(X), static_cast<float>(Y), static_cast<float>(Z));
	return newVec;
}

double GameState::HotGenVec3::LengthSQ()
{
	return (X * X) + (Y * Y) + (Z * Z);
}

double GameState::HotGenVec3::Length()
{
	return static_cast<double>( sqrt(static_cast<float>(LengthSQ())) );
}

GameState::GameMedia::GameMedia()
{
	mediaData.clear();
	mediaSize = 0;
}

GameState::GameResult::GameResult(const std::wstring &name, int kills, int deaths, int clientID)
{
	Name = name;
	Kills = kills;
	Deaths = deaths;
	ClientID = clientID;
}

GameState::PlayerExplosion::PlayerExplosion()
{
	Delay = 0;
	Radius = 0;
	Damage = 0;
	AttackerClientID = 0;
}

GameState::PlayerExplosion::PlayerExplosion(HotGenVec3 *objSpacePos, HotGenVec3 *gpsSpacePos, double delay, double radius, double damage, int attackerClientID)
{
	ObjectSpacePosition = *objSpacePos;
	GpsSpacePosition = *gpsSpacePos;
	Delay = delay;
	Radius = radius;
	Damage = damage;
	AttackerClientID = attackerClientID;
}


GameState::GameState()
{

	serverState = State::None;

	serverGameMode = GameMode::None;

	serverGameModeTimeLimit = 0;

	serverTimestamp = 0.0f;

	serverMap = new GameMedia();


	serverLobbyOwner = L"";

	serverVoice = new GameMedia();

	serverLastExplosion = new PlayerExplosion();

	serverLastPlayerFire = new PlayerFire();

	serverLastPlayerMine = new PlayerMine();

	serverLastPlayerHomingMine = new PlayerHomingMine();

	serverLastPlayerProfile = new GameMedia();

	serverLastCaptainID = -1;

	serverLastBasePoint = new Vector3(Vector3::zero);// &Vector3::zero;

	serverLastRespawnPoint = new Vector3(Vector3::zero);//&Vector3::zero;

	serverLastAmmoPoint = new Vector3(Vector3::zero);//&Vector3::zero;

	serverLastPlantedBombPoint = new Vector3(Vector3::zero);//&Vector3::zero;

	playerExplosions = std::vector<PlayerExplosion*>();
	playerExplosionsStartTimes = std::vector<time_t>();

	playerFires = std::vector<PlayerFire*>();

	playerMines = std::vector<PlayerMine*>();

	playerHomingMines = std::vector<PlayerHomingMine*>();

	playerProfiles = std::vector<GameMedia*>();

	players = std::vector<GamePlayerState*>();

	playerClients = std::vector<int>();

	playerCaptainIDs = std::vector<int>();

	playerReachedBase = std::vector<int>();

	playerReachedRespawn = std::vector<int>();

	playerReachedAmmo = std::vector<int>();

	playerReachedBomb = std::vector<int>();

	playerBasePoints = std::vector<Vector3*>();

//	playerRespawnPoints = std::vector<HotGenVec3>(static_cast<int>(GamePlayerState::Team::NumTeams)); //new List<HotGenVec3>();
	playerRespawnPoints = std::vector<HotGenVec3>(static_cast<int>(NUM_TEAMS));

	playerPickupPoints = std::vector<PlayerPickupPoint>(MAX_REFILL_PICKUPS + MAX_SPECIAL_PICKUPS);

	playerPickups = std::vector<PlayerPickup>(MAX_REFILL_PICKUPS + MAX_SPECIAL_PICKUPS);

	playerAmmoPoints = std::vector<Vector3*>();

	playerBombPoints = std::vector<Vector3*>();
}
