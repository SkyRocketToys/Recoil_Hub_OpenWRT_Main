//.h file code:

#include <vector>
#include "IGameNetworkEvent.h"

#ifndef _IGAME_NETWORK_SERVER_H
#define _IGAME_NETWORK_SERVER_H

/// <summary>
/// IGameNetworkServer.cs
/// 
/// Description:
/// Defines the data and PROCEDURES or functions of a CLASS that allows
/// GAME SERVERS built with RECOIL NETWORKING to communicate.
/// 
/// Created:  18/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class IGameNetworkServer : public IGameNetworkEvent
{

public:
	virtual void StartHost() = 0;

	virtual void Setup(int newClientID) = 0;

	virtual void OnStartClient(int newClientID, int oldClientID) = 0;

	virtual void OnClientDisconnect(int oldClientID) = 0;

	virtual float getServerTimestamp() const = 0;

	virtual void CmdServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual bool ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual bool ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual void Update() = 0;

	virtual void StopHost() = 0;


};


#endif