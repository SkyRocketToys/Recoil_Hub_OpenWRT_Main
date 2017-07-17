#include "../jsoncpp/include/json/config.h"
#include "../jsoncpp/include/json/json.h"

#include "GamePlayerState.h"
#include "GameNetworkServer.h"
#include "GameNetworkServerWrapper.h"
#include "RPCNetworkClient.h"
#include "Mathf.h"
#include "Debug.h"

#include <thread>

#define INT_TO_JSONVALUE(x) Json::Value((int)x)



TangibleEvent<IGameNetworkEvent::CreateLobbyFn> *GameNetworkServer::ReceiveCreateLobby = new TangibleEvent<IGameNetworkEvent::CreateLobbyFn>();
TangibleEvent<IGameNetworkEvent::CreateLobbyFailedFn> * GameNetworkServer::ReceiveCreateLobbyFailed = new TangibleEvent<IGameNetworkEvent::CreateLobbyFailedFn>();

TangibleEvent<IGameNetworkEvent::JoinLobbyFn> * GameNetworkServer::ReceiveJoinLobby = new TangibleEvent<IGameNetworkEvent::JoinLobbyFn>();

TangibleEvent<IGameNetworkEvent::PlayerConnectFn> * GameNetworkServer::ReceivePlayerConnect = new TangibleEvent<IGameNetworkEvent::PlayerConnectFn>();

TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn> * GameNetworkServer::ReceiveOtherPlayerConnect = new TangibleEvent<IGameNetworkEvent::OtherPlayerConnectFn>();

TangibleEvent<IGameNetworkEvent::PlayerReconnectFn> * GameNetworkServer::ReceivePlayerReconnect = new TangibleEvent<IGameNetworkEvent::PlayerReconnectFn>();


TangibleEvent<IGameNetworkEvent::PlayerDisconnectFn> * GameNetworkServer::ReceivePlayerDisconnect = new TangibleEvent<IGameNetworkEvent::PlayerDisconnectFn>();

TangibleEvent<IGameNetworkEvent::SetGameModeFn> * GameNetworkServer::ReceiveSetGameMode = new TangibleEvent<IGameNetworkEvent::SetGameModeFn>();
TangibleEvent<IGameNetworkEvent::ChangeGameModeFn> * GameNetworkServer::ReceiveChangeGameMode = new TangibleEvent<IGameNetworkEvent::ChangeGameModeFn>();
TangibleEvent<IGameNetworkEvent::ChangeTeamFn> * GameNetworkServer::ReceiveChangeTeam = new TangibleEvent<IGameNetworkEvent::ChangeTeamFn>();
TangibleEvent<IGameNetworkEvent::StartFn> * GameNetworkServer::ReceiveStart = new TangibleEvent<IGameNetworkEvent::StartFn>();
TangibleEvent<IGameNetworkEvent::AbortFn> * GameNetworkServer::ReceiveAbort = new TangibleEvent<IGameNetworkEvent::AbortFn>();
TangibleEvent<IGameNetworkEvent::PlayerRankChangeFn> * GameNetworkServer::ReceivePlayerRankChange = new TangibleEvent<IGameNetworkEvent::PlayerRankChangeFn>();

TangibleEvent<IGameNetworkEvent::UpdateGameStateFn> * GameNetworkServer::ReceiveUpdateGameState = new TangibleEvent<IGameNetworkEvent::UpdateGameStateFn>();
TangibleEvent<IGameNetworkEvent::GoToAmmoFn> * GameNetworkServer::ReceiveGoToAmmo = new TangibleEvent<IGameNetworkEvent::GoToAmmoFn>();
TangibleEvent<IGameNetworkEvent::GoToBaseFn> * GameNetworkServer::ReceiveGoToBase = new TangibleEvent<IGameNetworkEvent::GoToBaseFn>();
TangibleEvent<IGameNetworkEvent::FireFn> * GameNetworkServer::ReceiveFire = new TangibleEvent<IGameNetworkEvent::FireFn>();
TangibleEvent<IGameNetworkEvent::PlayerWasHitFn> * GameNetworkServer::ReceivePlayerWasHit = new TangibleEvent<IGameNetworkEvent::PlayerWasHitFn>();
TangibleEvent<IGameNetworkEvent::OtherPlayerWasHitFn> * GameNetworkServer::ReceiveOtherPlayerWasHit = new TangibleEvent<IGameNetworkEvent::OtherPlayerWasHitFn>();

TangibleEvent<IGameNetworkEvent::UpdatePlayerStateFn> * GameNetworkServer::ReceiveUpdatePlayerState = new TangibleEvent<IGameNetworkEvent::UpdatePlayerStateFn>();
TangibleEvent<IGameNetworkEvent::UpdateOtherPlayersStateFn> * GameNetworkServer::ReceiveUpdateOtherPlayersState = new TangibleEvent<IGameNetworkEvent::UpdateOtherPlayersStateFn>();

TangibleEvent<IGameNetworkEvent::ReloadClipFn> * GameNetworkServer::ReceiveReloadClip = new TangibleEvent<IGameNetworkEvent::ReloadClipFn>();
TangibleEvent<IGameNetworkEvent::PickedUpItemFn> * GameNetworkServer::ReceivePickedUpItem = new TangibleEvent<IGameNetworkEvent::PickedUpItemFn>();

TangibleEvent<IGameNetworkEvent::PlayerDeathFn> * GameNetworkServer::ReceivePlayerDeath = new TangibleEvent<IGameNetworkEvent::PlayerDeathFn>();
TangibleEvent<IGameNetworkEvent::OtherPlayerDiedFn> * GameNetworkServer::ReceiveOtherPlayerDied = new TangibleEvent<IGameNetworkEvent::OtherPlayerDiedFn>();
TangibleEvent<IGameNetworkEvent::RespawnPlayerFn> * GameNetworkServer::ReceiveRespawnPlayer = new TangibleEvent<IGameNetworkEvent::RespawnPlayerFn>();
TangibleEvent<IGameNetworkEvent::SetBasePointFn> * GameNetworkServer::ReceiveSetBase = new TangibleEvent<IGameNetworkEvent::SetBasePointFn>();
TangibleEvent<IGameNetworkEvent::SetRespawnPointFn> * GameNetworkServer::ReceiveSetRespawn = new TangibleEvent<IGameNetworkEvent::SetRespawnPointFn>();
TangibleEvent<IGameNetworkEvent::SetAmmoPointFn> * GameNetworkServer::ReceiveSetAmmo = new TangibleEvent<IGameNetworkEvent::SetAmmoPointFn>();
TangibleEvent<IGameNetworkEvent::SetBombPointFn> * GameNetworkServer::ReceiveSetBomb = new TangibleEvent<IGameNetworkEvent::SetBombPointFn>();
TangibleEvent<IGameNetworkEvent::GameOverFn> * GameNetworkServer::ReceiveGameOver = new TangibleEvent<IGameNetworkEvent::GameOverFn>();

TangibleEvent<IGameNetworkEvent::FlagPickedUpFn> * GameNetworkServer::ReceiveFlagPickedUp = new TangibleEvent<IGameNetworkEvent::FlagPickedUpFn>();
TangibleEvent<IGameNetworkEvent::FlagDroppedFn> * GameNetworkServer::ReceiveFlagDropped = new TangibleEvent<IGameNetworkEvent::FlagDroppedFn>();
TangibleEvent<IGameNetworkEvent::FlagReturnedFn> * GameNetworkServer::ReceiveFlagReturned = new TangibleEvent<IGameNetworkEvent::FlagReturnedFn>();

TangibleEvent<IGameNetworkEvent::RetryFn> * GameNetworkServer::ReceiveRetry = new TangibleEvent<IGameNetworkEvent::RetryFn>();

TangibleEvent<IGameNetworkEvent::NewMapFn> * GameNetworkServer::ReceiveMap = new TangibleEvent<IGameNetworkEvent::NewMapFn>();
TangibleEvent<IGameNetworkEvent::NewPlayerProfileFn> * GameNetworkServer::ReceivePlayerProfile = new TangibleEvent<IGameNetworkEvent::NewPlayerProfileFn>();
TangibleEvent<IGameNetworkEvent::NewVoiceDataFn> * GameNetworkServer::ReceiveVoiceData = new TangibleEvent<IGameNetworkEvent::NewVoiceDataFn>();
TangibleEvent<IGameNetworkEvent::NewExplosionFn> * GameNetworkServer::ReceiveExplosion = new TangibleEvent<IGameNetworkEvent::NewExplosionFn>();

TangibleEvent<IGameNetworkEvent::NewSpawnFireFn> * GameNetworkServer::ReceiveNewSpawnFire = new TangibleEvent<IGameNetworkEvent::NewSpawnFireFn>();

TangibleEvent<IGameNetworkEvent::NewSpawnMineFn> * GameNetworkServer::ReceiveNewSpawnMine = new TangibleEvent<IGameNetworkEvent::NewSpawnMineFn>();
TangibleEvent<IGameNetworkEvent::NewSpawnHomingMineFn> * GameNetworkServer::ReceiveNewSpawnHomingMine = new TangibleEvent<IGameNetworkEvent::NewSpawnHomingMineFn>();

TangibleEvent<IGameNetworkEvent::NewScrambleMapOnFn> * GameNetworkServer::ReceiveNewScrambleMapOn = new TangibleEvent<IGameNetworkEvent::NewScrambleMapOnFn>();
TangibleEvent<IGameNetworkEvent::NewScrambleMapOffFn> * GameNetworkServer::ReceiveNewScrambleMapOff = new TangibleEvent<IGameNetworkEvent::NewScrambleMapOffFn>();


// THREAD used to periodically update the state of the GAME SERVER
std::thread *updateThread = nullptr;


float GameNetworkServer::getServerTimestamp() const
{
	return _gameState->serverTimestamp;
}

int GameNetworkServer::FindPlayerIndexFromClientID(int clientID)
{
	int index = -1;
	for (int i = 0; i < _gameState->players.size(); i++)
	{
		if (_gameState->players[i]->clientID == clientID)
		{
			index = i;
			return index;
		}
	}
	return index;
}

void GameNetworkServer::Start()
{
	networkTime.Start();
	if (threadStarted == false)
	{
		threadStarted = true;
		updateThread = new std::thread(&GameNetworkServer::Update, this);
	}


	//------------------------------------------------------------------------
	// Set up the GAME SERVER to monitor incoming connections
	//
	// Add a local function to the list of those that will receive the ID of
	// the GAME SERVER or CLIENTS connected to the computer network.
	//------------------------------------------------------------------------
	//WorldLoadedAssets::Instance->networkManager.AddServerAddPlayerFn(OnStartClient);

	//------------------------------------------------------------------------
	// Set up the GAME SERVER to monitor disconnects
	//
	// Add a local function to the list of those that will receive the ID of
	// the GAME CLIENTS that disconnect from the network.
	//------------------------------------------------------------------------
	//WorldLoadedAssets::Instance->networkManager.AddServerDisconnectFn(OnClientDisconnect);

}

void GameNetworkServer::StartHost()
{
	DEBUG_LOG("start host called");

	Start();

	//------------------------------------------------------------------------
	// Start listening for incoming connections from the GAME CLIENTS.
	//------------------------------------------------------------------------
	//CustomNetworkManager::Instance->Host();
}

void GameNetworkServer::Setup(int clientID)
{
	DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: Setting up new CLIENT ") + std::to_wstring(clientID));

	ICustomNetworkClient *newNetworkObject = new RPCNetworkClient();
	//ICustomNetworkClient *newNetworkObject = nullptr;

	//std::vector<ICustomNetworkClient*> clients = WorldLoadedAssets::Instance->allClients;

	//// Loop through all the NETWORK OBJECTS that control communication for each PEER in a PEER-TO-PEER NETWORK
	//for (int i = 0; i < clients.size(); i++)
	//{
	//	// If this OBJECT were controlling the communication of the new GAME CLIENT,
	//	if (clients[i]->GetNetIdValue() == clientID)
	//	{
	//		newNetworkObject = clients[i];
	//		break;
	//	}
	//}

	if (newNetworkObject == nullptr)
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: NETWORK OBJECT not found. No communication with network");
		return;
	}

	// Add a local function that will receive data, sent to the GAME SERVER, in JSON FORMAT, from that CLIENT.
	// This data contains EVENTS.
	//newNetworkObject->AddServerReceiveJsonDataFn(f);

	if (_localNetworkObject == nullptr)
	{
		//if (newNetworkObject != nullptr)
		//{
		//	_localNetworkObject = newNetworkObject;
		//}

		_localNetworkObject = new RPCNetworkClient();
		// Add a local function that will receive data, sent to the GAME SERVER, in JSON FORMAT, from that CLIENT.
		// This data contains EVENTS.
		//
		// In UNITY NETWORKING each CLIENT had a unique instance of a GAME OBJECT (RPCNetworkClient) on the
		// GAME SERVER, which that CLIENT used to communicate with the SERVER. In this case, one GAME OBJECT
		// on the SERVER will be used by all the CLIENTS to communicate with the SERVER.
		CmdServerReceiveJsonDataFn f = std::bind(&GameNetworkServer::CmdServerReceiveJsonData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		_localNetworkObject->AddServerReceiveJsonDataFn(f);

		if (_localNetworkObject == nullptr)
		{
			DEBUG_LOGERROR(L"GAME NETWORK SERVER: NETWORK OBJECT not found. No communication with network");
			return;
		}

		// Add a function to the list that will respond to the EVENT
		//
		//     CREATE LOBBY
		//
		// sent when the player sends a command to create a new LOBBY for other players to join.
		GameNetworkServer::ReceiveCreateLobby->addListener(L"GameNetworkServer_ReceiveCreateLobby", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveCreateLobby, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     JOIN LOBBY
		//
		// sent when the player sends a command to join the LOBBY.
		GameNetworkServer::ReceiveJoinLobby->addListener(L"GameNetworkServer_ReceiveJoinLobby", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveJoinLobby, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     PLAYER CONNECT
		//
		// sent when the player connects to the GAME SERVER.
		GameNetworkServer::ReceivePlayerConnect->addListener(L"GameNetworkServer_ReceivePlayerConnect", std::bind(&GameNetworkServer::GameNetworkServer_ReceivePlayerConnect, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     OTHER PLAYER CONNECT
		//
		// sent when other player connects to the GAME SERVER.
		GameNetworkServer::ReceiveOtherPlayerConnect->addListener(L"GameNetworkServer_ReceiveOtherPlayerConnect", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveOtherPlayerConnect, this, std::placeholders::_1));

        // Add a function to the list that will respond to the EVENT
        //
        //     PLAYER RECONNECT
        //
        // sent when the player reconnects to a GAME SERVER
        GameNetworkServer::ReceivePlayerReconnect->addListener(L"GameNetworkServer_ReceivePlayerReconnect", std::bind(&GameNetworkServer::GameNetworkServer_ReceivePlayerReconnect, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     PLAYER DISCONNECT
		//
		// sent when the player disconnects from a GAME SERVER.
		// NOTE: the SERVER currently only sends this EVENT
		// it does not respond to it.
		GameNetworkServer::ReceivePlayerDisconnect->addListener(L"GameNetworkServer_ReceivePlayerDisconnect", std::bind(&GameNetworkServer::GameNetworkServer_ReceivePlayerDisconnect, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     CHANGE TEAM
		//
		// sent when the number of a player's team changes.
		GameNetworkServer::ReceiveChangeTeam->addListener(L"GameNetworkServer_ReceiveChangeTeam", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveChangeTeam, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     PLAYER RANK CHANGE
		//
		// sent when the player was promoted or demoted from being a captain of a team.
		GameNetworkServer::ReceivePlayerRankChange->addListener(L"GameNetworkServer_ReceivePlayerRankChange", std::bind(&GameNetworkServer::GameNetworkServer_ReceivePlayerRankChange, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     START
		//
		// sent when the master client is ready to start the game
		// (or when debug start is pressed on base station sim)
		GameNetworkServer::ReceiveStart->addListener(L"GameNetworkServer_ReceiveStart", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveStart, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     UPDATE PLAYER STATE
		//
		// sent when the player client updates its state
		GameNetworkServer::ReceiveUpdatePlayerState->addListener(L"GameNetworkServer_ReceiveUpdatePlayerState", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveUpdatePlayerState, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     PLAYER WAS HIT
		//
		// sent when the player client is hit by an IR bullet
		GameNetworkServer::ReceivePlayerWasHit->addListener(L"GameNetworkServer_ReceivePlayerWasHit", std::bind(&GameNetworkServer::GameNetworkServer_ReceivePlayerWasHit, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     PLAYER RESPAWN POINT
		//
		// sent when the player was respawned.
		GameNetworkServer::ReceiveSetRespawn->addListener(L"GameNetworkServer_ReceiveSetRespawnPointEvent", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveSetRespawnPointEvent, this, std::placeholders::_1));

		// Add a function to the list that will respond to the EVENT
		//
		//     EXPLOSION
		//
		// sent when a new explosion was pending on the map of the GAME WORLD.
		GameNetworkServer::ReceiveExplosion->addListener(L"GameNetworkServer_ReceiveExplosion", std::bind(&GameNetworkServer::GameNetworkServer_ReceiveExplosion, this, std::placeholders::_1));
	}


	newNetworkObject->getPlayerState()->clientID = clientID;
	if (!newNetworkObject->getIsLocalPlayer())
	{
		DEBUG_LOG(std::wstring(L"-------ADDING PLAYER DATA: ") + std::to_wstring(clientID) + std::wstring(L" Event History Size: ") + std::to_wstring(_eventHistory.size()));

		// Add the properties of the new player to the list of player's properties.
		_gameState->players.push_back(newNetworkObject->getPlayerState());

		// Add the ID of the new player to the list of IDs.
		_gameState->playerClients.push_back(clientID);
	}
}

void GameNetworkServer::GameNetworkServer_ReceiveCreateLobby(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':

		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		int newClientID = static_cast<int>(newClientIDObj);

		_gameState->serverState = GameState::State::Lobby;
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		_gameState->serverLobbyOwner = newEvent->ParamsString[GameEvents::eventDataString[(int)GameEvents::eventData::serverLobbyOwner]];

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		if (newEvent->ParamsVec3ContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::serverBaseStationLocation]))
		{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			GameState::HotGenVec3 *vectorPtr = &(newEvent->ParamsVec3[GameEvents::eventDataString[(int)GameEvents::eventData::serverBaseStationLocation]]);
			_gameState->baseStationPosition = (*vectorPtr);
		}

		if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::serverGameModeTimeLimit]))
		{
			_gameState->serverGameModeTimeLimit = newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::serverGameModeTimeLimit]];
		}

		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveCreateLobby: clientID->") + std::to_wstring(newClientID) + 
			std::wstring(L" serverLobbyOwner -> ") + _gameState->serverLobbyOwner +
			std::wstring(L" serverBaseStationLocation -> ") + std::to_wstring(_gameState->baseStationPosition.X) + std::wstring(L",") + std::to_wstring(_gameState->baseStationPosition.Y) +
			std::wstring(L" serverGameModeTimeLimit -> ") + std::to_wstring(_gameState->serverGameModeTimeLimit));

		// Echo the EVENT back to all the CLIENTS.
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)_gameState->serverState);
		SendEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceiveJoinLobby(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		int newClientID = static_cast<int>(newClientIDObj);
		GamePlayerState::Team newClientTeamID;

		// If the players were in the LOBBBY,
		if (_gameState->serverState == GameState::State::Lobby)
		{
			int index = FindPlayerIndexFromClientID(newClientID);
			if (index < 0)
			{
				DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(newClientID) + L" found or player does not exist!");
				return;
			}

			int orangeTeamListCount = 0;
			int blueTeamListCount = 0;

			// Count the number of players on the orange and blue teams.
			for (unsigned int i = 0; i < _gameState->players.size(); i++)
			{
				if (_gameState->players[i]->clientTeamID == GamePlayerState::Team::Orange)
				{
					orangeTeamListCount++;
				}
				else if (_gameState->players[i]->clientTeamID == GamePlayerState::Team::Blue)
				{
					blueTeamListCount++;
				}
			}

			// If the number of players on the blue team were less than the numbers on the orange team,
			if (blueTeamListCount < orangeTeamListCount)
			{
				// Add the new player to the blue team.
				newClientTeamID = GamePlayerState::Team::Blue;
			}
			else
			{
				// Add the new player to the orange team.
				newClientTeamID = GamePlayerState::Team::Orange;
			}

			_gameState->players[index]->clientTeamID = newClientTeamID;

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			if (newEvent->ParamsStringContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientPlayerName]))
			{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
				_gameState->players[index]->clientPlayerName = newEvent->ParamsString[GameEvents::eventDataString[(int)GameEvents::eventData::clientPlayerName]];
			}

			DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveJoinLobby: clientID->") + std::to_wstring(newClientID) + std::wstring(L" clientTeamID ") + std::to_wstring((int)newClientTeamID));

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			// Send back the team the new player was on
			newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientTeamID], (long)newClientTeamID);
			// Send back the position of the BASE STATION
			newEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::serverBaseStationLocation], _gameState->baseStationPosition);

			// Echo the EVENT back to all the CLIENTS.
			SendEvent(newEvent);
		}
		else
		{
			DEBUG_LOGERROR(std::wstring(L"GameNetworkServer_ReceiveJoinLobby: clientID->") + std::to_wstring(newClientID) + std::wstring(L" Failed"));
		}
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceivePlayerConnect(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);

		int newClientID = static_cast<int>(newClientIDObj);

		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceivePlayerConnect: clientID->") + std::to_wstring(newClientID) + std::wstring(L" type ") + StringHelper::typeToString(newClientIDObj));

		// If the players were in the LOBBY,
		if (_gameState->serverState == GameState::State::Lobby)
		{
			// Replay the previous EVENTS before the new player entered the LOBBY.
			for (int i = 0; i < _eventHistory.size(); i++)
			{
				SendEvent(newClientID, _eventHistory[i]);
			}
		}

		// Loop through the existing CLIENTS
		for (int i = 0; i < _gameState->playerClients.size(); i++)
		{
			// If this were not the CLIENT that just connected,
			if (_gameState->playerClients[i] != newClientID)
			{
				// Send an OTHER PLAYER CONNECT EVENT, to this existing CLIENT, for the new CLIENT
				SendOtherPlayerConnect(_gameState->playerClients[i], newClientID);
			}
		}

		// Echo the EVENT back to the CLIENT.
		SendEvent(newClientID, newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceiveOtherPlayerConnect(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveOtherPlayerConnect: clientID->") + std::to_wstring(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]) + std::wstring(L" type ") + StringHelper::typeToString(newClientIDObj));

		int newClientID = static_cast<int>(newClientIDObj);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceivePlayerReconnect(GameEvents *newEvent)
{
    if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
    {
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		long long oldClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientOldID]]);
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceivePlayerReconnect:") +
			std::wstring(L"clientID->") +
			std::to_wstring(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]) +
			std::wstring(L"clientOldID->") +
			std::to_wstring(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientOldID]]) +
			std::wstring(L" type ") + StringHelper::typeToString(newClientIDObj));

        OnStartClient((int)newClientIDObj, (int)oldClientIDObj);

        // Echo the EVENT back to all the CLIENTS.
        SendEvent(newEvent);
    }
    else
    {
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
    }
}

void GameNetworkServer::GameNetworkServer_ReceivePlayerDisconnect(GameEvents *newEvent)
{
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
		//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceivePlayerDisconnect:") +
			std::wstring(L"clientID->") +
			std::to_wstring(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]) +
			std::wstring(L" type ") + StringHelper::typeToString(newClientIDObj));

	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceiveChangeTeam(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientTeamIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientTeamID]]);

		int newClientID = static_cast<int>(newClientIDObj);
		GamePlayerState::Team newClientTeamID = static_cast<GamePlayerState::Team>(newClientTeamIDObj);

		int index = FindPlayerIndexFromClientID(newClientID);
		if (index < 0)
		{
			DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(newClientID) + L" found or player does not exist!");
			return;
		}

		_gameState->players[index]->clientTeamID = newClientTeamID;
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveChangeTeam: clientID->") + std::to_wstring(newClientID) + std::wstring(L" clientTeamID ") + std::to_wstring((int)newClientTeamID));

		// Echo the EVENT back to all the CLIENTS.
		SendEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceivePlayerRankChange(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		bool newIsCaptain = static_cast<bool>(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientIsCaptain]]);

		int newClientID = static_cast<int>(newClientIDObj);
		int index = FindPlayerIndexFromClientID(newClientID);
		if (index < 0)
		{
			DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(newClientID) + L" found or player does not exist!");
			return;
		}

		_gameState->players[index]->clientIsCaptain = newIsCaptain;

		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceivePlayerRankChange: clientID->") + std::to_wstring(newClientID) + std::wstring(L" clientIsCaptain ") + StringHelper::toString(newIsCaptain));

		// Echo the EVENT back to all the CLIENTS.
		SendEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}

}

void GameNetworkServer::GameNetworkServer_ReceiveStart(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		int newClientID = static_cast<int>(newClientIDObj);

		_gameState->serverState = GameState::State::WaitingPlayers;

		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveStart: clientID->") + std::to_wstring(newClientID));

		// Echo the EVENT back to all the CLIENTS.
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)(_gameState->serverState));
		SendEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}

}

void GameNetworkServer::GameNetworkServer_ReceiveUpdatePlayerState(GameEvents *newEvent)
{
	DEBUG_LOG(L"update player state received");
	//pull out the ready value
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientID = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
		int index = FindPlayerIndexFromClientID(static_cast<int>(newClientID));
		if (index < 0)
		{
			DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(newClientID) + L" found or player does not exist!");
			return;
		}

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientReady]))
		{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			_gameState->players[index]->clientReady = static_cast<bool>(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientReady]]);
		}

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		if (newEvent->ParamsVec3ContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientPosition]))
		{
			DEBUG_LOG(L"player position received");
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			GameState::HotGenVec3 *vectorPtr = &(newEvent->ParamsVec3[GameEvents::eventDataString[(int)GameEvents::eventData::clientPosition]]);

			_gameState->players[index]->clientPosition = *vectorPtr; //.ToVector3();
		}

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		if (newEvent->ParamsDoubleContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientHeading]))
		{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
			_gameState->players[index]->clientHeading = (newEvent->ParamsDouble[GameEvents::eventDataString[(int)GameEvents::eventData::clientHeading]]);
		}

		// Echo the EVENT back to all the CLIENTS.
		SendEvent(newEvent);
	}
}

void GameNetworkServer::GameNetworkServer_ReceivePlayerWasHit(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);

		int newClientID = static_cast<int>(newClientIDObj);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long shooterClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::DamageSrc]]);
		int shooterClientID = static_cast<int>(shooterClientIDObj);
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceivePlayerWasHit: ") + std::to_wstring(newClientID) + std::wstring(L" , ") + std::to_wstring(shooterClientID));
		//todo, arbitrate and then return a damage message
		ArbitrateAndSendDamage(newClientID, shooterClientID, DamageType::Bullet);
	}
	else
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: Missing clientID");
	}
}

void GameNetworkServer::GameNetworkServer_ReceiveSetRespawnPointEvent(GameEvents *newEvent)
{
	DEBUG_LOG(L"recieved respawn location");
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);

		int newClientID = static_cast<int>(newClientIDObj);
		//get the players team
		int playerIndex = FindPlayerIndexFromClientID(newClientID);
		if (playerIndex < 0)
		{
			DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(newClientID) + L" found or player does not exist!");
			return;
		}

		GamePlayerState::Team team = _gameState->players[playerIndex]->clientTeamID;

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		GameState::HotGenVec3 *newRespawnPos = &(newEvent->ParamsVec3[GameEvents::eventDataString[(int)GameEvents::eventData::serverLastRespawnPoint]]);

		_gameState->playerRespawnPoints[static_cast<int>(team)] = *newRespawnPos;

		//acknowledge reciept by echoing the message back
		SendSetRespawn(newClientID, newRespawnPos);

	}
	////sets the position of the players team base (respawn point)
	//Debug.Log("sending base position data");
	//    GameEvents newEvent = new GameEvents(clientID, GameEvents.eventID.SetRespawnPointEvent);

	//newEvent.ParamsVec3.Add(GameEvents.eventData.serverLastRespawnPoint.ToString(), serverLastRespawnPoint);

	//    //todo, rotation
	//    SendEvent(newEvent);
}

void GameNetworkServer::GameNetworkServer_ReceiveExplosion(GameEvents *newEvent)
{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	if (newEvent->ParamsLongContainsKey(GameEvents::eventDataString[(int)GameEvents::eventData::clientID]))
	{
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		long long newClientIDObj = (long long)(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		GameState::PlayerExplosion *newExplosion = &(newEvent->ParamsExplosion[GameEvents::eventDataString[(int)GameEvents::eventData::serverLastExplosion]]);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
		DEBUG_LOG(std::wstring(L"GameNetworkServer_ReceiveExplosion: clientID->") + std::to_wstring(newEvent->ParamsLong[GameEvents::eventDataString[(int)GameEvents::eventData::clientID]]) + std::wstring(L" explosion AttackerClientID ") + std::to_wstring(newExplosion->AttackerClientID));

		// Add a new pending explosion to the list.
		_gameState->playerExplosions.push_back(newExplosion);
		_gameState->playerExplosionsStartTimes.push_back(networkTime.getCurrentTime());

		// Echo the EVENT back to all the CLIENTS.
		SendEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(L"GameNetworkServer_ReceiveExplosion: Missing clientID");
	}
}

int GameNetworkServer::GetDamageFromArmourAmmoCombination(GamePlayerState::ArmourType armourType, GamePlayerState::AmmoType ammoType)
{
	//todo, perform correct calculation to calculate damage from the amour/ammo combo
	//for now just return 6 for each hit.
	return 6;
}

void GameNetworkServer::ArbitrateAndSendDamage(int targetClient, int sourceClient, DamageType type, int damage)
{
	//check the type of armour that the target has (needs target index)
	int targetIndex = FindPlayerIndexFromClientID(targetClient);
	if (targetIndex < 0)
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(targetClient) + L" found or player does not exist!");
		return;
	}

	//check the type of ammo that the source has (needs source index)
	int sourceIndex = FindPlayerIndexFromClientID(sourceClient);
	if (sourceIndex < 0)
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(sourceClient) + L" found or player does not exist!");
		return;
	}

	GameState::HotGenVec3 *sourcePos;

	switch (type)
	{
		case DamageType::Bullet:
		{

				//with bullets, it the source player is dead, he cant do any damage so just ignore this hit event
				if (_gameState->players[sourceIndex]->armour <= 0)
				{
					//do nothing
					return;
				}
				if (_gameState->players[targetIndex]->armour <= 0)
				{
					//do nothing player already dead
					return;
				}

				//calculate damage amount
				damage = GetDamageFromArmourAmmoCombination(_gameState->players[targetIndex]->armourType, _gameState->players[sourceIndex]->ammoType);

				DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: ") + std::to_wstring(damage) + std::wstring(L" damage done by ") + std::to_wstring(sourceClient) + std::wstring(L" to ") + std::to_wstring(targetClient));

				//find the source location
				sourcePos = &(_gameState->players[sourceIndex]->clientPosition);

				//calculate new armour value for the target
				_gameState->players[targetIndex]->armour -= damage;

				if (_gameState->players[targetIndex]->armour <= 0)
				{
					_gameState->players[targetIndex]->armour = 0;
					_gameState->players[targetIndex]->clientDisplayType = GamePlayerState::PlayerDisplayType::Dead;
					//player is dead
					//add score to the killing player
					_gameState->players[targetIndex]->deaths++;
					_gameState->players[sourceIndex]->kills++;
				}

				//send to the target
				SendPlayerTakeDamage(targetClient, sourceClient, sourcePos, _gameState->players[targetIndex]->armour);
				//TODO, send a message to the source of the damage informing him that he has hit or killed this player

		}

			break;

		case DamageType::Explosion:
		{
				//with bullets, it the source player is dead, he cant do any damage so just ignore this hit event
				if (_gameState->players[targetIndex]->armour <= 0)
				{
					//do nothing player already dead
					return;
				}

				DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: ") + std::to_wstring(damage) + std::wstring(L" damage done by ") + std::to_wstring(sourceClient) + std::wstring(L" to ") + std::to_wstring(targetClient));

				//find the source location
				sourcePos = &(_gameState->players[sourceIndex]->clientPosition);

				//calculate new armour value for the target
				_gameState->players[targetIndex]->armour -= damage;

				if (_gameState->players[targetIndex]->armour <= 0)
				{
					_gameState->players[targetIndex]->armour = 0;
					_gameState->players[targetIndex]->clientDisplayType = GamePlayerState::PlayerDisplayType::Dead;
					//player is dead
					//add score to the killing player
					_gameState->players[targetIndex]->deaths++;
					_gameState->players[sourceIndex]->kills++;
				}

				//send to the target
				SendPlayerTakeDamage(targetClient, sourceClient, sourcePos, _gameState->players[targetIndex]->armour);
				//TODO, send a message to the source of the damage informing him that he has hit or killed this player
		}

			break;

		default:
			DEBUG_LOGWARNING(L"damage type not implemented yet");
			break;
	}
}

void GameNetworkServer::OnEnable()
{

}

void GameNetworkServer::OnDisable()
{
	StopHost();
}

/// <summary>
/// When a GAME CLIENT has connected or reconnected to the SERVER
/// </summary>
/// <param name="newClientID">The new ID of the CLIENT</param>
/// <param name="oldClientID">The old ID of the CLIENT if reconnecting or -1</param>
void GameNetworkServer::OnStartClient(int newClientID, int oldClientID)
{
    // Find the new CLIENT
    int newClientIdx = FindPlayerIndexFromClientID(newClientID);
 
    // If there were no record of the new CLIENT,
    if (newClientIdx == -1)
    {
		numberOfClientsConnected++;
        // Set up the new CLIENT
        Setup(newClientID);
    }
    else
    {
        // Find the old CLIENT
        int oldClientIdx = FindPlayerIndexFromClientID(oldClientID);

		DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: Old Client") + std::to_wstring(oldClientID) +
			std::wstring(L" reconnecting with new ID ") + std::to_wstring(newClientID) +
			std::wstring(L" oldClientIdx=") + std::to_wstring(oldClientIdx) +
			std::wstring(L" newClientIdx=") + std::to_wstring(newClientIdx));

        // If the old CLIENT were found,
        if (oldClientIdx != -1)
        {
			DEBUG_LOG("GAME NETWORK SERVER: Copying properties from old Client to new Client");

			// If the ID of the old and new CLIENT were different,
			if (oldClientIdx != newClientIdx)
			{
				// Copy the properties of the old Client to the new Client
				_gameState->players[newClientIdx] = _gameState->players[oldClientIdx];

				// Except the ID and the status of the connection.
				_gameState->players[newClientIdx]->clientID = newClientID;
				_gameState->players[newClientIdx]->clientIsDisconnected = false;

				// Remove the properties of the old CLIENT.
				_gameState->PlayersRemoveAt(oldClientIdx);
				_gameState->PlayerClientsRemoveAt(oldClientIdx);
			}
			else
			{
				// Mark the new (i.e. old) CLIENT as reconnected.
				_gameState->players[newClientIdx]->clientIsDisconnected = false;
			}
        }
    }

}
 
void GameNetworkServer::OnClientDisconnect(int oldClientID)
{
    int i = 0;

    DEBUG_LOG(L"GameNetworkServer: OnClientDisconnect " + std::to_wstring(oldClientID));

    // Loop through all the existing GAME CLIENTS
    for (i = 0; i < _gameState->playerClients.size(); i++)
    {
        // If this existing CLIENT had the old ID
        if (_gameState->playerClients[i] == oldClientID)
        {
            // Remember the time.
			_gameState->players[i]->clientDisconnectTime = networkTime.getCurrentTime();

            // Mark the player as disconnected
            _gameState->players[i]->clientIsDisconnected = true;
			break;
        }
    }
 
    if (i < _gameState->playerClients.size())
    {
        switch (_gameState->serverState)
        {
			case GameState::State::Playing:
                // During a GAME MODE disconnect players with a delay, allowing the players time to reconnect
                SendPlayerDisconnect(oldClientID, DEFAULT_TIME_LEFT_TO_RECONNECT);
                break;

            default:
                // Before or after a GAME MODE, disconnect players immediately.
                SendPlayerDisconnect(oldClientID, 0.0f);
                _gameState->players[i]->clientDisconnectTime = 0.0f;
				numberOfClientsConnected--;
				break;
        }
    }
}

void GameNetworkServer::CmdServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	Json::CharReaderBuilder b;
	Json::CharReader* reader(b.newCharReader());
	JSONCPP_STRING errs;
	Json::Value root;
	std::string jsonText(jsonBytes.begin(), jsonBytes.end());
	bool ok = reader->parse(
		jsonText.data(), jsonText.data() + jsonText.size(),
		&root, &errs);

	GameEvents *newEvent = nullptr;
	if (errs.size() == 0)
	{
		newEvent = new GameEvents();

		// Read the ID of the EVENT from JSON.
		newEvent->ID = (GameEvents::eventID)root["ID"].asInt();

		// Read the DICTIONARY of positions, from JSON
		Json::Value jsonVec3 = root["ParamsVec3"];
		//if (jsonVec3 != nullptr)
		{
			for (Json::ValueIterator it = jsonVec3.begin(); it != jsonVec3.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();

				GameState::HotGenVec3 newVec;
				newVec.X = itValue["X"].asDouble();
				newVec.Y = itValue["Y"].asDouble();
				newVec.Z = itValue["Z"].asDouble();
				newEvent->ParamsVec3Add(std::wstring(keyName.begin(), keyName.end()), newVec);
			}
		}

		// Read the DICTIONARY of list of results, from JSON
		Json::Value jsonResult = root["ParamsResult"];
		//if (jsonResult != nullptr)
		{
			for (Json::ValueIterator it = jsonResult.begin(); it != jsonResult.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();
				std::wstring wKeyName(keyName.begin(), keyName.end());

				// Read list of results, from JSON
				std::vector<GameState::GameResult> newList;
				Json::Value jsonResultList = itValue;
				for (Json::ValueIterator itList = jsonResultList.begin(); itList != jsonResultList.end(); ++itList) {
					Json::Value itResult = (*itList);
					std::string keyListIdx = itList.key().asCString();
					std::string playerName = jsonResultList[keyListIdx]["Name"].asString();

					std::wstring wName(playerName.begin(), playerName.end());
					int kills = itResult["Kills"].asInt();
					int deaths = itResult["Deaths"].asInt();;
					int clientID = itResult["ClientID"].asInt();

					GameState::GameResult newResult(wName, kills, deaths, clientID);
					newList.push_back(newResult);
				}

				newEvent->ParamsResultAdd(wKeyName, newList);
			}
		}

		// Read the DICTIONARY of explosions from JSON
		Json::Value jsonExplosions = root["ParamsExplosion"];
		//if (jsonExplosions != nullptr)
		{
			for (Json::ValueIterator it = jsonExplosions.begin(); it != jsonExplosions.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();
				std::wstring wKeyName(keyName.begin(), keyName.end());

				GameState::HotGenVec3 newObjSpacePosition;
				GameState::HotGenVec3 newGpsSpacePosition;
				double Delay = 0;
				double Radius = 0;
				double Damage = 0;
				long AttackerClientID = 0;

				newObjSpacePosition.X = itValue["objectSpacePosition"]["X"].asDouble();
				newObjSpacePosition.Y = itValue["objectSpacePosition"]["Y"].asDouble();
				newObjSpacePosition.Z = itValue["objectSpacePosition"]["Z"].asDouble();
				newGpsSpacePosition.X = itValue["gpsSpacePosition"]["X"].asDouble();
				newGpsSpacePosition.Y = itValue["gpsSpacePosition"]["Y"].asDouble();
				newGpsSpacePosition.Z = itValue["gpsSpacePosition"]["Z"].asDouble();
				Delay = itValue["delay"].asDouble();
				Radius = itValue["radius"].asDouble();
				Damage = itValue["damage"].asDouble();
				AttackerClientID = itValue["attackerClientID"].asInt64();
				GameState::PlayerExplosion newExplosion(&newObjSpacePosition, &newGpsSpacePosition, Delay, Radius, Damage, AttackerClientID);
				newEvent->ParamsExplosionAdd(wKeyName, newExplosion);
			}
		}

		// Read the DICTIONARY of WHOLE NUMBERS (long), from JSON
		Json::Value jsonLong = root["ParamsLong"];
		//if (jsonLong != nullptr)
		{
			for (Json::ValueIterator it = jsonLong.begin(); it != jsonLong.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();
				std::wstring wKeyName(keyName.begin(), keyName.end());

				long newValue = itValue.asInt64();
				newEvent->ParamsLongAdd(wKeyName, newValue);
			}
		}

		// Read the DICTIONARY of FLOATING NUMBERS (double), from JSON
		Json::Value jsonDouble = root["ParamsDouble"];
		//if (jsonDouble != nullptr)
		{
			for (Json::ValueIterator it = jsonDouble.begin(); it != jsonDouble.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();
				std::wstring wKeyName(keyName.begin(), keyName.end());

				double newValue = itValue.asDouble();
				newEvent->ParamsDoubleAdd(wKeyName, newValue);
			}
		}

		// Read the DICTIONARY of STRINGS (std::wstring), from JSON
		Json::Value jsonString = root["ParamsString"];
		//if (jsonString != nullptr)
		{
			for (Json::ValueIterator it = jsonString.begin(); it != jsonString.end(); ++it) {
				Json::Value itValue = (*it);
				std::string keyName = it.key().asCString();
				std::wstring wKeyName(keyName.begin(), keyName.end());
				std::string newValue = itValue.asString();
				std::wstring wNewValue(newValue.begin(), newValue.end());

				newEvent->ParamsStringAdd(wKeyName, wNewValue);
			}
		}

	}

	if (newEvent != nullptr)
	{
		DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: CmdServerReceiveJsonData Client ") + std::to_wstring(clientID) + std::wstring(L" Event ") + std::to_wstring((int)newEvent->ID) + std::wstring(L" Size ") + std::to_wstring((int)jsonText.length()) + std::wstring(L" Text ") + std::wstring(jsonText.begin(), jsonText.end()));
		ReceiveEvent(newEvent);
	}
	else
	{
		DEBUG_LOGERROR(std::wstring(L"GAME NETWORK SERVER: CmdServerReceiveJsonData Bad Event ") + std::wstring(jsonText.begin(), jsonText.end()));
	}

	delete reader;
}

bool GameNetworkServer::ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	return false;
}

bool GameNetworkServer::ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	return false;
}

void GameNetworkServer::ReceiveEvent(GameEvents *newEvent)
{
	// Keep a history of all EVENTS received.
	// So that this could be played back to any new players who join the game, typically in the LOBBY.
	_eventHistory.push_back(newEvent);

	switch (newEvent->ID)
	{
		case GameEvents::eventID::CreateLobbyEvent:
			if (GameNetworkServer::ReceiveCreateLobby != nullptr)
			{
				for (auto listener : ReceiveCreateLobby->listeners())
				{
					listener(newEvent);
				}
			}

			break;
		case GameEvents::eventID::CreateLobbyFailedEvent:
			if (GameNetworkServer::ReceiveCreateLobbyFailed != nullptr)
			{
				for (auto listener : ReceiveCreateLobbyFailed->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::JoinLobbyEvent:
			if (GameNetworkServer::ReceiveJoinLobby != nullptr)
			{
				for (auto listener : ReceiveJoinLobby->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::PlayerConnectEvent:
			if (GameNetworkServer::ReceivePlayerConnect != nullptr)
			{
				for (auto listener : ReceivePlayerConnect->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::OtherPlayerConnectEvent:
			if (GameNetworkServer::ReceiveOtherPlayerConnect != nullptr)
			{
				for (auto listener : ReceiveOtherPlayerConnect->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::PlayerReconnectEvent:
                if (GameNetworkServer::ReceivePlayerReconnect != nullptr)
                {
					for (auto listener : ReceivePlayerReconnect->listeners())
					{
						listener(newEvent);
					}
				}
                break;
		case GameEvents::eventID::PlayerDisconnectEvent:
			if (GameNetworkServer::ReceivePlayerDisconnect != nullptr)
			{
				for (auto listener : ReceivePlayerDisconnect->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::SetGameModeEvent:
			if (GameNetworkServer::ReceiveSetGameMode != nullptr)
			{
				for (auto listener : ReceiveSetGameMode->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::ChangeGameModeEvent:
			if (GameNetworkServer::ReceiveChangeGameMode != nullptr)
			{
				for (auto listener : ReceiveChangeGameMode->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::ChangeTeamEvent:
			if (GameNetworkServer::ReceiveChangeTeam != nullptr)
			{
				for (auto listener : ReceiveChangeTeam->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::StartEvent:
			if (GameNetworkServer::ReceiveStart != nullptr)
			{
				for (auto listener : ReceiveStart->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::AbortEvent:
			if (GameNetworkServer::ReceiveAbort != nullptr)
			{
				for (auto listener : ReceiveAbort->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::PlayerRankChangeEvent:
			if (GameNetworkServer::ReceivePlayerRankChange != nullptr)
			{
				for (auto listener : ReceivePlayerRankChange->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::UpdateGameStateEvent:
			if (GameNetworkServer::ReceiveUpdateGameState != nullptr)
			{
				for (auto listener : ReceiveUpdateGameState->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::GoToAmmoEvent:
			if (GameNetworkServer::ReceiveGoToAmmo != nullptr)
			{
				for (auto listener : ReceiveGoToAmmo->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::GoToBaseEvent:
			if (GameNetworkServer::ReceiveGoToBase != nullptr)
			{
				for (auto listener : ReceiveGoToBase->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::FireEvent:
			if (GameNetworkServer::ReceiveFire != nullptr)
			{
				for (auto listener : ReceiveFire->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::PlayerWasHitEvent:
			if (GameNetworkServer::ReceivePlayerWasHit != nullptr)
			{
				for (auto listener : ReceivePlayerWasHit->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::OtherPlayerWasHitEvent:
			if (GameNetworkServer::ReceiveOtherPlayerWasHit != nullptr)
			{
				for (auto listener : ReceiveOtherPlayerWasHit->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::UpdatePlayerStateEvent:
			if (GameNetworkServer::ReceiveUpdatePlayerState != nullptr)
			{
				for (auto listener : ReceiveUpdatePlayerState->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::UpdateOtherPlayersStateEvent:
			if (GameNetworkServer::ReceiveUpdateOtherPlayersState != nullptr)
			{
				for (auto listener : ReceiveUpdateOtherPlayersState->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::ReloadClipEvent:
			if (GameNetworkServer::ReceiveReloadClip != nullptr)
			{
				for (auto listener : ReceiveReloadClip->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::PickedUpItemEvent:
			if (GameNetworkServer::ReceivePickedUpItem != nullptr)
			{
				for (auto listener : ReceivePickedUpItem->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::PlayerDeathEvent:
			if (GameNetworkServer::ReceivePlayerDeath != nullptr)
			{
				for (auto listener : ReceivePlayerDeath->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::OtherPlayerDiedEvent:
			if (GameNetworkServer::ReceiveOtherPlayerDied != nullptr)
			{
				for (auto listener : ReceiveOtherPlayerDied->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::RespawnPlayerEvent:
			if (GameNetworkServer::ReceiveRespawnPlayer != nullptr)
			{
				for (auto listener : ReceiveRespawnPlayer->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::SetBasePointEvent:
			if (GameNetworkServer::ReceiveSetBase != nullptr)
			{
				for (auto listener : ReceiveSetBase->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::SetRespawnPointEvent:
			if (GameNetworkServer::ReceiveSetRespawn != nullptr)
			{
				for (auto listener : ReceiveSetRespawn->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::SetAmmoPointEvent:
			if (GameNetworkServer::ReceiveSetAmmo != nullptr)
			{
				for (auto listener : ReceiveSetAmmo->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::SetBombPointEvent:
			if (GameNetworkServer::ReceiveSetBomb != nullptr)
			{
				for (auto listener : ReceiveSetBomb->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::GameOverEvent:
			if (GameNetworkServer::ReceiveGameOver != nullptr)
			{
				for (auto listener : ReceiveGameOver->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::FlagPickedUpEvent:
			if (GameNetworkServer::ReceiveFlagPickedUp != nullptr)
			{
				for (auto listener : ReceiveFlagPickedUp->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::FlagDroppedEvent:
			if (GameNetworkServer::ReceiveFlagDropped != nullptr)
			{
				for (auto listener : ReceiveFlagDropped->listeners())
				{
					listener(newEvent);
				}
			}
			break;
		case GameEvents::eventID::FlagReturnedEvent:
			if (GameNetworkServer::ReceiveFlagReturned != nullptr)
			{
				for (auto listener : ReceiveFlagReturned->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::RetryEvent:
			if (GameNetworkServer::ReceiveRetry != nullptr)
			{
				for (auto listener : ReceiveRetry->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		case GameEvents::eventID::NewExplosionEvent:
			if (GameNetworkServer::ReceiveExplosion != nullptr)
			{
				for (auto listener : ReceiveExplosion->listeners())
				{
					listener(newEvent);
				}
			}
			break;

		// The following EVENTS are not implemented at all. And you cannot add a function that responds to them.
		case GameEvents::eventID::NewMapEvent:
		case GameEvents::eventID::NewPlayerProfileEvent:
		case GameEvents::eventID::NewVoiceDataEvent:

		case GameEvents::eventID::NewSpawnFireEvent:
		case GameEvents::eventID::NewSpawnMineEvent:
		case GameEvents::eventID::NewSpawnHomingMineEvent:

		case GameEvents::eventID::NewScrambleMapOnEvent:
		case GameEvents::eventID::NewScrambleMapOffEvent:

		default:
			DEBUG_LOGERROR(std::wstring(L"GAME NETWORK SERVER: Event not implemented ") + std::to_wstring((int)newEvent->ID));
			break;
	}
}

void GameNetworkServer::SendEvent(GameEvents *newEvent)
{
	for (int i = 0; i < _gameState->playerClients.size(); i++)
	{
		DEBUG_LOG(std::wstring(L"sending event to: ") + std::to_wstring(_gameState->playerClients[i]));
		SendEvent(_gameState->playerClients[i], newEvent);
	}
	//NOTE! not working atm, do not use
	//string jsonText = JsonMapper.ToJson(newEvent);

	//if (_localNetworkObject != null)
	//{
	//    byte[] jsonBytes = System.Text.Encoding.UTF8.GetBytes(jsonText);
	//    _localNetworkObject.ServerSendJsonData(jsonBytes, jsonText.Length);
	//}
}

void GameNetworkServer::SendEvent(int clientID, GameEvents *newEvent)
{
	Json::StyledWriter writer;
	Json::Value root;
	
	Json::Value jsonID = INT_TO_JSONVALUE(newEvent->ID);
	root["ID"] = jsonID;

	//root["ID"] = JSONCPP_STRING( StringHelper::toNarrowString((int)newEvent->ID) );

	if (newEvent->ParamsVec3.size() > 0)
	{
		// Write a DICTIONARY of positions in JSON FORMAT
		for (PARAMSVEC3_ITERATOR it = newEvent->ParamsVec3.begin(); it != newEvent->ParamsVec3.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			GameState::HotGenVec3 value = (*it).second;

			root["ParamsVec3"][kName]["X"] = value.X;
			root["ParamsVec3"][kName]["Y"] = value.Y;
			root["ParamsVec3"][kName]["Z"] = value.Z;
		}
	}

	if (newEvent->ParamsResult.size() > 0)
	{
		// Write a DICTIONARY of list of results in JSON FORMAT
		for (PARAMSRESULT_ITERATOR it = newEvent->ParamsResult.begin(); it != newEvent->ParamsResult.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			std::vector<GameState::GameResult> value = (*it).second;

			// Write a list of results in JSON FORMAT
			for (int i = 0; i < value.size(); i++)
			{
				root["ParamsResult"][kName][i]["ClientID"] = INT_TO_JSONVALUE(value[i].ClientID);
				root["ParamsResult"][kName][i]["Deaths"] = INT_TO_JSONVALUE(value[i].Deaths);
				root["ParamsResult"][kName][i]["Kills"] = INT_TO_JSONVALUE(value[i].Kills);
				root["ParamsResult"][kName][i]["Name"] = StringHelper::toNarrowString(value[i].Name);
			}
		}
	}

	if (newEvent->ParamsExplosion.size() > 0)
	{
		// Write a DICTIONARY of explosions in JSON FORMAT
		for (PARAMSEXPLOSION_ITERATOR it = newEvent->ParamsExplosion.begin(); it != newEvent->ParamsExplosion.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			GameState::PlayerExplosion value = (*it).second;
			root["ParamsExplosion"][kName]["objectSpacePosition"]["X"] = value.ObjectSpacePosition.X;
			root["ParamsExplosion"][kName]["objectSpacePosition"]["Y"] = value.ObjectSpacePosition.Y;
			root["ParamsExplosion"][kName]["objectSpacePosition"]["Z"] = value.ObjectSpacePosition.Z;
			root["ParamsExplosion"][kName]["gpsSpacePosition"]["X"] = value.GpsSpacePosition.X;
			root["ParamsExplosion"][kName]["gpsSpacePosition"]["Y"] = value.GpsSpacePosition.Y;
			root["ParamsExplosion"][kName]["gpsSpacePosition"]["Z"] = value.GpsSpacePosition.Z;
			root["ParamsExplosion"][kName]["delay"] = value.Delay;
			root["ParamsExplosion"][kName]["radius"] = value.Radius;
			root["ParamsExplosion"][kName]["damage"] = value.Damage;
			root["ParamsExplosion"][kName]["attackerClientID"] = INT_TO_JSONVALUE(value.AttackerClientID);
		}
	}

	if (newEvent->ParamsLong.size() > 0)
	{
		// Write a DICTIONARY of WHOLE NUMBERS (long) in JSON FORMAT
		for (PARAMSLONG_ITERATOR it = newEvent->ParamsLong.begin(); it != newEvent->ParamsLong.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			long value = (*it).second;

			root["ParamsLong"][kName] = INT_TO_JSONVALUE(value);
		}
	}

	if (newEvent->ParamsDouble.size() > 0)
	{
		// Write a DICTIONARY of FLOATING POINT NUMBERS (double) in JSON FORMAT
		for (PARAMSDOUBLE_ITERATOR it = newEvent->ParamsDouble.begin(); it != newEvent->ParamsDouble.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			double value = (*it).second;

			root["ParamsDouble"][kName] = value;
		}
	}

	if (newEvent->ParamsString.size() > 0)
	{
		// Write a DICTIONARY of STRINGS in JSON FORMAT
		for (PARAMSSTRING_ITERATOR it = newEvent->ParamsString.begin(); it != newEvent->ParamsString.end(); ++it)
		{
			std::string keyName = StringHelper::toNarrowString((*it).first);
			const char *kName = keyName.c_str();
			std::string value = StringHelper::toNarrowString((*it).second);

			root["ParamsString"][kName] = value;
		}
	}


	std::string jsonText = writer.write(root);

	if (_localNetworkObject != nullptr)
	{
		std::vector<unsigned char> jsonBytes(jsonText.begin(), jsonText.end());

		_localNetworkObject->ServerSendJsonData(clientID, jsonBytes, jsonText.length());
	}
}

void GameNetworkServer::DebugChangeServerState(GameState::State newState)
{
	_gameState->serverState = newState;
}

void GameNetworkServer::Update()
{
	while (threadStarted)
	{
		struct timespec tim, tim2;
		tim.tv_sec = 0;
		tim.tv_nsec = 1000000L;

		if (nanosleep(&tim, &tim2) < 0)
		{
			printf("Nano sleep system call failed \n");
			return;
		}

		networkTime.Update();

		//if (firstUpdate)
		//{
		//	StartHost();
		//	firstUpdate = false;
		//}

		switch (_gameState->serverState)
		{
		case GameState::State::WaitingPlayers:
			UpdateWaitingForPlayers();
			break;
		case GameState::State::Playing:
			UpdatePlaying();
			break;
		case GameState::State::Results:
			UpdateResults();
			break;
		case GameState::State::ServerRestart:
			UpdateServerRestart();
			break;
		}
	}

}

void GameNetworkServer::UpdateWaitingForPlayers()
{
	//the server should wait for all players to verify that they are "ready"
	bool allReady = true;
	for (int i = 0; i < _gameState->players.size(); i++)
	{
		if (!_gameState->players[i]->clientReady)
		{
			allReady = false;
			//Debug.Log("player is not ready: " + _gameState.players[i].clientID);
		}
	}
	if (allReady)
	{
		// Remember the number of GAME CLIENTS at the start.
		_numberOfClientsAtStart = numberOfClientsConnected;

		// Remember the time the game started.
		_timeAtStart = networkTime.getCurrentTime();

		//change the server state to Playing
		//send info to all player that they can start
		DEBUG_LOG(std::wstring(L"All ") + std::to_wstring(_numberOfClientsAtStart) + std::wstring(L" ready!"));
		//initialise game data things
		InitGameState(GameState::GameMode::DeathMatch);
		_gameState->serverState = GameState::State::Playing;
		//send out the base/respawn positions
		SendUpdateRespawnPositions();
		//SendUpdateServerState(_gameState.serverState);
		_lastPlayerStateUpdateTime = networkTime.getTimeSinceLevelLoad();
	}
}

void GameNetworkServer::InitGameState(GameState::GameMode mode)
{
	//initialise things ready for game start (may be different in different game modes)         
	int maxPickups = _gameState->players.size() / 2;
	maxPickups = Mathf::Clamp(maxPickups, 1, 4);
	//initialise the pickup spawning

	for (int i = 0; i < _gameState->playerPickupPoints.size(); i++)
	{
		//set positions, (design doc says they are hard coded to these positions, may vary in other game modes?)
		//special genus pickups
		if (i == 0)
		{
			GameState::HotGenVec3 tempVar(0.0f, 6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::SpecialItem;
			DEBUG_LOG(std::wstring(L"--------pickup Spawn location 1:") + std::to_wstring(_gameState->playerPickupPoints[i].position.X) + std::wstring(L",") + std::to_wstring(_gameState->playerPickupPoints[i].position.Y));
		}
		if (i == 1)
		{
			GameState::HotGenVec3 tempVar2(6.10f, 0.0f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar2);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::SpecialItem;
		}
		if (i == 2)
		{
			GameState::HotGenVec3 tempVar3(0.0f, -6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar3);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::SpecialItem;
		}
		if (i == 3)
		{
			GameState::HotGenVec3 tempVar4(-6.10f, 0.0f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar4);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::SpecialItem;
		}

		//refill genus pickups
		if (i == 4)
		{
			GameState::HotGenVec3 tempVar5(6.10f, 6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar5);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::Refill;
		}
		if (i == 5)
		{
			GameState::HotGenVec3 tempVar6(-6.10f, 6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar6);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::Refill;
		}
		if (i == 6)
		{
			GameState::HotGenVec3 tempVar7(6.10f, -6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar7);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::Refill;
		}
		if (i == 7)
		{
			GameState::HotGenVec3 tempVar8(-6.10f, -6.10f, 0.0f);
			_gameState->playerPickupPoints[i].position = GameState::HotGenVec3::Add(_gameState->baseStationPosition, tempVar8);
			_gameState->playerPickupPoints[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::Refill;
		}
		_gameState->playerPickupPoints[i].InUse = false;

	}
	for (int i = 0; i < GameState::MAX_REFILL_PICKUPS; i++)
	{
		_gameState->playerPickups[i].SpawnTime = networkTime.getTimeSinceLevelLoad();
		_gameState->playerPickups[i].PickupAvailable = false;
		_gameState->playerPickups[i].Enabled = i < maxPickups;
		_gameState->playerPickups[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::SpecialItem;
	}
	for (int i = GameState::MAX_REFILL_PICKUPS; i < _gameState->playerPickups.size(); i++)
	{
		_gameState->playerPickups[i].SpawnTime = networkTime.getTimeSinceLevelLoad();
		_gameState->playerPickups[i].PickupAvailable = false;
		_gameState->playerPickups[i].Enabled = (i - GameState::MAX_REFILL_PICKUPS) < maxPickups;
		_gameState->playerPickups[i].pickupGenus = GameState::PlayerPickupPoint::PickupGenus::Refill;
	}
}

std::vector<int> GameNetworkServer::GetPlayersWithinRange(GameState::HotGenVec3 *gpsSpacePosition, double radius)
{
	std::vector<int> result;

	// Loop through all the players.
	for (int j = 0; j < _gameState->players.size(); j++)
	{
		GamePlayerState *targetPlayer = _gameState->players[j];
		GameState::HotGenVec3 *targetPos = &(targetPlayer->clientPosition);
		GameState::HotGenVec3 *diffPos = new GameState::HotGenVec3();
		diffPos->X = targetPos->X - gpsSpacePosition->X;
		diffPos->Y = targetPos->Y - gpsSpacePosition->Y;
		diffPos->Z = targetPos->Z - gpsSpacePosition->Z;

		DEBUG_LOG(std::wstring(L"GAME NETWORK SERVER: diffPos X ") + std::to_wstring(diffPos->X) + std::wstring(L" Y ") + std::to_wstring(diffPos->Y) + std::wstring(L" Z ") + std::to_wstring(diffPos->Z) + std::wstring(L" length ") + std::to_wstring(diffPos->Length()) + std::wstring(L" radius ") + std::to_wstring(radius));

		// If this player were within range of the explosion,
		if (diffPos->Length() < radius)
		{
			result.push_back(targetPlayer->clientID);
		}
	}

	return result;
}

/// <summary>
/// Removes players whose GAME CLIENTS have disconnected for too long,
/// from the list of active players
/// </summary>
void GameNetworkServer::UpdateDisconnectedPlayers()
{
	float currentTime = networkTime.getCurrentTime();

    // Loop through all the existing GAME CLIENTS
    for (int i = 0; i < _gameState->playerClients.size(); i++)
    {
        // If this existing CLIENT had the old ID
        if (_gameState->players[i]->clientIsDisconnected)
        {
            if ((currentTime - _gameState->players[i]->clientDisconnectTime) > DEFAULT_TIME_LEFT_TO_RECONNECT)
            {
                DEBUG_LOG(L"GameNetworkServer: Timed-out waiting for Client " +
                    std::to_wstring(_gameState->playerClients[i]) + L" to reconnect");
                // Remove the details of that CLIENT.
                _gameState->PlayersRemoveAt(i);
                _gameState->PlayerClientsRemoveAt(i);

				numberOfClientsConnected--;
            }
        }
    }
}

void GameNetworkServer::UpdatePlaying()
{
	UpdateDisconnectedPlayers();

	// Loop through all the current explosions
	for (int i = _gameState->playerExplosions.size() - 1; i > -1; i--)
	{
		// Update the time left on the explosion
		// If the current time minus the time it started was greater than the delay,
		time_t timeDiff = networkTime.getCurrentTime() - _gameState->playerExplosionsStartTimes[i];
		if ( timeDiff > _gameState->playerExplosions[i]->Delay)
		{
			DEBUG_LOG(L"playerExplosions " + std::to_wstring(i) + L" Delay " + std::to_wstring(_gameState->playerExplosions[i]->Delay) + L" CurrTime-StartTime " + std::to_wstring(timeDiff));
			// Remove the delay.
			_gameState->playerExplosions[i]->Delay = 0.0f;
		}

		// If no time were left on the explosion
		if (_gameState->playerExplosions[i]->Delay <= 0.0f)
		{
			GameState::PlayerExplosion *plrExplosion = _gameState->playerExplosions[i];

			std::vector<int> targetClientIDs = GetPlayersWithinRange(&(plrExplosion->GpsSpacePosition), plrExplosion->Radius);
			for (int j = 0; j < targetClientIDs.size(); j++)
			{
				ArbitrateAndSendDamage(targetClientIDs[j], static_cast<int>(plrExplosion->AttackerClientID), DamageType::Explosion, static_cast<int>(plrExplosion->Damage));
			}

			// Remove the explosion.
			_gameState->PlayerExplosionsRemoveAt(i);
		}
	}

	//itterate through each player and do a small amount of logic on them
	for (int i = 0; i < _gameState->players.size(); i++)
	{
		if (_gameState->players[i]->armour <= 0)
		{
			//player is dead, check if they can respawn
			//check the distance from the players respawn point(base)
			GameState::HotGenVec3 RelPos = GameState::HotGenVec3::Subtract(_gameState->playerRespawnPoints[(int)(_gameState->players[i]->clientTeamID)], _gameState->players[i]->clientPosition);
			if (RelPos.LengthSQ() < RespawnZoneRadSq)
			{
				RespawnPlayer(i);
			}

		}
	}

	//update pickup spawning and collection

	int maxPickups = _gameState->players.size() / 2;
	maxPickups = Mathf::Clamp(maxPickups, 1, 4);
	int availableSpawnLocations = 0;
	for (int i = 0; i < _gameState->playerPickupPoints.size(); i++)
	{
		if (!_gameState->playerPickupPoints[i].InUse)
		{
			availableSpawnLocations++;
		}
	}
	for (int i = 0; i < _gameState->playerPickups.size(); i++)
	{
		if (_gameState->playerPickups[i].Enabled)
		{
			//see if this pickup is currently available
			if (_gameState->playerPickups[i].PickupAvailable)
			{
				//check to see if a player is collecting this pickup
				for (int playerIndex = 0; playerIndex < _gameState->players.size(); playerIndex++)
				{
					//for each player, first check they are alive
					if (_gameState->players[playerIndex]->armour > 0)
					{
						//now check if they intersect the pickup
						GameState::HotGenVec3 RelPos = GameState::HotGenVec3::Subtract(_gameState->playerPickupPoints[_gameState->playerPickups[i].SpawnPointID].position, _gameState->players[playerIndex]->clientPosition);
						if (RelPos.LengthSQ() < PickupRadSq)
						{
							DEBUG_LOG(L"--------Pickup Collected----------");
							//collect the pickup
							//TODO! send message to collecting player

							//disable the pickup
							_gameState->playerPickups[i].PickupAvailable = false;
							_gameState->playerPickups[i].SpawnTime = networkTime.getTimeSinceLevelLoad();
							//free up the spawn location
							_gameState->playerPickupPoints[_gameState->playerPickups[i].SpawnPointID].InUse = false;
							PickupStateChanged(i);
						}
					}
				}
						//see if this pickup needs to UnSpawn
				if (networkTime.getTimeSinceLevelLoad() - _gameState->playerPickups[i].SpawnTime > PickupUnspawnTime)
				{
					//unspawn
					_gameState->playerPickups[i].PickupAvailable = false;
					_gameState->playerPickups[i].SpawnTime = networkTime.getTimeSinceLevelLoad();
					//free up the spawn location
					_gameState->playerPickupPoints[_gameState->playerPickups[i].SpawnPointID].InUse = false;
					PickupStateChanged(i);
				}
			}
			else
			{
				//see if this pickup needs to respawn
				if (networkTime.getTimeSinceLevelLoad() - _gameState->playerPickups[i].SpawnTime > PickupRespawnTime)
				{
					_gameState->playerPickups[i].PickupAvailable = true;
					_gameState->playerPickups[i].SpawnTime = networkTime.getTimeSinceLevelLoad();
					//choose a pickup type..
					_gameState->playerPickups[i].type = _gameState->SpawnFrequencyTable[RandomRange(0, _gameState->SpawnFrequencyTable.size())];
					//find a free spawn location (randomly)
					int chosenID = RandomRange(0, availableSpawnLocations);
					int currentID = 0;
					for (int j = 0;j < _gameState->playerPickupPoints.size();j++)
					{
						if (!_gameState->playerPickupPoints[j].InUse && _gameState->playerPickupPoints[j].pickupGenus == _gameState->playerPickups[i].pickupGenus)
						{
							if (chosenID == currentID)
							{
								_gameState->playerPickups[i].SpawnPointID = j;
								_gameState->playerPickupPoints[j].InUse = true;
								break;
							}
							currentID++;
						}
					}
					PickupStateChanged(i);
					//_gameState.playerPickupPoints[_gameState.playerPickups[i].SpawnPointID].InUse = false;
				}
			}
		}

	}

	bool isGameOver = false;

	//see if a player has achieved the required scroe for game completion
	for (int playerIndex = 0; playerIndex < _gameState->players.size(); playerIndex++)
	{
		if (_gameState->players[playerIndex]->kills >= RequiredKillCount)
		{
			// End the game
			isGameOver = true;
			break;
		}
	}

	// If a limit had been set for the time for the game,
	if (_gameState->serverGameModeTimeLimit > 0)
	{
		// Get how long the game has lasted.
		float gameDuration = networkTime.getCurrentTime() - _timeAtStart;

		// If the time exceeds the limit,
		if (gameDuration > _gameState->serverGameModeTimeLimit)
		{
			// End the game.
			isGameOver = true;
		}
	}

	// If the number of players left connected had reached the minimum,
	// and the number started above the minimum,
	if (numberOfClientsConnected == 0 ||
		(numberOfClientsConnected <= MIN_CLIENT_NUMBER_PLAYING &&
		_numberOfClientsAtStart > MIN_CLIENT_NUMBER_PLAYING))
	{
		// End the game.
		//
		// NOTE: if only 1 played started the game, then it was probably
		//       for DEBUGGING purposes. So the game should not end
		//       because there is only 1 player left.
		isGameOver = true;
	}

	if (isGameOver)
	{
		// Switch to displaying the results
		_gameState->serverState = GameState::State::Results;

		// Send a GAME OVER EVENT.
		SendGameOver();
	}
}

void GameNetworkServer::UpdateResults()
{
	//when everybody has disconnected, restart the server
	//NOTE! when using unity we will always have 1 client (us) but with the basestation system we should have 0

	if (numberOfClientsConnected <= MIN_CLIENT_NUMBER_RESULTS)
	{
		DEBUG_LOG(L"----resetting server!-----");
		//StopHost();
		_gameState->serverState = GameState::State::ServerRestart;

	}

}

void GameNetworkServer::UpdateServerRestart()
{
	//StartHost();
	GameNetworkServerWrapper_Restart();
	DEBUG_LOG(L"----server restarted!-----");
	_gameState->serverState = GameState::State::None;
}

void GameNetworkServer::PickupStateChanged(int pickupIndex)
{
	SendUpdatePickupState(pickupIndex);
	//the state of a pickup has changed, tell the players about it
}

void GameNetworkServer::RespawnPlayer(int playerIndex)
{
	_gameState->players[playerIndex]->armour = 100;
	_gameState->players[playerIndex]->clientDisplayType = GamePlayerState::PlayerDisplayType::Normal;
	SendRespawnPlayer(_gameState->players[playerIndex]->clientID);
}

void GameNetworkServer::StopHost()
{
	//CustomNetworkManager::Instance->Stop();
	if (_localNetworkObject != nullptr)
	{
		_localNetworkObject->ClearServerReceiveJsonDataFn();
		_localNetworkObject = nullptr;

		//remove all listeners!
		ClearReceiveEventList();

		if (_gameState)
		{
			// If the GAME SERVER were being shutdown not to be restarted,
			if (_gameState->serverState != GameState::State::ServerRestart)
			{
				// Shutdown all THREADS permanently.
				threadStarted = false;
				if (updateThread->joinable())
				{
					updateThread->join();
				}

				delete updateThread;
				updateThread = nullptr;
			}

			// If the GAME SERVER were being shutdown to be restarted,
			if (_gameState->serverState == GameState::State::ServerRestart)
			{
				//TODO! cant delete, find out why or we will leek :(
				delete _gameState;
				_gameState = 0;

				// Reset all the states of the GAME SERVER.
				_gameState = new GameState();

				// Reset the history of EVENTS.
				_eventHistory = std::vector<GameEvents*>();
			}
		}

	}
	
}

void GameNetworkServer::ClearReceiveEventList()
{
	GameNetworkServer::ReceiveCreateLobby->removeListener(L"GameNetworkServer_ReceiveCreateLobby");
	GameNetworkServer::ReceiveJoinLobby->removeListener(L"GameNetworkServer_ReceiveJoinLobby");
	GameNetworkServer::ReceivePlayerConnect->removeListener(L"GameNetworkServer_ReceivePlayerConnect");
	GameNetworkServer::ReceiveOtherPlayerConnect->removeListener(L"GameNetworkServer_ReceiveOtherPlayerConnect");
	GameNetworkServer::ReceivePlayerReconnect->removeListener(L"GameNetworkServer_ReceivePlayerReconnect");
	GameNetworkServer::ReceivePlayerDisconnect->removeListener(L"GameNetworkServer_ReceivePlayerDisconnect");
	GameNetworkServer::ReceiveChangeTeam->removeListener(L"GameNetworkServer_ReceiveChangeTeam");
	GameNetworkServer::ReceivePlayerRankChange->removeListener(L"GameNetworkServer_ReceivePlayerRankChange");
	GameNetworkServer::ReceiveStart->removeListener(L"GameNetworkServer_ReceiveStart");
	GameNetworkServer::ReceiveUpdatePlayerState->removeListener(L"GameNetworkServer_ReceiveUpdatePlayerState");
	GameNetworkServer::ReceivePlayerWasHit->removeListener(L"GameNetworkServer_ReceivePlayerWasHit");
	GameNetworkServer::ReceiveSetRespawn->removeListener(L"GameNetworkServer_ReceiveSetRespawnPointEvent");
	GameNetworkServer::ReceiveExplosion->removeListener(L"GameNetworkServer_ReceiveExplosion");
}

void GameNetworkServer::SendCreateLobby(const std::wstring &serverLobbyOwner, GameState::HotGenVec3 *BaseStationGPSLocation, int gameModeTimeLimit)
{

}

void GameNetworkServer::SendCreateLobbyFailed()
{

}

void GameNetworkServer::SendJoinLobby(int clientID, const std::wstring &clientPlayerName)
{

}

void GameNetworkServer::SendPlayerConnect(int clientID)
{
	GameEvents *newPlayerConnectEvent = new GameEvents(clientID, GameEvents::eventID::PlayerConnectEvent);
	SendEvent(clientID, newPlayerConnectEvent);
}

void GameNetworkServer::SendOtherPlayerConnect(int clientID, int otherClientID)
{
	GameEvents *newOtherPlayerConnectEvent = new GameEvents(otherClientID, GameEvents::eventID::OtherPlayerConnectEvent);
	SendEvent(clientID, newOtherPlayerConnectEvent);
}

void GameNetworkServer::SendPlayerReconnect(int clientID, int clientOldID)
{
    GameEvents *newPlayerReconnectEvent = new GameEvents(clientID, GameEvents::eventID::PlayerReconnectEvent);
    newPlayerReconnectEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientOldID], clientOldID);
    SendEvent(clientID, newPlayerReconnectEvent);
}


void GameNetworkServer::SendPlayerDisconnect(int clientID, float clientTimeLeftToReconnect)
{
	GameEvents *newPlayerDisconnectEvent = new GameEvents(clientID, GameEvents::eventID::PlayerDisconnectEvent);
	newPlayerDisconnectEvent->ParamsDoubleAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientTimeLeftToReconnect], clientTimeLeftToReconnect);
	_eventHistory.push_back(newPlayerDisconnectEvent);
	SendEvent(newPlayerDisconnectEvent);
}

void GameNetworkServer::SendSetGameMode(GameState::GameMode serverGameMode)
{

}

void GameNetworkServer::SendChangeGameMode(GameState::GameMode serverGameMode)
{

}

void GameNetworkServer::SendChangeTeam(GamePlayerState::Team clientTeamID)
{

}

void GameNetworkServer::SendStart()
{
	//sends an event which triggers clients to start the game

	DEBUG_LOG(L"sending start event all");

	GameEvents *newStartEvent = new GameEvents(_gameState->playerClients[0], GameEvents::eventID::StartEvent);

	SendEvent(newStartEvent);

}

void GameNetworkServer::SendAbort()
{

}

void GameNetworkServer::SendPlayerRankChange(bool clientIsCaptain)
{

}

void GameNetworkServer::SendUpdateServerState(GameState::State newState)
{
	//updates the "state" paramater of gamestate
	DEBUG_LOG(L"sending new sub state to all");

	GameEvents *newStartEvent = new GameEvents(_gameState->playerClients[0], GameEvents::eventID::UpdateGameStateEvent);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)(newState));
	SendEvent(newStartEvent);
}

void GameNetworkServer::SendUpdateRespawnPositions()
{
	//updates the required paramaters of gamestate
	DEBUG_LOG(L"sending new game state info to all");

	GameEvents *newStartEvent = new GameEvents(_gameState->playerClients[0], GameEvents::eventID::UpdateGameStateEvent);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)(_gameState->serverState));

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::serverRespawnPointTeam1], (_gameState->playerRespawnPoints[0]));
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::serverRespawnPointTeam2], (_gameState->playerRespawnPoints[1]));

	SendEvent(newStartEvent);
}

void GameNetworkServer::SendUpdatePickupState(int pickupID)
{
	DEBUG_LOG(L"sending new pickup state info to all");

	GameEvents *newStartEvent = new GameEvents(GameEvents::eventID::UpdateGameStateEvent);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)(_gameState->serverState));

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::PickupID], (long)pickupID);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::PickupType], (long)_gameState->playerPickups[pickupID].type);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::PickupAvailable], (long)_gameState->playerPickups[pickupID].PickupAvailable);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newStartEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::PickupPosition], (_gameState->playerPickupPoints[_gameState->playerPickups[pickupID].SpawnPointID].position));
	SendEvent(newStartEvent);
}

void GameNetworkServer::SendUpdateGameState(GameState *newState)
{

}

void GameNetworkServer::SendGoToAmmo(bool clientHasReachedAmmo)
{

}

void GameNetworkServer::SendGoToBase(bool clientHasReachedBase)
{

}

void GameNetworkServer::SendFire(GameState::PlayerFire *serverLastPlayerFire)
{

}

void GameNetworkServer::SendPlayerWasHit(int clientLastAttackerID)
{
}

void GameNetworkServer::SendPlayerTakeDamage(int targetID, int sourceID, GameState::HotGenVec3 *sourcePosition, int newArmourValue)
{
	//when the server sends a was hit event, we speficy the actual damage done (by sending the new armour value) and the source location (source location used to show damage direction)
	GameEvents *newEvent = new GameEvents(targetID, GameEvents::eventID::PlayerWasHitEvent);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::DamageSrc], (long)sourceID);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientLastArmourDamage], (long)newArmourValue);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::SrcPosition], *sourcePosition);


	SendEvent(targetID, newEvent);

    // If the target were different to the source,
    if (targetID != sourceID)
    {
        // Send this also to source!
        SendEvent(sourceID, newEvent);
    }

}

void GameNetworkServer::SendOtherPlayerWasHit(int clientID, int clientLastArmourDamage, int clientLastAttackerID)
{

}

void GameNetworkServer::SendUpdatePlayerStateReady(bool value)
{

}

void GameNetworkServer::SendUpdatePlayerState(GamePlayerState newState)
{

}

void GameNetworkServer::SendUpdateOtherPlayerStatePositionAndHeading(int clientID)
{
	//use this function when we want to inform players about the status of other players (not about a change in their own state)
	DEBUG_LOG(L"sending position data");

	int playerIndex = FindPlayerIndexFromClientID(clientID);
	if (playerIndex < 0)
	{
		DEBUG_LOGERROR(L"GAME NETWORK SERVER: invalid client id " + std::to_wstring(clientID) + L" found or player does not exist!");
		return;
	}

	GameEvents *newEvent = new GameEvents(clientID, GameEvents::eventID::UpdateOtherPlayersStateEvent);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::clientPosition], (_gameState->players[playerIndex]->clientPosition));

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsDoubleAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientHeading], _gameState->players[playerIndex]->clientHeading);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::clientDisplayType], (long)_gameState->players[playerIndex]->clientDisplayType);


	//todo, rotation
	SendEvent(newEvent);
}

void GameNetworkServer::SendUpdateOtherPlayersState(int clientID, GamePlayerState newState)
{

}

void GameNetworkServer::SendReloadClip()
{

}

void GameNetworkServer::SendPickedUpItem(GamePlayerState::PickableItems clientLastPickedUpItem)
{

}

void GameNetworkServer::SendPlayerDeath(int clientID, int clientLastAttackerID)
{

}

void GameNetworkServer::SendOtherPlayerDied(int clientID, int clientLastAttackerID)
{

}

void GameNetworkServer::SendRespawnPlayer(int clientID)
{
	//sets the position of the players team base (respawn point)..used for acknoledgement 
	DEBUG_LOG(L"sending base position data");
	GameEvents *newEvent = new GameEvents(clientID, GameEvents::eventID::RespawnPlayerEvent);
	SendEvent(clientID,newEvent);
}

void GameNetworkServer::SendSetBase(GameState::HotGenVec3 *serverLastBasePoint)
{

}

void GameNetworkServer::SendSetRespawn(int clientID, GameState::HotGenVec3 *serverLastRespawnPoint)
{
	//sets the position of the players team base (respawn point)..used for acknoledgement 
	DEBUG_LOG(L"sending base position data");
	GameEvents *newEvent = new GameEvents(clientID, GameEvents::eventID::SetRespawnPointEvent);

//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsVec3Add(GameEvents::eventDataString[(int)GameEvents::eventData::serverLastRespawnPoint], *serverLastRespawnPoint);

	//todo, rotation
	SendEvent(clientID,newEvent);
}

void GameNetworkServer::SendSetAmmo(Vector3 *serverLastAmmoPoint)
{

}

void GameNetworkServer::SendSetBomb(Vector3 *serverLastBombPoint)
{

}

void GameNetworkServer::SendGameOver()
{
	//sets the position of the players team base (respawn point)..used for acknoledgement 
	DEBUG_LOG(L"sending game over event");
	GameEvents *newEvent = new GameEvents(-1, GameEvents::eventID::GameOverEvent);
	std::vector<GameState::GameResult> results;
	//add in all the results data
	for (int playerIndex = 0; playerIndex < _gameState->players.size(); playerIndex++)
	{
		GameState::GameResult playerResult(_gameState->players[playerIndex]->clientPlayerName, _gameState->players[playerIndex]->kills, _gameState->players[playerIndex]->deaths, _gameState->players[playerIndex]->clientID);
		results.push_back(playerResult);
	}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsResultAdd(GameEvents::eventDataString[(int)GameEvents::eventData::GameResults], results);
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
	newEvent->ParamsLongAdd(GameEvents::eventDataString[(int)GameEvents::eventData::serverState], (long)(_gameState->serverState));

	//send to all players
	SendEvent(newEvent);
}

void GameNetworkServer::SendFlagPickedUp()
{

}

void GameNetworkServer::SendFlagDropped()
{

}

void GameNetworkServer::SendFlagReturned()
{

}

void GameNetworkServer::SendRetry()
{

}

void GameNetworkServer::SendNewMap(GameState::GameMedia *serverMap)
{

}

void GameNetworkServer::SendNewPlayerProfile(GameState::GameMedia *serverLastPlayerProfile)
{

}

void GameNetworkServer::SendNewVoiceData(GameState::GameMedia *serverVoice)
{

}

void GameNetworkServer::SendExplosion(GameState::PlayerExplosion *serverLastExplosion)
{
}

void GameNetworkServer::SendNewSpawnFire(GameState::PlayerFire *serverLastPlayerFire)
{

}

void GameNetworkServer::SendNewSpawnMine(GameState::PlayerMine *serverLastPlayerMine)
{

}

void GameNetworkServer::SendNewSpawnHomingMine(GameState::PlayerHomingMine *serverLastPlayerHomingMine)
{

}

void GameNetworkServer::SendNewScrambleMapOn()
{

}

void GameNetworkServer::SendNewScrambleMapOff()
{

}
