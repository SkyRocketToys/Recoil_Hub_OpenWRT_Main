//.cpp file code:

#include "GamePlayerState.h"
#include "Vector3.h"
#include "GameState.h"

GamePlayerState::GamePlayerState()
{
	clientPlayerName = L"";

	clientID = -1;

	clientTeamID = Team::None;

	clientTimestamp = 0.0f;

	clientLastAttackerID = -1;

	clientIsDead = false;

	clientIsLobbyOwner = false;

	clientIsCaptain = false;

	clientIsDisconnected = false;

	clientDisconnectTime = 0.0f;

	clientDisplayType = PlayerDisplayType::Normal;

	clientHasMapScrambled = false;

	clientHasReachedBasePoint = false;

	clientHasReachedRespawnPoint = false;

	clientHasReachedAmmoPoint = false;

	clientHasReachedBombPoint = false;

	clientPosition = GameState::HotGenVec3(0.0f,0.0f,0.0f);

	clientLastArmourDamage = 0;

	clientLastPickedUpItem = PickableItems::None;

	kills = 0;

	deaths = 0;

	ammo = 0;

	clips = 0;

	armour = 100;

	flagsReturned = 0;

	ammoType = AmmoType::None;

	armourType = ArmourType::None;

	specialAmmoCount = 0;

	inventory = std::vector<PickableItems>();

	localMap = new GameState::GameMedia();

	localPlayerProfile = new GameState::GameMedia();

	clientReady = false;

}
