//.h file code:

#include <string>
#include <vector>
#include "stringhelper.h"

#include "IGameNetworkServer.h"
#include "ICustomNetworkClient.h"
#include "GamePlayerState.h"
#include "Vector3.h"
#include "GameNetworkTime.h"


#ifndef _GAME_NETWORK_SERVER_H
#define _GAME_NETWORK_SERVER_H

/// <summary>
/// GameNetworkServer.h
/// 
/// Description:
/// Based on the INTERFACE IGAME NETWORK SERVER, using UNITY NETWORKING to implement the INTERFACE,
/// written in a way in which it could later be rewritten on the WIRELESS ROUTER or BASE STATION with 
/// the same INTERFACE. But with RAJ's RECOIL NETWORKING replacing UNITY NETWORKING, and 'C++' replacing 'C#'.
/// 
/// Created:  20/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class GameNetworkServer : public IGameNetworkServer
{
public:
	GameNetworkServer()
	{
		_localNetworkObject = nullptr;
		_gameState = new GameState();
	}
	virtual ~GameNetworkServer()
	{
		if (_localNetworkObject != nullptr)
		{
			delete _localNetworkObject;
		}

		delete _gameState;
	}

//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [Show] public override float ServerTimestamp
	virtual float getServerTimestamp() const override;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
	static TangibleEvent<IGameNetworkEvent::CreateLobbyFn> * ReceiveCreateLobby;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event CreateLobbyFailedFn ReceiveCreateLobbyFailed;
	static TangibleEvent<IGameNetworkEvent::CreateLobbyFailedFn> * ReceiveCreateLobbyFailed;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event JoinLobbyFn ReceiveJoinLobby;
	static TangibleEvent<IGameNetworkEvent::JoinLobbyFn> * ReceiveJoinLobby;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerConnectFn ReceivePlayerConnect;
	static TangibleEvent<IGameNetworkEvent::PlayerConnectFn> * ReceivePlayerConnect;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event OtherPlayerConnectFn ReceiveOtherPlayerConnect;
	static TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn> * ReceiveOtherPlayerConnect;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerReconnectFn ReceivePlayerReconnect;
static TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn> * ReceivePlayerReconnect;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerDisconnectFn ReceivePlayerDisconnect;
	static TangibleEvent<IGameNetworkEvent::PlayerDisconnectFn> * ReceivePlayerDisconnect;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event SetGameModeFn ReceiveSetGameMode;
	static TangibleEvent<IGameNetworkEvent::SetGameModeFn> * ReceiveSetGameMode;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event ChangeGameModeFn ReceiveChangeGameMode;
	static TangibleEvent<IGameNetworkEvent::ChangeGameModeFn> * ReceiveChangeGameMode;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event ChangeTeamFn ReceiveChangeTeam;
	static TangibleEvent<IGameNetworkEvent::ChangeTeamFn> * ReceiveChangeTeam;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event StartFn ReceiveStart;
	static TangibleEvent<IGameNetworkEvent::StartFn> * ReceiveStart;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event AbortFn ReceiveAbort;
	static TangibleEvent<IGameNetworkEvent::AbortFn> * ReceiveAbort;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerRankChangeFn ReceivePlayerRankChange;
	static TangibleEvent<IGameNetworkEvent::PlayerRankChangeFn> * ReceivePlayerRankChange;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event UpdateGameStateFn ReceiveUpdateGameState;
	static TangibleEvent<IGameNetworkEvent::UpdateGameStateFn> * ReceiveUpdateGameState;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event GoToAmmoFn ReceiveGoToAmmo;
	static TangibleEvent<IGameNetworkEvent::GoToAmmoFn> * ReceiveGoToAmmo;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event GoToBaseFn ReceiveGoToBase;
	static TangibleEvent<IGameNetworkEvent::GoToBaseFn> * ReceiveGoToBase;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event FireFn ReceiveFire;
	static TangibleEvent<IGameNetworkEvent::FireFn> * ReceiveFire;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerWasHitFn ReceivePlayerWasHit;
	static TangibleEvent<IGameNetworkEvent::PlayerWasHitFn> * ReceivePlayerWasHit;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event OtherPlayerWasHitFn ReceiveOtherPlayerWasHit;
	static TangibleEvent<IGameNetworkEvent::OtherPlayerWasHitFn> * ReceiveOtherPlayerWasHit;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event UpdatePlayerStateFn ReceiveUpdatePlayerState;
	static TangibleEvent<IGameNetworkEvent::UpdatePlayerStateFn> * ReceiveUpdatePlayerState;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event UpdateOtherPlayersStateFn ReceiveUpdateOtherPlayersState;
	static TangibleEvent<IGameNetworkEvent::UpdateOtherPlayersStateFn> * ReceiveUpdateOtherPlayersState;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event ReloadClipFn ReceiveReloadClip;
	static TangibleEvent<IGameNetworkEvent::ReloadClipFn> * ReceiveReloadClip;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PickedUpItemFn ReceivePickedUpItem;
	static TangibleEvent<IGameNetworkEvent::PickedUpItemFn> * ReceivePickedUpItem;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event PlayerDeathFn ReceivePlayerDeath;
	static TangibleEvent<IGameNetworkEvent::PlayerDeathFn> * ReceivePlayerDeath;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event OtherPlayerDiedFn ReceiveOtherPlayerDied;
	static TangibleEvent<IGameNetworkEvent::OtherPlayerDiedFn> * ReceiveOtherPlayerDied;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event RespawnPlayerFn ReceiveRespawnPlayer;
	static TangibleEvent<IGameNetworkEvent::RespawnPlayerFn> * ReceiveRespawnPlayer;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event SetBasePointFn ReceiveSetBase;
	static TangibleEvent<IGameNetworkEvent::SetBasePointFn> * ReceiveSetBase;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event SetRespawnPointFn ReceiveSetRespawn;
	static TangibleEvent<IGameNetworkEvent::SetRespawnPointFn> * ReceiveSetRespawn;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event SetAmmoPointFn ReceiveSetAmmo;
	static TangibleEvent<IGameNetworkEvent::SetAmmoPointFn> * ReceiveSetAmmo;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event SetBombPointFn ReceiveSetBomb;
	static TangibleEvent<IGameNetworkEvent::SetBombPointFn> * ReceiveSetBomb;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event GameOverFn ReceiveGameOver;
	static TangibleEvent<IGameNetworkEvent::GameOverFn> * ReceiveGameOver;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event FlagPickedUpFn ReceiveFlagPickedUp;
	static TangibleEvent<IGameNetworkEvent::FlagPickedUpFn> * ReceiveFlagPickedUp;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event FlagDroppedFn ReceiveFlagDropped;
	static TangibleEvent<IGameNetworkEvent::FlagDroppedFn> * ReceiveFlagDropped;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event FlagReturnedFn ReceiveFlagReturned;
	static TangibleEvent<IGameNetworkEvent::FlagReturnedFn> * ReceiveFlagReturned;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event RetryFn ReceiveRetry;
	static TangibleEvent<IGameNetworkEvent::RetryFn> * ReceiveRetry;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewMapFn ReceiveMap;
	static TangibleEvent<IGameNetworkEvent::NewMapFn> * ReceiveMap;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewPlayerProfileFn ReceivePlayerProfile;
	static TangibleEvent<IGameNetworkEvent::NewPlayerProfileFn> * ReceivePlayerProfile;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewVoiceDataFn ReceiveVoiceData;
	static TangibleEvent<IGameNetworkEvent::NewVoiceDataFn> * ReceiveVoiceData;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewExplosionFn ReceiveExplosion;
	static TangibleEvent<IGameNetworkEvent::NewExplosionFn> * ReceiveExplosion;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewSpawnFireFn ReceiveNewSpawnFire;
	static TangibleEvent<IGameNetworkEvent::NewSpawnFireFn> * ReceiveNewSpawnFire;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewSpawnMineFn ReceiveNewSpawnMine;
	static TangibleEvent<IGameNetworkEvent::NewSpawnMineFn> * ReceiveNewSpawnMine;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewSpawnHomingMineFn ReceiveNewSpawnHomingMine;
	static TangibleEvent<IGameNetworkEvent::NewSpawnHomingMineFn> * ReceiveNewSpawnHomingMine;

//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewScrambleMapOnFn ReceiveNewScrambleMapOn;
	static TangibleEvent<IGameNetworkEvent::NewScrambleMapOnFn> * ReceiveNewScrambleMapOn;
//C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
//	public static new event NewScrambleMapOffFn ReceiveNewScrambleMapOff;
	static TangibleEvent<IGameNetworkEvent::NewScrambleMapOffFn> * ReceiveNewScrambleMapOff;

	/// <summary>
	/// The NETWORK OBJECT or GAME OBJECT that controls the communication
	/// of the local GAME SERVER or CLIENT with the computer network.
	/// </summary>
private:
//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [Show] private ICustomNetworkClient _localNetworkObject = null;
	ICustomNetworkClient *_localNetworkObject = nullptr;

//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [Show] private GameState _gameState = new GameState();
	GameState *_gameState = 0;

	/// <summary>
	/// The history of EVENTS that have occurred so far.
	/// This is replayed back to new players that join the game, typically in the LOBBY.
	/// </summary>
//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
//ORIGINAL LINE: [Show] private List<GameEvents> _eventHistory = new List<GameEvents>();
	std::vector<GameEvents*> _eventHistory;

	/// <summary>
	/// Holds the number of GAME CLIENTS at the start of the game.
	/// </summary>
	int _numberOfClientsAtStart = 0;

	/// <summary>
	/// Holds the time the game started.
	/// </summary>
	float _timeAtStart = 0.0f;

	//returns -1 if no player found
	int FindPlayerIndexFromClientID(int clientID);

	bool threadStarted = false;

	void Start();

public:
	ICustomNetworkClient *GetLocalNetworkObject() { return _localNetworkObject; }

	virtual void StartHost() override;

	/// <summary>
	/// Sets up the GAME SERVER if this has not been done.
	/// Gets its NETWORK OBJECT or GAME OBJECT that controls its communication with the computer network.
	/// The first GAME OBJECT connected to the network will the OBJECT for the SERVER.
	/// </summary>
	/// <param name="clientID"></param>
	virtual void Setup(int clientID) override;

private:
	void GameNetworkServer_ReceiveCreateLobby(GameEvents *newEvent);

	void GameNetworkServer_ReceiveJoinLobby(GameEvents *newEvent);

	void GameNetworkServer_ReceivePlayerConnect(GameEvents *newEvent);

	void GameNetworkServer_ReceiveOtherPlayerConnect(GameEvents *newEvent);

	void GameNetworkServer_ReceivePlayerReconnect(GameEvents *newEvent);

	void GameNetworkServer_ReceivePlayerDisconnect(GameEvents *newEvent);

	void GameNetworkServer_ReceiveChangeTeam(GameEvents *newEvent);

	void GameNetworkServer_ReceivePlayerRankChange(GameEvents *newEvent);

	void GameNetworkServer_ReceiveStart(GameEvents *newEvent);

	void GameNetworkServer_ReceiveUpdatePlayerState(GameEvents *newEvent);
	//ReceivePlayerWasHit
	void GameNetworkServer_ReceivePlayerWasHit(GameEvents *newEvent);

	void GameNetworkServer_ReceiveSetRespawnPointEvent(GameEvents *newEvent);

	void GameNetworkServer_ReceiveExplosion(GameEvents *newEvent);


public:
	enum class DamageType
	{
		Bullet,
		Explosion,
		Fire
	};
private:
	int GetDamageFromArmourAmmoCombination(GamePlayerState::ArmourType armourType, GamePlayerState::AmmoType ammoType);

	void ArbitrateAndSendDamage(int targetClient, int sourceClient, DamageType type, int damage = 0);

public:
	void OnEnable();

private:
	void OnDisable();

public:
	virtual void OnStartClient(int newClientID, int oldClientID) override;

	virtual void OnClientDisconnect(int oldClientID) override;

	virtual void CmdServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) override;

	virtual bool ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength) override;

	virtual bool ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) override;

	virtual void ReceiveEvent(GameEvents *newEvent) override;

	virtual void SendEvent(GameEvents *newEvent) override;

	virtual void SendEvent(int clientID, GameEvents *newEvent) override;
	//------------------------------------------------------------------------------------------
	//below is code which affects the actual game (not part of the fundamental networking system)
	void DebugChangeServerState(GameState::State newState);
private:
	bool firstUpdate = true;
public:
	virtual void Update() override;
private:
	void UpdateWaitingForPlayers();
	void InitGameState(GameState::GameMode mode);
public:
	float DataSendInterval = 0.25f;
private:
	float _lastPlayerStateUpdateTime = 0;
	GameNetworkTime networkTime;

public:
	float RespawnZoneRadSq = 25.0f;
	float PickupRadSq = 9.0f;
	float PickupRespawnTime = 30.0f; //30.0f
	float PickupUnspawnTime = 60.0f;
	/// <summary>
	/// The maximum number of kills required to end the GAME MODE.
	/// </summary>
	int RequiredKillCount = 99;
	/// <summary>
	/// The number of GAME CLIENTS connected to the GAME SERVER,
	/// including those CLIENTS that have disconnected but have been
	/// given a limited time to reconnect.
	/// </summary>
	int numberOfClientsConnected = 0;
	/// <summary>
	/// The minimum number of GAME CLIENTS connected to the GAME SERVER,
	/// during the results, before it will automatically restart itself.
	///
	/// NOTE: With UNITY NETWORKING, the GAME SERVER also has a CLIENT on the
	///       network, which it uses to communicate. So the minimum is
	///
	///           1 CLIENT (for the SERVER) or 1
	///
	///       With RECOIL NETWORKING, the GAME SERVER does not have a CLIENT on the
	///       network. So the minimum is
	///
	///           0 CLIENT or 0
	///           
	/// </summary>
	int MIN_CLIENT_NUMBER_RESULTS = 0;

	/// <summary>
	/// The minimum number of GAME CLIENTS connected to the GAME SERVER,
	/// during the player, before it will automatically restart itself.
	/// </summary>
	int MIN_CLIENT_NUMBER_PLAYING = MIN_CLIENT_NUMBER_RESULTS + 1;

    /// <summary>
    /// The default time left (in seconds) for a disconnected CLIENT
    /// to reconnect.
    /// </summary>
	float DEFAULT_TIME_LEFT_TO_RECONNECT = 60.0f;


private:
	/// <summary>
	/// Gets all the players within range of a given point in the GPS SPACE
	/// i.e. 2D space used by the GLOBAL POSITIONING SYSTEM.
	/// </summary>
	/// <param name="gpsSpacePosition"></param>
	/// <param name="radius"></param>
	/// <returns></returns>
	std::vector<int> GetPlayersWithinRange(GameState::HotGenVec3 *gpsSpacePosition, double radius);

	void UpdateDisconnectedPlayers();
	void UpdatePlaying();
	void UpdateResults();
	void UpdateServerRestart();
	void PickupStateChanged(int pickupIndex);
	void RespawnPlayer(int playerIndex);
	void ClearReceiveEventList();
	//above is code which affects the actual game (not part of the fundamental networking system
	//------------------------------------------------------------------------------------------

public:
	virtual void StopHost() override;

	virtual void SendCreateLobby(const std::wstring &serverLobbyOwner, GameState::HotGenVec3 *BaseStationGPSLocation, int gameModeTimeLimit) override;

	virtual void SendCreateLobbyFailed() override;

	virtual void SendJoinLobby(int clientID, const std::wstring &clientPlayerName) override;

	virtual void SendPlayerConnect(int clientID) override;

	virtual void SendOtherPlayerConnect(int clientID, int otherClientID) override;

	virtual void SendPlayerReconnect(int clientID, int clientOldID) override;

	virtual void SendPlayerDisconnect(int clientID, float clientTimeLeftToReconnect) override;

	virtual void SendSetGameMode(GameState::GameMode serverGameMode) override;

	virtual void SendChangeGameMode(GameState::GameMode serverGameMode) override;

	virtual void SendChangeTeam(GamePlayerState::Team clientTeamID) override;

	virtual void SendStart() override;

	virtual void SendAbort() override;

	virtual void SendPlayerRankChange(bool clientIsCaptain) override;

	virtual void SendUpdateServerState(GameState::State newState) override;

	void SendUpdateRespawnPositions();
	void SendUpdatePickupState(int pickupID);
	virtual void SendUpdateGameState(GameState *newState) override;

	virtual void SendGoToAmmo(bool clientHasReachedAmmo) override;

	virtual void SendGoToBase(bool clientHasReachedBase) override;

	virtual void SendFire(GameState::PlayerFire *serverLastPlayerFire) override;

	virtual void SendPlayerWasHit(int clientLastAttackerID) override;
	//not an override..I think thats ok
	void SendPlayerTakeDamage(int targetID, int sourceID, GameState::HotGenVec3 *sourcePosition, int newArmourValue);

	virtual void SendOtherPlayerWasHit(int clientID, int clientLastArmourDamage, int clientLastAttackerID) override;
	virtual void SendUpdatePlayerStateReady(bool value) override;

    virtual void SendUpdatePlayerStatePositionAndHeading(GameState::HotGenVec3 *Pos, float rot) override
    {
    } // not implemented.

	virtual void SendUpdatePlayerState(GamePlayerState newState) override;

	//not inherited..todo, discuss with Rodney
	void SendUpdateOtherPlayerStatePositionAndHeading(int ClientID);
	virtual void SendUpdateOtherPlayersState(int clientID, GamePlayerState newState) override;

	virtual void SendReloadClip() override;

	virtual void SendPickedUpItem(GamePlayerState::PickableItems clientLastPickedUpItem) override;

	virtual void SendPlayerDeath(int clientID, int clientLastAttackerID) override;

	virtual void SendOtherPlayerDied(int clientID, int clientLastAttackerID) override;

	virtual void SendRespawnPlayer(int clientID) override;

	virtual void SendSetBase(GameState::HotGenVec3 *serverLastBasePoint) override;

	virtual void SendSetRespawn(int clientID, GameState::HotGenVec3 *serverLastRespawnPoint) override;

	virtual void SendSetAmmo(Vector3 *serverLastAmmoPoint) override;

	virtual void SendSetBomb(Vector3 *serverLastBombPoint) override;

	virtual void SendGameOver() override;

	virtual void SendFlagPickedUp() override;

	virtual void SendFlagDropped() override;

	virtual void SendFlagReturned() override;

	virtual void SendRetry() override;

	virtual void SendNewMap(GameState::GameMedia *serverMap) override;

	virtual void SendNewPlayerProfile(GameState::GameMedia *serverLastPlayerProfile) override;

	virtual void SendNewVoiceData(GameState::GameMedia *serverVoice) override;

	virtual void SendExplosion(GameState::PlayerExplosion *serverLastExplosion) override;

	virtual void SendNewSpawnFire(GameState::PlayerFire *serverLastPlayerFire) override;

	virtual void SendNewSpawnMine(GameState::PlayerMine *serverLastPlayerMine) override;

	virtual void SendNewSpawnHomingMine(GameState::PlayerHomingMine *serverLastPlayerHomingMine) override;

	virtual void SendNewScrambleMapOn() override;

	virtual void SendNewScrambleMapOff() override;

	int RandomRange(int min, int max) {
		int randNum = rand() % (max - min + 1) + min; return randNum;
	}
};


#endif
