//.h file code:

#include <string>
#include <vector>

class GamePlayerState;

// #include "GamePlayerState.h"
#include "Vector3.h"

#ifndef _GAME_STATE_H
#define _GAME_STATE_H

//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [assembly: InternalsVisibleTo("GameNetworkServer")]

/// <summary>
/// GameState.h
/// 
/// Description:
/// All the data required by the GAME SERVER, on the computer network
/// for RECOIL.
/// 
/// This includes the properties of GAME MODES, RESPAWN POINTS, weapons,
/// armour and other items in the GAME WORLD.
/// 
/// Some of this data is sent across the network in JSON FORMAT. Errors
/// will occur if you do not use data of the right type, during the
/// conversion of the data back from JSON FORMAT, using the UNITY PLUGIN,
/// LIT JSON.
/// 
/// The type of data should be the largest possible for the OPERATING SYSTEM.
/// For example, to store FLOATING POINT NUMBERS, you should use data of type
/// 
///     double
///     
/// 
/// instead of 
/// 
///     float
///     
/// 
/// Or for WHOLE NUMBERS, you should use data of type
/// 
///     long
///     
/// 
/// instead of
/// 
///     int
///     
/// 
/// 
/// Created:  18/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class GameState
{
	friend class GameNetworkServer;

public:
	static constexpr int MAX_REFILL_PICKUPS = 4;
	static constexpr int MAX_SPECIAL_PICKUPS = 4;
	static constexpr int NUM_TEAMS = 2;

	/// <summary>
	/// ID of the different states of the SERVER, or stages of the GAME MODES.
	/// </summary>
public:
	enum class State
	{
		None,
		PreLobby,
		Lobby,
		WaitingPlayers,
		Playing,
		Results,
		ServerRestart,
	};

	/// <summary>
	/// ID of GAME MODES.
	/// </summary>
public:
	enum class GameMode
	{
		None,
		CaptureTheFlag,
		DeathMatch,
	};

	/// <summary>
	/// A utility class for vector operations, needed because we cant use Vector3 on the final hardware
	/// </summary>
public:
	class HotGenVec3
	{
	public:
		double X = 0;
		double Y = 0;
		double Z = 0;
		HotGenVec3();
		HotGenVec3(float x, float y, float z);
		void SetToZero();
		void SetFromVector3(Vector3 *inVec);
		static HotGenVec3 Subtract(HotGenVec3 fromVec, HotGenVec3 subVec); //const
		static HotGenVec3 Add(HotGenVec3 vecA, HotGenVec3 vecB); //const
		Vector3 *ToVector3();
		double LengthSQ();
		double Length();
	};

	/// <summary>
	/// Properties of some audio or image
	/// transferred across the computer network
	/// </summary>
public:
	class GameMedia
	{
	public:
		std::vector<unsigned char> mediaData;
		int mediaSize = 0;
		GameMedia();
	};

	/// <summary>
	/// Properties of each player's results at the end of a GAME MODE
	/// transferred across the computer network
	/// </summary>
public:
	class GameResult
	{
	public:
		std::wstring Name;
		long Kills = 0;
		long Deaths = 0;
		long ClientID = 0;
		GameResult(const std::wstring &name, int kills, int deaths, int clientID);
	};

	/// <summary>
	/// Properties of pending explosions in the GAME WORLD
	/// transferred across the computer network
	/// </summary>
public:
	class PlayerExplosion
	{
	public:
		/// <summary>
		/// The position of the explosion relative to the GAME OBJECT showing the map of the GAME WORLD
		/// </summary>
		HotGenVec3 ObjectSpacePosition;
		/// <summary>
		/// The position relative to the space used by the GPS (Global Positioning System)
		/// </summary>
		HotGenVec3 GpsSpacePosition;
		double Delay = 0;
		double Radius = 0;
		double Damage = 0;
		long AttackerClientID = 0;
		PlayerExplosion();
		PlayerExplosion(HotGenVec3 *objSpacePos, HotGenVec3 *gpsSpacePos, double delay, double radius, double damage, int attackerClientID);
	};

public:
	class PlayerPickupPoint //the location where a pickup can be spawned
	{
	public:
		enum class PickupGenus
		{
			Refill, //used for ammo and armour
			SpecialItem // used fopr special ammo, special armour and gadgets!
		};
	public:
		HotGenVec3 position;
		PickupGenus pickupGenus = static_cast<PickupGenus>(0);
		bool InUse = false;


	};

public:
	class PlayerPickup //the actual pickup that is spawned at a pickup location
	{
	public:
		enum class PickupType
		{
			Ammo,
			Armour,
			Mine,
		};
	public:
		PickupType type = static_cast<PickupType>(0);
		bool PickupAvailable = false;
		float SpawnTime = 0;
		bool Enabled = false;
		PlayerPickupPoint::PickupGenus pickupGenus = static_cast<PlayerPickupPoint::PickupGenus>(0);
		int SpawnPointID = 0;
	};

public:
	std::vector<PlayerPickup::PickupType> SpawnFrequencyTable = {PlayerPickup::PickupType::Ammo, PlayerPickup::PickupType::Armour, PlayerPickup::PickupType::Mine};

	/// <summary>
	/// Properties of fires in the GAME WORLD
	/// transferred across the computer network
	/// </summary>
public:
	class PlayerFire : public PlayerExplosion
	{
	public:
		float duration = 0;
	};

	/// <summary>
	/// Properties of mines in the GAME WORLD
	/// transferred across the computer network
	/// </summary>
public:
	class PlayerMine
	{
	public:
		Vector3 *position = 0;
		float radius = 0;
		float damage = 0;
		int attackerClientID = 0;
		virtual ~PlayerMine()
		{
			if (position != 0)
			{
				delete position;
			}
		}

	};

	/// <summary>
	/// Properties of HOMING MINES in the GAME WORLD
	/// transferred across the computer network
	/// </summary>
public:
	class PlayerHomingMine : public PlayerMine
	{
	public:
		Vector3 *dir=0;
		int targetClientID = 0;
		virtual ~PlayerHomingMine()
		{
			if (dir != 0)
			{
				delete dir;
			}
		}

	};

	/// <summary>
	/// Holds the current state of the GAME SERVER or stage of the GAME MODE.
	/// Whenever this changes, an EVENT should be sent, from the SERVER to
	/// all the GAME CLIENTS. And the properties of that EVENT, sent across
	/// the network, should include the new state.
	/// 
	/// The EVENT need not originate from the SERVER. It can originate from
	/// the GAME CLIENT. And when the SERVER receives it, and changes its
	/// state, it should return it back to all the CLIENTS with its new state.
	/// </summary>
public:
	State serverState = static_cast<State>(0);

	GameMode serverGameMode = static_cast<GameMode>(0);

	long serverGameModeTimeLimit;

	float serverTimestamp = 0;

	GameMedia *serverMap;

	std::wstring serverLobbyOwner;

	GameMedia *serverVoice;

	HotGenVec3 baseStationPosition;

protected:
	PlayerExplosion *serverLastExplosion;

	PlayerFire *serverLastPlayerFire;

	PlayerMine *serverLastPlayerMine;

	PlayerHomingMine *serverLastPlayerHomingMine;

	GameMedia *serverLastPlayerProfile;

	int serverLastCaptainID = 0;

	Vector3 *serverLastBasePoint = 0;

	Vector3 *serverLastRespawnPoint = 0;

	Vector3 *serverLastAmmoPoint = 0;

	Vector3 *serverLastPlantedBombPoint = 0;

	std::vector<PlayerExplosion*> playerExplosions;
	std::vector<time_t> playerExplosionsStartTimes;

	std::vector<PlayerFire*> playerFires;

	std::vector<PlayerMine*> playerMines;

	std::vector<PlayerHomingMine*> playerHomingMines;

	std::vector<GameMedia*> playerProfiles;

	std::vector<GamePlayerState*> players;

	std::vector<int> playerClients;

	std::vector<int> playerCaptainIDs;

	std::vector<int> playerReachedBase;

	std::vector<int> playerReachedRespawn;

	std::vector<int> playerReachedAmmo;

	std::vector<int> playerReachedBomb;

	std::vector<Vector3*> playerBasePoints;

	std::vector<HotGenVec3> playerRespawnPoints;

	std::vector<PlayerPickupPoint> playerPickupPoints; //includes all types of pickup
	std::vector<PlayerPickup> playerPickups; //includes all types of pickup
	std::vector<Vector3*> playerAmmoPoints;

	std::vector<Vector3*> playerBombPoints;

public:
	virtual ~GameState()
	{
		delete serverMap;
		delete serverVoice;
		delete serverLastExplosion;
		delete serverLastPlayerFire;
		delete serverLastPlayerMine;
		delete serverLastPlayerHomingMine;
		delete serverLastPlayerProfile;
		if (serverLastBasePoint)
			delete serverLastBasePoint;
		if (serverLastRespawnPoint)		
			delete serverLastRespawnPoint;
		if (serverLastAmmoPoint)
			delete serverLastAmmoPoint;
		if (serverLastPlantedBombPoint)
			delete serverLastPlantedBombPoint;
	}

	GameState();

	void PlayersRemoveAt(int i)
	{
		players.erase(players.begin() + i);
	}

	void PlayerClientsRemoveAt(int i)
	{
		playerClients.erase(playerClients.begin() + i);
	}

	void PlayerExplosionsRemoveAt(int i)
	{
		playerExplosions.erase(playerExplosions.begin() + i);
		playerExplosionsStartTimes.erase(playerExplosionsStartTimes.begin() + i);
	}
};


#endif
