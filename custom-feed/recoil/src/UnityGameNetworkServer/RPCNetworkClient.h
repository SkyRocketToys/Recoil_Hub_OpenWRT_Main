
#include "ICustomNetworkClient.h"
#include "GamePlayerState.h"

/// <summary>
/// RPCNetworkClient.cs
/// 
/// Description:
/// Controls the sending of data from the GAME SERVER or HOST to a GAME CLIENT, and vice versa.
/// 
/// Created: 17/01/17
/// Authors: Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>
class RPCNetworkClient : public ICustomNetworkClient
{
public:

	RPCNetworkClient();
	~RPCNetworkClient();

	unsigned int GetNetIdValue();
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
	bool getIsServer() const;

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
	bool getIsClient() const;

	/// <summary>
	/// Flag. TRUE if this GAME OBJECT was controlling the local PEER in a PEER TO PEER NETWORK.
	/// There is a BUG with the UNITY ENGINE which means using
	/// 
	///     Network.isLocalPlayer
	/// 
	/// 
	/// is not realiable. It is sometimes not set, even when the OBJECT is controlling the local PEER.
	/// </summary>
	bool getIsLocalPlayer() const;

	int getID() const;

	float getHostTimestamp() const;

	float getClientTimestamp() const;

	GamePlayerState *getPlayerState() const;

	void setPlayerState(GamePlayerState *value);


	TangibleEvent<CmdServerReceiveJsonDataFn> *ServerReceiveJsonData;

	void AddServerReceiveJsonDataFn(CmdServerReceiveJsonDataFn newReceiveJsonDataFn);

	void AddClientReceiveJsonDataFn(RpcClientReceiveJsonDataFn newReceiveJsonDataFn);

	void ClearServerReceiveJsonDataFn();

	void ClearClientReceiveJsonDataFn();

	void Setup(int clientID);

	/// <summary>
	/// Receives commands, from the GAME CLIENT, to the GAME SERVER
	/// </summary>
	/// <param name="clientID"></param>
	/// <param name="jsonBytes"></param>
	/// <param name="jsonDataLength"></param>
	//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
	//ORIGINAL LINE: [Command] public void CmdServerReceiveJsonData(int clientID, byte[] jsonBytes, int jsonDataLength)
	void CustomServerReceiveJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength);

	/// <summary>
	/// Receives commands, from the GAME SERVER, to this GAME CLIENT
	/// </summary>
	/// <param name="jsonBytes"></param>s
	/// <param name="jsonDataLength"></param>
	//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
	//ORIGINAL LINE: [ClientRpc] public void RpcClientReceiveJsonData(byte[] jsonBytes, int jsonDataLength)
	void CustomClientReceiveJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength);

	bool ServerSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength);

	bool ServerSendJsonData(int clientID, std::vector<unsigned char> &jsonBytes, int jsonDataLength);

	bool ClientSendJsonData(std::vector<unsigned char> &jsonBytes, int jsonDataLength);

private:
	//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
	//ORIGINAL LINE: [SyncVar] private float _hostTimestamp;
	float _hostTimestamp = 0.0f;

	//C# TO C++ CONVERTER NOTE: The following .NET attribute has no direct equivalent in native C++:
	//ORIGINAL LINE: [SerializeField] private GamePlayerState _playerState = new GamePlayerState();
	GamePlayerState *_playerState;

};