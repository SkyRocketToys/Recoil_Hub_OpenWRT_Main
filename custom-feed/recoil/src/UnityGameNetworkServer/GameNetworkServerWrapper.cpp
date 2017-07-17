//.cpp file code:

#include "GameNetworkServer.h"
#include "GameNetworkServerWrapper.h"
#include <string.h>
/// <summary>
/// GameNetworkServerWrapper.cpp
/// 
/// Description:
/// Provides the INTERFACE (written in 'C') for the GAME SERVER, written in RAJ,
/// to interact with the CLASSES used to build the GAME SERVER with the UNITY ENGINE,
/// converted from C# to C+++.
/// 
/// Created:  24/02/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class GameNetworkServerWrapper 
{
public:
	static GameNetworkServerWrapper* UnityGameNetworkServerWrapper;

	GameNetworkServer& GetGameNetworkServer() {
		return _unityGameNetworkServer;
	}

private:
	GameNetworkServer _unityGameNetworkServer;
};

GameNetworkServerWrapper* GameNetworkServerWrapper::UnityGameNetworkServerWrapper = nullptr;

#ifdef __cplusplus
extern "C"
{
#endif
	void GameNetworkServerWrapper_StartHost()
	{
		if (GameNetworkServerWrapper::UnityGameNetworkServerWrapper == nullptr)
		{
			GameNetworkServerWrapper::UnityGameNetworkServerWrapper = new GameNetworkServerWrapper();
		}

		GameNetworkServerWrapper::UnityGameNetworkServerWrapper->GetGameNetworkServer().StartHost();
	}

	void GameNetworkServerWrapper_OnStartClient(int newClientID, int oldClientID)
	{
		GameNetworkServerWrapper::UnityGameNetworkServerWrapper->GetGameNetworkServer().OnStartClient(newClientID, oldClientID);
	}

	void GameNetworkServerWrapper_OnClientDisconnect(int oldClientID)
	{
		GameNetworkServerWrapper::UnityGameNetworkServerWrapper->GetGameNetworkServer().OnClientDisconnect(oldClientID);
	}

	void GameNetworkServerWrapper_StopHost()
	{
		GameNetworkServerWrapper::UnityGameNetworkServerWrapper->GetGameNetworkServer().StopHost();
	}

	void GameNetworkServerWrapper_CustomServerReceiveJsonData(int clientID, unsigned char *jsonBytes, int jsonDataLength)
	{
		std::vector<unsigned char> newJsonBytes(jsonBytes, jsonBytes + jsonDataLength);

		GameNetworkServerWrapper::UnityGameNetworkServerWrapper->GetGameNetworkServer().GetLocalNetworkObject()->CustomServerReceiveJsonData(clientID, newJsonBytes, jsonDataLength);
	}

	bool GameNetworkServerWrapper_RecoilServerSendJsonData(int clientID, unsigned char *jsonBytes, int jsonDataLength)
	{
		unsigned int sent;

		//insert json meta data
		unsigned char* finalPacket = new unsigned char[jsonDataLength + 3];
		memcpy(&finalPacket[3], jsonBytes, jsonDataLength);
		finalPacket[0] = NW_DATA_TYPE_EVENT;
		unsigned short shotjson = (unsigned short)jsonDataLength;
		unsigned char* jlen = (unsigned char*)&shotjson;

#ifdef ENABLE_BIG_ENDIAN_BUILD
		//TODO, endianess might be wrong on actual hardware!!
		//perhaps use #ifdef ENABLE_BIG_ENDIAN_BUILD
		finalPacket[1] = jlen[1];
		finalPacket[2] = jlen[0];
#else
		finalPacket[1] = jlen[0];
		finalPacket[2] = jlen[1];
#endif
		// Insert RAJ's code to send data here.
		NWEVENT_Send(clientID, finalPacket, jsonDataLength + 3, &sent);
		//NWEVENT_Send(clientID, jsonBytes, jsonDataLength, &sent); 
		delete finalPacket;
		return true;
	}

	void GameNetworkServerWrapper_Restart()
	{
		RECOILNetwork_Stop();
#if defined(ENABLE_PRODUCTION) && defined(ENABLE_TEST_RESET_PASSWORD_AND_REBOOT)
		RECOILAPP_ResetBaseStationPassword();
#else
		RECOILNetwork_Start();
#endif
	}


#ifdef __cplusplus
}
#endif


