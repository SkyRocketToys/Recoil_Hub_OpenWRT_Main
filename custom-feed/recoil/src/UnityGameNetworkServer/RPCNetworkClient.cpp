#include "RPCNetworkClient.h"
#include "Debug.h"
#include "GameNetworkServerWrapper.h"

//TangibleEvent<CmdServerReceiveJsonDataFn> *RPCNetworkClient::ServerReceiveJsonData = new TangibleEvent<CmdServerReceiveJsonDataFn>();

RPCNetworkClient::RPCNetworkClient()
{
	_playerState = new GamePlayerState();
	ServerReceiveJsonData = new TangibleEvent<CmdServerReceiveJsonDataFn>();
}

RPCNetworkClient::~RPCNetworkClient()
{
	delete _playerState;
}

unsigned int RPCNetworkClient::GetNetIdValue()
{
	return 0;
}

/// <summary>
/// Flag. TRUE if the PEER in a PEER TO PEER NETWORK is acting as a GAME SERVER.
/// There is a BUG with the UNITY ENGINE which means using
/// 
///     Network.isServer
/// 
/// 
/// is not realiable. It is sometimes not set, even when the PEER is acting as
/// a GAME SERVER.
/// </summary>
bool RPCNetworkClient::getIsServer() const
{
	return true;
}

/// <summary>
/// Flag. TRUE if the PEER in a PEER TO PEER NETWORK is acting as a GAME CLIENT.
/// There is a BUG with the UNITY ENGINE which means using
/// 
///     Network.isClient
/// 
/// 
/// is not realiable. It is sometimes not set, even when the PEER is acting as
/// a GAME CLIENT.
/// </summary>
bool RPCNetworkClient::getIsClient() const
{
	return false;
}

/// <summary>
/// Flag. TRUE if this GAME OBJECT was controlling the local PEER in a PEER TO PEER NETWORK.
/// There is a BUG with the UNITY ENGINE which means using
/// 
///     Network.isLocalPlayer
/// 
/// 
/// is not realiable. It is sometimes not set, even when the OBJECT is controlling the local PEER.
/// </summary>
bool RPCNetworkClient::getIsLocalPlayer() const
{
	return false;
}

int RPCNetworkClient::getID() const
{
	return 0;
}

float RPCNetworkClient::getHostTimestamp() const
{
	return 0.0f;
}

float RPCNetworkClient::getClientTimestamp() const
{
	return 0.0f;
}

GamePlayerState *RPCNetworkClient::getPlayerState() const
{
	return _playerState;
}

void RPCNetworkClient::setPlayerState(GamePlayerState *value)
{
	_playerState = value;
}

void RPCNetworkClient::AddServerReceiveJsonDataFn(CmdServerReceiveJsonDataFn newReceiveJsonDataFn)
{
	RPCNetworkClient::ServerReceiveJsonData->addListener(L"newReceiveJsonDataFn", newReceiveJsonDataFn);

}

void RPCNetworkClient::AddClientReceiveJsonDataFn(RpcClientReceiveJsonDataFn newReceiveJsonDataFn)
{
	// Not implemented for GAME SERVER
}

void RPCNetworkClient::ClearServerReceiveJsonDataFn()
{
	for (CmdServerReceiveJsonDataFn d : ServerReceiveJsonData->listeners())
	{
		ServerReceiveJsonData->removeListener(L"d");
	}
}

void RPCNetworkClient::ClearClientReceiveJsonDataFn()
{
	// Not implemented for GAME SERVER
}

void RPCNetworkClient::Setup(int clientID)
{
	// Not implemented for GAME SERVER
}

// RAJ's tool should send data from the CLIENTS through this.
void RPCNetworkClient::CustomServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	std::wstring jsonText(jsonBytes.begin(), jsonBytes.end());

	if (ServerReceiveJsonData != nullptr)
	{
		for (auto listener : ServerReceiveJsonData->listeners())
		{
			listener(clientID, jsonBytes, jsonDataLength);
		}
	}

	DEBUG_LOG(std::wstring(L"RPC NETWORK CLIENT: CustomServerReceiveJsonData Client ") + std::to_wstring(clientID) + std::wstring(L" Size ") + std::to_wstring(jsonText.length()) + std::wstring(L" Text ") + std::wstring(jsonText.begin(), jsonText.end()));
}

void RPCNetworkClient::CustomClientReceiveJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	// Not implemented for GAME SERVER
}

bool RPCNetworkClient::ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	// Not implemented for GAME SERVER
	return false;
}

bool RPCNetworkClient::ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	// If data were being sent to the first CLIENT,
	if (clientID == 0)
	{
		// Display DEBUGGING information about the data being sent.
		std::wstring jsonText(jsonBytes.begin(), jsonBytes.end());

		DEBUG_LOG(std::wstring(L"RPC NETWORK CLIENT: ServerSendJsonData Client ") + std::to_wstring(clientID) + std::wstring(L" Size ") + std::to_wstring(jsonText.length()) + std::wstring(L" Text ") + std::wstring(jsonText.begin(), jsonText.end()));
	}

	GameNetworkServerWrapper_RecoilServerSendJsonData(clientID, jsonBytes.data(), jsonDataLength);
	return false;
}

bool RPCNetworkClient::ClientSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength)
{
	// Not implemented for GAME SERVER
	return false;
}
