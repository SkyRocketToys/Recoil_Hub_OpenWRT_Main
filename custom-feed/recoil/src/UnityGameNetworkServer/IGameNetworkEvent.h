//.h file code:

#include <string>
#include "tangible_event.h"
#include "GameEvents.h"
#include "GamePlayerState.h"
#include "Vector3.h"

#ifndef _IGAME_NETWORK_EVENT_H
#define _IGAME_NETWORK_EVENT_H

/// <summary>
/// IGameNetworkEvent.cs
/// 
/// Description:
/// Defines the PROCEDURES or functions of a CLASS that can receive and send EVENTS
/// across a computer network using RECOIL NETWORKING.
/// 
/// When using C#, create a new instance of this using the INSTANTIATE command.
/// 
/// When using C++, create a new instance using the NEW command. But do not inherit this CLASS from MONOBEHAVIOUR.
/// 
/// Created:  18/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class IGameNetworkEvent 
{
	/// <summary>
	/// Receives an EVENT from the computer network.
	/// </summary>
	/// <param name="newEvent"></param>
public:
	virtual void ReceiveEvent(GameEvents *newEvent) = 0;

	/// <summary>
	/// Sends an EVENT to the computer network.
	/// </summary>
	/// <param name="newEvent"></param>
	virtual void SendEvent(GameEvents *newEvent) = 0;
	virtual void SendEvent(int clientID, GameEvents *newEvent) = 0;

	/// <summary>
	/// The following are functions that receive EVENTS that occur in the LOBBY.
	/// 
	/// For each EVENT, there is a template e.g.
	/// 
	///     CreateLobbyFn(GameEvents newEvent)
	///     
	/// 
	/// which responds to an EVENT e.g.
	/// 
	///     GameEvents.eventID.CreateLobbyEvent
	///     
	/// 
	/// and a list of functions that implement the template e.g.
	/// 
	///     ReceiveCreateLobby
	///     
	/// 
	/// All the functions in that list will be executed when that EVENT occurs.
	/// You can add as many functions as you want to that list e.g.
	/// 
	///     GameNetworkClient.ReceiveCreateLobby += SomeCreateLobbyFunc;
	///     
	/// 
	/// And you can execute all the functions in the list, passing all the parameters
	/// with one command e.g.
	/// 
	///     GameNetworkClient.ReceiveCreateLobby(newEvent);
	///  
	/// </summary>
	using CreateLobbyFn = std::function<void (GameEvents *newEvent)>;

	using CreateLobbyFailedFn = std::function<void (GameEvents *newEvent)>;

	using JoinLobbyFn = std::function<void (GameEvents *newEvent)>;

	using PlayerConnectFn = std::function<void (GameEvents *newEvent)>;

	using OtherPlayerConnectFn = std::function<void (GameEvents *newEvent)>;

	using PlayerReconnectFn = std::function<void(GameEvents *newEvent)>;

	using PlayerDisconnectFn = std::function<void (GameEvents *newEvent)>;

	using SetGameModeFn = std::function<void (GameEvents *newEvent)>;
	using ChangeGameModeFn = std::function<void (GameEvents *newEvent)>;
	using ChangeTeamFn = std::function<void (GameEvents *newEvent)>;
	using StartFn = std::function<void (GameEvents *newEvent)>;
	using AbortFn = std::function<void (GameEvents *newEvent)>;
	using PlayerRankChangeFn = std::function<void (GameEvents *newEvent)>;

	/// <summary>
	/// The following are functions that receive EVENTS during a GAME MODE
	/// </summary>
	using UpdateGameStateFn = std::function<void (GameEvents *newEvent)>;
	using GoToAmmoFn = std::function<void (GameEvents *newEvent)>;
	using GoToBaseFn = std::function<void (GameEvents *newEvent)>;
	using FireFn = std::function<void (GameEvents *newEvent)>;
	using PlayerWasHitFn = std::function<void (GameEvents *newEvent)>;
	using OtherPlayerWasHitFn = std::function<void (GameEvents *newEvent)>;

	using UpdatePlayerStateFn = std::function<void (GameEvents *newEvent)>;
	using UpdateOtherPlayersStateFn = std::function<void (GameEvents *newEvent)>;

	using ReloadClipFn = std::function<void (GameEvents *newEvent)>;
	using PickedUpItemFn = std::function<void (GameEvents *newEvent)>;

	using PlayerDeathFn = std::function<void (GameEvents *newEvent)>;
	using OtherPlayerDiedFn = std::function<void (GameEvents *newEvent)>;
	using RespawnPlayerFn = std::function<void (GameEvents *newEvent)>;
	using SetBasePointFn = std::function<void (GameEvents *newEvent)>;
	using SetRespawnPointFn = std::function<void (GameEvents *newEvent)>;
	using SetAmmoPointFn = std::function<void (GameEvents *newEvent)>;
	using SetBombPointFn = std::function<void (GameEvents *newEvent)>;
	using GameOverFn = std::function<void (GameEvents *newEvent)>;

	using FlagPickedUpFn = std::function<void (GameEvents *newEvent)>;
	using FlagDroppedFn = std::function<void (GameEvents *newEvent)>;
	using FlagReturnedFn = std::function<void (GameEvents *newEvent)>;

	/// <summary>
	/// The following are functions that receives EVENTS during the results.
	/// </summary>
	using RetryFn = std::function<void (GameEvents *newEvent)>;

	/// <summary>
	/// The following are functions that receive EVENTS for features not yet implemented
	/// </summary>
	using NewMapFn = std::function<void (GameEvents *newEvent)>;
	using NewPlayerProfileFn = std::function<void (GameEvents *newEvent)>;
	using NewVoiceDataFn = std::function<void (GameEvents *newEvent)>;
	using NewExplosionFn = std::function<void (GameEvents *newEvent)>;

	using NewSpawnFireFn = std::function<void (GameEvents *newEvent)>;

	using NewSpawnMineFn = std::function<void (GameEvents *newEvent)>;
	using NewSpawnHomingMineFn = std::function<void (GameEvents *newEvent)>;

	using NewScrambleMapOnFn = std::function<void (GameEvents *newEvent)>;
	using NewScrambleMapOffFn = std::function<void (GameEvents *newEvent)>;

	/// <summary>
	/// The following are functions that send EVENTS and the data that accompanies each EVENT during the LOBBY.
	/// </summary>
	virtual void SendCreateLobby(const std::wstring &serverLobbyOwner, GameState::HotGenVec3 *BaseStationGPSLocation, int gameModeTimeLimit) = 0;
	virtual void SendCreateLobbyFailed() = 0;
	virtual void SendJoinLobby(int clientID, const std::wstring &clientPlayerName) = 0;
	virtual void SendPlayerConnect(int clientID) = 0;
	virtual void SendOtherPlayerConnect(int clientID, int otherClientID) = 0;
	virtual void SendPlayerReconnect(int clientID, int clientOldID) = 0;
	virtual void SendPlayerDisconnect(int clientID, float clientTimeLeftToReconnect) = 0;
	virtual void SendSetGameMode(GameState::GameMode serverGameMode) = 0;
	virtual void SendChangeGameMode(GameState::GameMode serverGameMode) = 0;
	virtual void SendChangeTeam(GamePlayerState::Team clientTeamID) = 0;
	virtual void SendStart() = 0;
	virtual void SendAbort() = 0;

	/// <summary>
	/// The following are functions that send EVENTS and the data that accompanies each EVENT during a GAME MODE.
	/// </summary>
	virtual void SendPlayerRankChange(bool clientIsCaptain) = 0;
	virtual void SendUpdateServerState(GameState::State newState) = 0;
	virtual void SendUpdateGameState(GameState *newState) = 0;
	virtual void SendGoToAmmo(bool clientHasReachedAmmo) = 0;
	virtual void SendGoToBase(bool clientHasReachedBase) = 0;
	virtual void SendFire(GameState::PlayerFire *serverLastPlayerFire) = 0;
	virtual void SendPlayerWasHit(int clientLastAttackerID) = 0;
	virtual void SendOtherPlayerWasHit(int clientID, int clientLastArmourDamage, int clientLastAttackerID) = 0;
	virtual void SendUpdatePlayerStateReady(bool value) = 0;

	virtual void SendUpdatePlayerStatePositionAndHeading(GameState::HotGenVec3 *Pos, float rot) = 0;


	virtual void SendUpdatePlayerState(GamePlayerState newState) = 0;
	virtual void SendUpdateOtherPlayersState(int clientID, GamePlayerState newState) = 0;
	virtual void SendReloadClip() = 0;
	virtual void SendPickedUpItem(GamePlayerState::PickableItems clientLastPickedUpItem) = 0;
	virtual void SendPlayerDeath(int clientID, int clientLastAttackerID) = 0;
	virtual void SendOtherPlayerDied(int clientID, int clientLastAttackerID) = 0;
	virtual void SendRespawnPlayer(int clientID) = 0;
	virtual void SendSetBase(GameState::HotGenVec3 *serverLastBasePoint) = 0;
	virtual void SendSetRespawn(int clientID, GameState::HotGenVec3 *serverLastRespawnPoint) = 0;
	virtual void SendSetAmmo(Vector3 *serverLastAmmoPoint) = 0;
	virtual void SendSetBomb(Vector3 *serverLastBombPoint) = 0;
	virtual void SendGameOver() = 0;
	virtual void SendFlagPickedUp() = 0;
	virtual void SendFlagDropped() = 0;
	virtual void SendFlagReturned() = 0;

	/// <summary>
	/// The following are functions that send EVENTS and the data that accompanies each EVENT during the results.
	/// </summary>
	virtual void SendRetry() = 0;

	/// <summary>
	/// The following are functions that send EVENTS and the data that accompanies each EVENT for features not yet implemented.
	/// </summary>
	virtual void SendNewMap(GameState::GameMedia *serverMap) = 0;
	virtual void SendNewPlayerProfile(GameState::GameMedia *serverLastPlayerProfile) = 0;
	virtual void SendNewVoiceData(GameState::GameMedia *serverVoice) = 0;
	virtual void SendExplosion(GameState::PlayerExplosion *serverLastExplosion) = 0;
	virtual void SendNewSpawnFire(GameState::PlayerFire *serverLastPlayerFire) = 0;
	virtual void SendNewSpawnMine(GameState::PlayerMine *serverLastPlayerMine) = 0;
	virtual void SendNewSpawnHomingMine(GameState::PlayerHomingMine *serverLastPlayerHomingMine) = 0;
	virtual void SendNewScrambleMapOn() = 0;
	virtual void SendNewScrambleMapOff() = 0;

};

#endif
