//.cpp file code:

#include "GameEvents.h"
#include "stringhelper.h"

std::wstring GameEvents::eventDataString[] = {
	//clientPlayerName,
	L"clientPlayerName",
	//clientID,
	L"clientID",
	//clientOldID,
	L"clientOldID",
	//clientTeamID,
	L"clientTeamID",
	//clientTimestamp,
	L"clientTimestamp",
	//clientTimeLeftToReconnect,
	L"clientTimeLeftToReconnect",
	//clientLastAttackerID,
	L"clientLastAttackerID",
	//clientIsDead,
	L"clientIsDead",
	//clientIsLobbyOwner,
	L"clientIsLobbyOwner",
	//clientIsCaptain,
	L"clientIsCaptain",
	//clientDisplayType,
	L"clientDisplayType",
	//clientHasMapScrambled,
	L"clientHasMapScrambled",
	//clientHasReachedBase,
	L"clientHasReachedBase",
	//clientHasReachedAmmo,
	L"clientHasReachedAmmo",
	//clientHasReachedRespawnPoint,
	L"clientHasReachedRespawnPoint",
	//clientHasReachedBomb,
	L"clientHasReachedBomb",
	//clientPosition,
	L"clientPosition",
	//clientHeading,
	L"clientHeading",
	//clientReady,
	L"clientReady",
	//clientLastArmourDamage,
	L"clientLastArmourDamage",
	//clientLastPickedUpItem,
	L"clientLastPickedUpItem",

	//serverState,
	L"serverState",
	//serverGameMode,
	L"serverGameMode",
	//serverGameModeTimeLimit,
	L"serverGameModeTimeLimit",
	//serverTimestamp,
	L"serverTimestamp",
	//serverMap,
	L"serverMap",
	//serverLobbyOwner,
	L"serverLobbyOwner",
	//serverVoice,
	L"serverVoice",
	//serverLastExplosion,
	L"serverLastExplosion",
	//serverLastPlayerFire,
	L"serverLastPlayerFire",
	//serverLastPlayerMine,
	L"serverLastPlayerMine",
	//serverLastPlayerHomingMine,
	L"serverLastPlayerHomingMine",
	//serverLastPlayerProfile,
	L"serverLastPlayerProfile",
	//serverLastCaptainID,
	L"serverLastCaptainID",
	//serverLastBasePoint,
	L"serverLastBasePoint",
	//serverLastRespawnPoint,
	L"serverLastRespawnPoint",
	//serverLastAmmoPoint,
	L"serverLastAmmoPoint",
	//serverLastBombPoint,
	L"serverLastBombPoint",
	//serverBaseStationLocation,
	L"serverBaseStationLocation",

	//serverRespawnPointTeam1,
	L"serverRespawnPointTeam1",
	//serverRespawnPointTeam2,
	L"serverRespawnPointTeam2",

	//PickupPosition,
	L"PickupPosition",
	//PickupID,
	L"PickupID",
	//PickupType,
	L"PickupType",
	//PickupAvailable,
	L"PickupAvailable",

	//DamageSrc,
	L"DamageSrc",
	//SrcPosition,
	L"SrcPosition",
	//GameResults,
	L"GameResults"
};

GameEvents::GameEvents()
{
	ID = eventID::None;
	ParamsVec3 = std::unordered_map<std::wstring, GameState::HotGenVec3>();
	ParamsResult = std::unordered_map<std::wstring, std::vector<GameState::GameResult>>();
	ParamsExplosion = std::unordered_map<std::wstring, GameState::PlayerExplosion>();
	ParamsMedia = std::unordered_map<std::wstring, GameState::GameMedia>();
	ParamsPickupPoint = std::unordered_map<std::wstring, GameState::PlayerPickupPoint>();
	ParamsPickup = std::unordered_map<std::wstring, GameState::PlayerPickup>();
	ParamsFire = std::unordered_map<std::wstring, GameState::PlayerFire>();
	ParamsMine = std::unordered_map<std::wstring, GameState::PlayerMine>();
	ParamsHomingMine = std::unordered_map<std::wstring, GameState::PlayerHomingMine>();
	ParamsLong = std::unordered_map<std::wstring, long>();
	ParamsDouble = std::unordered_map<std::wstring, double>();
	ParamsString = std::unordered_map<std::wstring, std::wstring>();
}

GameEvents::GameEvents(eventID newEventID)
{
	ID = newEventID;
	ParamsVec3 = std::unordered_map<std::wstring, GameState::HotGenVec3>();
	ParamsResult = std::unordered_map<std::wstring, std::vector<GameState::GameResult>>();
	ParamsExplosion = std::unordered_map<std::wstring, GameState::PlayerExplosion>();
	ParamsMedia = std::unordered_map<std::wstring, GameState::GameMedia>();
	ParamsPickupPoint = std::unordered_map<std::wstring, GameState::PlayerPickupPoint>();
	ParamsPickup = std::unordered_map<std::wstring, GameState::PlayerPickup>();
	ParamsFire = std::unordered_map<std::wstring, GameState::PlayerFire>();
	ParamsMine = std::unordered_map<std::wstring, GameState::PlayerMine>();
	ParamsHomingMine = std::unordered_map<std::wstring, GameState::PlayerHomingMine>();
	ParamsLong = std::unordered_map<std::wstring, long>();
	ParamsDouble = std::unordered_map<std::wstring, double>();
	ParamsString = std::unordered_map<std::wstring, std::wstring>();
}

GameEvents::GameEvents(int newClientID, eventID newEventID)
{
	ID = newEventID;
	ParamsVec3 = std::unordered_map<std::wstring, GameState::HotGenVec3>();
	ParamsResult = std::unordered_map<std::wstring, std::vector<GameState::GameResult>>();
	ParamsExplosion = std::unordered_map<std::wstring, GameState::PlayerExplosion>();
	ParamsMedia = std::unordered_map<std::wstring, GameState::GameMedia>();
	ParamsPickupPoint = std::unordered_map<std::wstring, GameState::PlayerPickupPoint>();
	ParamsPickup = std::unordered_map<std::wstring, GameState::PlayerPickup>();
	ParamsFire = std::unordered_map<std::wstring, GameState::PlayerFire>();
	ParamsMine = std::unordered_map<std::wstring, GameState::PlayerMine>();
	ParamsHomingMine = std::unordered_map<std::wstring, GameState::PlayerHomingMine>();
	ParamsLong = std::unordered_map<std::wstring, long>();
	ParamsDouble = std::unordered_map<std::wstring, double>();
	ParamsString = std::unordered_map<std::wstring, std::wstring>();

	//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
//	ParamsLongAdd(StringHelper::toString(GameEvents::eventData::clientID), newClientID);
	//std::pair<std::wstring, long> newEntry(TOSTRING(clientID), newClientID);
	std::pair<std::wstring, long> newEntry(L"clientID", newClientID);
	ParamsLong.insert(newEntry);

}

