//.h file code:

#include "../recoilnetwork.h"

#ifndef _GAME_NETWORK_SERVER_WRAPPER_H
#define _GAME_NETWORK_SERVER_WRAPPER_H

/// <summary>
/// GameNetworkServerWrapper.h
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
#ifdef __cplusplus
extern "C"
{
#endif
	void GameNetworkServerWrapper_StartHost();

	void GameNetworkServerWrapper_OnStartClient(int newClientID, int oldClientID);

	void GameNetworkServerWrapper_OnClientDisconnect(int oldClientID);

	void GameNetworkServerWrapper_StopHost();

	void GameNetworkServerWrapper_CustomServerReceiveJsonData(int clientID, unsigned char *jsonBytes, int jsonDataLength);

	bool GameNetworkServerWrapper_RecoilServerSendJsonData(int clientID, unsigned char *jsonBytes, int jsonDataLength);

	void GameNetworkServerWrapper_Restart();
#ifdef __cplusplus
}
#endif


#endif

