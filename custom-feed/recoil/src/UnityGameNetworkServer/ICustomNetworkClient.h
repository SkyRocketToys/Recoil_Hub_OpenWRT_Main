//.h file code:

#include <vector>
#include "tangible_event.h"
#include "GamePlayerState.h"

#ifndef _ICUSTOM_NETWORK_CLIENT_H
#define _ICUSTOM_NETWORK_CLIENT_H

/// <summary>
/// ICustomNetworkClient.cs
/// 
/// Description:
/// Defines the data and PROCEDURES or functions of a CLASS that allows
/// APPS built with UNITY NETWORKING to communicate.
/// 
/// This was initially written to allow you to swap between using one of
/// two techniques available in UNITY NETWORKING, for communicating between
/// APPS i.e
/// 
///     RPC
///     SYNC VARS
///     
/// Created:  17/01/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
using CmdServerReceiveJsonDataFn = std::function<void (int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength)>;
using RpcClientReceiveJsonDataFn = std::function<void (std::vector<unsigned char> &jsonBytes, int jsonDataLength)>;

class ICustomNetworkClient //: NetworkBehaviour
{

public:

	virtual unsigned int GetNetIdValue() = 0;
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
	virtual bool getIsServer() const = 0;

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
	virtual bool getIsClient() const = 0;

	/// <summary>
	/// Flag. TRUE if this GAME OBJECT was controlling the local PEER in a PEER TO PEER NETWORK.
	/// There is a BUG with the UNITY ENGINE which means using
	/// 
	///     Network.isLocalPlayer
	/// 
	/// 
	/// is not realiable. It is sometimes not set, even when the OBJECT is controlling the local PEER.
	/// </summary>
	virtual bool getIsLocalPlayer() const = 0;

	virtual int getID() const = 0;

	virtual float getHostTimestamp() const = 0;

	virtual float getClientTimestamp() const = 0;

	virtual GamePlayerState *getPlayerState() const = 0;

	virtual void setPlayerState(GamePlayerState *value) = 0;

	virtual void AddServerReceiveJsonDataFn(CmdServerReceiveJsonDataFn newReceiveJsonDataFn) = 0;

	virtual void AddClientReceiveJsonDataFn(RpcClientReceiveJsonDataFn newReceiveJsonDataFn) = 0;

	virtual void ClearServerReceiveJsonDataFn() = 0;

	virtual void ClearClientReceiveJsonDataFn() = 0;

	virtual void Setup(int clientID) = 0;

	/// <summary>
	/// Receives commands, from the GAME CLIENT, to the GAME SERVER
	/// </summary>
	/// <param name="clientID"></param>
	/// <param name="jsonBytes"></param>
	/// <param name="jsonDataLength"></param>
	virtual void CustomServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	/// <summary>
	/// Receives commands, from the GAME SERVER, to this GAME CLIENT
	/// </summary>
	/// <param name="jsonBytes"></param>
	/// <param name="jsonDataLength"></param>
	virtual void CustomClientReceiveJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual bool ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual bool ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

	virtual bool ClientSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength) = 0;

};

#endif