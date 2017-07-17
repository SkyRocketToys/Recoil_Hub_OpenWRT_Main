//.h file code:

#include <string>
#include <vector>
#include "GameState.h"

#ifndef _GAME_PLAYER_STATE_H
#define _GAME_PLAYER_STATE_H

//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [assembly: InternalsVisibleTo("GameNetworkServer")]

//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [assembly: InternalsVisibleTo("GameNetworkClient")]

/// <summary>
/// GamePlayerState.h
/// 
/// Description:
/// All the data required about individual players or CLIENTS, on the computer
/// network, for all the GAME MODES of RECOIL.
/// 
/// Created:  18/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class GamePlayerState
{
	friend class GameNetworkServer;

public:
	enum class Team
	{
		None = -1,
		Orange,
		Blue,
		NumTeams
	};

public:
	enum class PickableItems
	{
		None,
		Flag,
		Clips,
		Ammo,
		Armour,
	};

public:
	enum class AmmoType
	{
		Normal,
		None,
	};

public:
	enum class ArmourType
	{
		Normal,
		None,
	};
public:
	enum class PlayerDisplayType
	{
		Normal,
		Dead,
		Invisible,
	};
public:
	std::wstring clientPlayerName;

	int clientID = 0;

	Team clientTeamID = static_cast<Team>(0);

	float clientTimestamp = 0;

	int clientLastAttackerID = 0;

	bool clientIsDead = false;

	bool clientIsLobbyOwner = false;

	bool clientIsCaptain = false;

	bool clientIsDisconnected = false;

	float clientDisconnectTime = 0.0f;

	PlayerDisplayType clientDisplayType = static_cast<PlayerDisplayType>(0);

	bool clientHasMapScrambled = false;

	bool clientHasReachedBasePoint = false;

	bool clientHasReachedRespawnPoint = false;

	bool clientHasReachedAmmoPoint = false;

	bool clientHasReachedBombPoint = false;

	GameState::HotGenVec3 clientPosition;

	float clientHeading = 0;

	GameState::HotGenVec3 clientLastPosition;

	bool clientReady = false;

protected:
	int clientLastArmourDamage = 0;

	PickableItems clientLastPickedUpItem = static_cast<PickableItems>(0);

	int kills = 0;
	int deaths = 0;

	int ammo = 0;

	int clips = 0;

	int armour = 0;

	int flagsReturned = 0;

public:
	AmmoType ammoType = static_cast<AmmoType>(0);

	ArmourType armourType = static_cast<ArmourType>(0);

	int specialAmmoCount = 0;

protected:
	std::vector<PickableItems> inventory;

	GameState::GameMedia *localMap;

	GameState::GameMedia *localPlayerProfile;


public:
	virtual ~GamePlayerState()
	{
		delete localMap;
		delete localPlayerProfile;
	}

	GamePlayerState();
};


#endif
