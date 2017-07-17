//.h file code:

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#ifndef _GAME_EVENTS_H
#define _GAME_EVENTS_H

#include "GameState.h"

#define PARAMSVEC3_ITERATOR std::unordered_map<std::wstring, GameState::HotGenVec3>::const_iterator
#define PARAMSRESULT_ITERATOR std::unordered_map<std::wstring, std::vector<GameState::GameResult>>::const_iterator
#define PARAMSEXPLOSION_ITERATOR std::unordered_map<std::wstring, GameState::PlayerExplosion>::const_iterator
#define PARAMSMEDIA_ITERATOR std::unordered_map<std::wstring, GameState::GameMedia>::const_iterator
#define PARAMSPICKUPPOINT_ITERATOR std::unordered_map<std::wstring, GameState::PlayerPickupPoint>::const_iterator
#define PARAMSPICKUP_ITERATOR std::unordered_map<std::wstring, GameState::PlayerPickup>::const_iterator
#define PARAMSFIRE_ITERATOR std::unordered_map<std::wstring, GameState::PlayerFire>::const_iterator
#define PARAMSMINE_ITERATOR std::unordered_map<std::wstring, GameState::PlayerMine>::const_iterator
#define PARAMSHOMINGMINE_ITERATOR std::unordered_map<std::wstring, GameState::PlayerHomingMine>::const_iterator
#define PARAMSLONG_ITERATOR std::unordered_map<std::wstring, long>::const_iterator
#define PARAMSDOUBLE_ITERATOR std::unordered_map<std::wstring, double>::const_iterator
#define PARAMSSTRING_ITERATOR std::unordered_map<std::wstring, std::wstring>::const_iterator

/// <summary>
/// GameEvents.h
/// 
/// Description:
/// The ID of EVENTS and the type of data (a DICTIONARY or HASH TABLE) that
/// should accompany each EVENT sent across the computer network. The contents
/// of the DICTIONARY would change depending on the EVENT that was sent. It
/// would contain either some or all of the data about an individual player
/// or GAME CLIENT, held in the CLASS GAME PLAYER STATE. Or it would contain
/// some or all of the data about the curreht GAME MODE on the GAME SERVER
/// held in the CLASS GAME STATE.
/// 
/// Created:  20/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class GameEvents
{

	/// <summary>
	/// The ID of EVENTS that could be sent across the computer network.
	/// </summary>
public:
	enum class eventID
	{
		None,

		/// <summary>
		/// The following EVENTS will be used in the LOBBY
		/// </summary>
		CreateLobbyEvent,
		CreateLobbyFailedEvent,
		JoinLobbyEvent,
		PlayerConnectEvent,
		OtherPlayerConnectEvent,
		PlayerReconnectEvent,
		PlayerDisconnectEvent,
		SetGameModeEvent,
		ChangeGameModeEvent,
		ChangeTeamEvent,
		StartEvent,
		AbortEvent,
		PlayerRankChangeEvent,

		/// <summary>
		/// The following EVENTS will be used in GAME MODES
		/// </summary>
		UpdateGameStateEvent,
		GoToAmmoEvent,
		GoToBaseEvent,
		FireEvent,
		PlayerWasHitEvent,
		OtherPlayerWasHitEvent,

		UpdatePlayerStateEvent,
		UpdateOtherPlayersStateEvent,

		ReloadClipEvent,
		PickedUpItemEvent,

		PlayerDeathEvent,
		OtherPlayerDiedEvent,
		RespawnPlayerEvent,
		SetBasePointEvent,
		SetRespawnPointEvent,
		SetAmmoPointEvent,
		SetBombPointEvent,
		GameOverEvent,

		FlagPickedUpEvent,
		FlagDroppedEvent,
		FlagReturnedEvent,


		/// <summary>
		/// The following EVENTS will be used in RESULTS
		/// </summary>
		RetryEvent,

		/// <summary>
		/// The following EVENTS will be used in the future.
		/// </summary>
		NewMapEvent,
		NewPlayerProfileEvent,
		NewVoiceDataEvent,
		NewExplosionEvent,

		NewSpawnFireEvent,
		NewSpawnMineEvent,
		NewSpawnHomingMineEvent,

		NewScrambleMapOnEvent,
		NewScrambleMapOffEvent

	};

	/// <summary>
	/// The ID of data in the DICTIONARY that should accompany each EVENT sent across the computer network.
	/// </summary>
public:
	enum class eventData
	{
		clientPlayerName,
		clientID,
		clientOldID,
		clientTeamID,
		clientTimestamp,
		clientTimeLeftToReconnect,
		clientLastAttackerID,
		clientIsDead,
		clientIsLobbyOwner,
		clientIsCaptain,
		clientDisplayType,
		clientHasMapScrambled,
		clientHasReachedBase,
		clientHasReachedAmmo,
		clientHasReachedRespawnPoint,
		clientHasReachedBomb,
		clientPosition,
		clientHeading,
		clientReady,
		clientLastArmourDamage,
		clientLastPickedUpItem,

		serverState,
		serverGameMode,
		serverGameModeTimeLimit,
		serverTimestamp,
		serverMap,
		serverLobbyOwner,
		serverVoice,
		serverLastExplosion,
		serverLastPlayerFire,
		serverLastPlayerMine,
		serverLastPlayerHomingMine,
		serverLastPlayerProfile,
		serverLastCaptainID,
		serverLastBasePoint,
		serverLastRespawnPoint,
		serverLastAmmoPoint,
		serverLastBombPoint,
		serverBaseStationLocation,

		serverRespawnPointTeam1,
		serverRespawnPointTeam2,

		PickupPosition,
		PickupID,
		PickupType,
		PickupAvailable,
		//below are event data types that do not have a corresponding game state variable
		DamageSrc,
		SrcPosition,
		GameResults,


	};

	static std::wstring eventDataString[];

public:
	eventID ID = static_cast<eventID>(0);

	std::unordered_map<std::wstring, GameState::HotGenVec3> ParamsVec3;
	std::unordered_map<std::wstring, std::vector<GameState::GameResult>> ParamsResult;
	std::unordered_map<std::wstring, GameState::PlayerExplosion> ParamsExplosion;
	std::unordered_map<std::wstring, GameState::GameMedia> ParamsMedia;
	std::unordered_map<std::wstring, GameState::PlayerPickupPoint> ParamsPickupPoint;
	std::unordered_map<std::wstring, GameState::PlayerPickup> ParamsPickup;
	std::unordered_map<std::wstring, GameState::PlayerFire> ParamsFire;
	std::unordered_map<std::wstring, GameState::PlayerMine> ParamsMine;
	std::unordered_map<std::wstring, GameState::PlayerHomingMine> ParamsHomingMine;
	std::unordered_map<std::wstring, long> ParamsLong;
	std::unordered_map<std::wstring, double> ParamsDouble;
	std::unordered_map<std::wstring, std::wstring> ParamsString;

	GameEvents();

	GameEvents(eventID newEventID);

	GameEvents(int newClientID, eventID newEventID);

	bool ParamsVec3ContainsKey(std::wstring key) { PARAMSVEC3_ITERATOR got = ParamsVec3.find(key); return got != ParamsVec3.end(); }
	bool ParamsResultContainsKey(std::wstring key) { PARAMSRESULT_ITERATOR got = ParamsResult.find(key); return got != ParamsResult.end(); }
	bool ParamsExplosionContainsKey(std::wstring key) { PARAMSEXPLOSION_ITERATOR got = ParamsExplosion.find(key); return got != ParamsExplosion.end(); }
	bool ParamsMediaContainsKey(std::wstring key) { PARAMSMEDIA_ITERATOR got = ParamsMedia.find(key); return got != ParamsMedia.end(); }
	bool ParamsPickupPointContainsKey(std::wstring key) { PARAMSPICKUPPOINT_ITERATOR got = ParamsPickupPoint.find(key); return got != ParamsPickupPoint.end(); }
	bool ParamsPickupContainsKey(std::wstring key) { PARAMSPICKUP_ITERATOR got = ParamsPickup.find(key); return got != ParamsPickup.end(); }
	bool ParamsFireContainsKey(std::wstring key) { PARAMSFIRE_ITERATOR got = ParamsFire.find(key); return got != ParamsFire.end(); }
	bool ParamsMineContainsKey(std::wstring key) { PARAMSMINE_ITERATOR got = ParamsMine.find(key); return got != ParamsMine.end(); }
	bool ParamsHomingMineContainsKey(std::wstring key) { PARAMSHOMINGMINE_ITERATOR got = ParamsHomingMine.find(key); return got != ParamsHomingMine.end(); }
	bool ParamsLongContainsKey(std::wstring key) { PARAMSLONG_ITERATOR got = ParamsLong.find(key); return got != ParamsLong.end(); }
	bool ParamsDoubleContainsKey(std::wstring key) { PARAMSDOUBLE_ITERATOR got = ParamsDouble.find(key); return got != ParamsDouble.end(); }
	bool ParamsStringContainsKey(std::wstring key) { PARAMSSTRING_ITERATOR got = ParamsString.find(key); return got != ParamsString.end(); }

	void ParamsVec3Add(std::wstring key, GameState::HotGenVec3 value) { std::pair<std::wstring, GameState::HotGenVec3> newEntry(key, value); ParamsVec3.insert(newEntry); }
	void ParamsResultAdd(std::wstring key, std::vector<GameState::GameResult> value) { std::pair<std::wstring, std::vector<GameState::GameResult>> newEntry(key, value); ParamsResult.insert(newEntry); }
	void ParamsExplosionAdd(std::wstring key, GameState::PlayerExplosion value) { std::pair<std::wstring, GameState::PlayerExplosion> newEntry(key, value); ParamsExplosion.insert(newEntry); }
	void ParamsMediaAdd(std::wstring key, GameState::GameMedia value) { std::pair<std::wstring, GameState::GameMedia> newEntry(key, value); ParamsMedia.insert(newEntry); }
	void ParamsPickupPointAdd(std::wstring key, GameState::PlayerPickupPoint value) { std::pair<std::wstring, GameState::PlayerPickupPoint> newEntry(key, value); ParamsPickupPoint.insert(newEntry); }
	void ParamsPickupAdd(std::wstring key, GameState::PlayerPickup value) { std::pair<std::wstring, GameState::PlayerPickup> newEntry(key, value); ParamsPickup.insert(newEntry); }
	void ParamsFireAdd(std::wstring key, GameState::PlayerFire value) { std::pair<std::wstring, GameState::PlayerFire> newEntry(key, value); ParamsFire.insert(newEntry); }
	void ParamsMineAdd(std::wstring key, GameState::PlayerMine value) { std::pair<std::wstring, GameState::PlayerMine> newEntry(key, value); ParamsMine.insert(newEntry); }
	void ParamsHomingMineAdd(std::wstring key, GameState::PlayerHomingMine value) { std::pair<std::wstring, GameState::PlayerHomingMine> newEntry(key, value); ParamsHomingMine.insert(newEntry); }
	void ParamsLongAdd(std::wstring key, long value) { std::pair<std::wstring, long> newEntry(key, value); ParamsLong.insert(newEntry); }
	void ParamsDoubleAdd(std::wstring key, double value) { std::pair<std::wstring, double> newEntry(key, value); ParamsDouble.insert(newEntry); }
	void ParamsStringAdd(std::wstring key, std::wstring value) { std::pair<std::wstring, std::wstring> newEntry(key, value); ParamsString.insert(newEntry); }

};

#endif
