
#ifndef RECOIL_NETWORK_H
#define RECOIL_NETWORK_H

#include <syslog.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

//#include <bits/pthreadtypes.h>
#include <pthread.h>

// ------------ HIGH LEVEL FLAGS TO ENABLE

#define ENABLE_TEST_APP                     // enable test app to be embedded in the product
//#define ENABLE_RECOIL_APP                   // enable recoil app to be embedded in the product
#define ENABLE_PRODUCTION                   // enable this to use port range 500XX and 510XX for product
#define ENABLE_BIG_ENDIAN_BUILD             // enable this to make the product run in the AR9331 base station hardware
//#define ENABLE_SYSLOG_LOGGING               // enable this to route logging to syslogd otherwise it will be printf
//#define USE_EXTRA_PORT_FOR_UDP_RECEIVE    // enable this if you want to use seperate handle to send (will be removed soon)

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/
// Recoil System details
#define RECOIL_PRODUCT_ID                   1   // must match with client app
#define RECOIL_PROTOCOL_VERSION             1   // must match with client app

// Recoil System version details
// below version macro represents this format vMajor.Minor.Package; 
// Ex: RECOIL v1.0.1 - Equivalent packet name will be like :- recoil_1.0-1_ar71xx.ipk
#define RECOIL_MAJOR_VERSION                1
#define RECOIL_MINOR_VERSION                0
#define RECOIL_PACKAGE_VERSION              1

// port's used in the system - use anything in this range 49152–65535

extern char *gNetworkDiscoveryPort;                       // TCP
extern char *gNetworkSyncPort;                            // TCP
extern char *gNetworkPingPort;                            // UDP
extern char *gNetworkTimePort;                            // UDP
extern char *gNetworkEventPort;                           // TCP

extern char *gNetworkPingReturnPort;                     // UDP
extern char *gNetworkTimeReturnPort;                     // UDP

#ifdef ENABLE_PRODUCTION

#define NETWORK_DISCOVERY_PORT_RECOILAPP    "50000"     // TCP
#define NETWORK_SYNC_PORT_RECOILAPP         "50001"     // TCP
#define NETWORK_PING_PORT_RECOILAPP         "50002"     // UDP
#define NETWORK_TIME_PORT_RECOILAPP         "50003"     // UDP
#define NETWORK_EVENT_PORT_RECOILAPP        "50004"     // TCP

#define NETWORK_PING_RETURN_PORT_RECOILAPP  "50007"     // UDP
#define NETWORK_TIME_RETURN_PORT_RECOILAPP  "50008"     // UDP

#define NETWORK_DISCOVERY_PORT_TESTAPP      "51000"     // TCP
#define NETWORK_SYNC_PORT_TESTAPP           "51001"     // TCP
#define NETWORK_PING_PORT_TESTAPP           "51002"     // UDP
#define NETWORK_TIME_PORT_TESTAPP           "51003"     // UDP
#define NETWORK_EVENT_PORT_TESTAPP          "51004"     // TCP

#define NETWORK_PING_RETURN_PORT_TESTAPP    "51007"     // UDP
#define NETWORK_TIME_RETURN_PORT_TESTAPP    "51008"     // UDP

#else

#define NETWORK_DISCOVERY_PORT_RECOILAPP    "60000"     // TCP
#define NETWORK_SYNC_PORT_RECOILAPP         "60001"     // TCP
#define NETWORK_PING_PORT_RECOILAPP         "60002"     // UDP
#define NETWORK_TIME_PORT_RECOILAPP         "60003"     // UDP
#define NETWORK_EVENT_PORT_RECOILAPP        "60004"     // TCP

#define NETWORK_PING_RETURN_PORT_RECOILAPP  "60007"     // UDP
#define NETWORK_TIME_RETURN_PORT_RECOILAPP  "60008"     // UDP

#define NETWORK_DISCOVERY_PORT_TESTAPP      "61000"     // TCP
#define NETWORK_SYNC_PORT_TESTAPP           "61001"     // TCP
#define NETWORK_PING_PORT_TESTAPP           "61002"     // UDP
#define NETWORK_TIME_PORT_TESTAPP           "61003"     // UDP
#define NETWORK_EVENT_PORT_TESTAPP          "61004"     // TCP

#define NETWORK_PING_RETURN_PORT_TESTAPP    "61007"     // UDP
#define NETWORK_TIME_RETURN_PORT_TESTAPP    "61008"     // UDP

#endif

// Recoil System network specific
#define SOCKET_INVALID_HANDLE               -1
#define SYSTEM_CALL_ERROR                   -1
#define INVALID_RETURN_VALUE                0xFFFFFFFF
#define INVALID_CLIENT_ID                   0xFF
#define MAX_NETWORK_CLIENTS                 16
#define RETRY_FOR_CONNECTION_REQUEST        250     // in milliseconds
#define RETRY_TO_ACCEPT_ON_MAX_CONNECTIONS  3       // in seconds - when max clients connected sleep until this time before checking for new accept

#define DEBUG_CONFIG_FILE                   "/tmp/recoil_debug.txt"

#ifdef ENABLE_BIG_ENDIAN_BUILD
#define WIRELESS_INTERFACE_NAME             "wlan0"
#else
#define WIRELESS_INTERFACE_NAME             "enp0s3"
#endif

#define NO_OF_BYTES_IP_ADDRESS              16  // "xxx.xxx.xxx.xxx"

// System threads
#define PING_HELPER_THREAD_STACK            8096
#define DEFAULT_THREAD_STACK_SIZE           8096    // 8K stack size


/*
* ------------------------------------ Debug macros ---------------------------------------------------------------
*/

// Enable here for project level debug message
#define __DEBUG_INFO
//#define __DEBUG_EXT_INFO    // extended info for noisy debugs
//#define __DEBUG_IN
//#define __DEBUG_OUT

#ifdef ENABLE_SYSLOG_LOGGING

#define DEBUG_MIL(format, ...)      syslog (LOG_NOTICE, "M:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#define DEBUG_WARN(format, ...)     syslog (LOG_ALERT,  "W:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#define DEBUG_ERROR(format, ...)    syslog (LOG_ERR,    "E:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#define DEBUG_FATAL(format, ...)    syslog (LOG_CRIT,   "F:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);

#ifdef __DEBUG_INFO
#define DEBUG_INFO(format, ...)     syslog (LOG_INFO,   "I:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define DEBUG_INFO(...)
#endif
#ifdef __DEBUG_EXT_INFO
#define DEBUG_EINFO(format, ...)    syslog (LOG_DEBUG,  "X:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define DEBUG_EINFO(...)
#endif
#ifdef __DEBUG
#define DEBUG(format, ...)          syslog (LOG_INFO,   "D:%s:%d> " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define DEBUG(...)
#endif
#ifdef __DEBUG_IN
#define DEBUG_IN                    syslog (LOG_DEBUG,  "x:%s:%d> IN\n", __func__, __LINE__);
#else
#define DEBUG_IN
#endif
#ifdef __DEBUG_OUT
#define DEBUG_OUT                   syslog (LOG_DEBUG,  "x:%s:%d> OUT\n", __func__, __LINE__);
#else
#define DEBUG_OUT
#endif

#else   // use printf

#ifdef __DEBUG_IN
#define DEBUG_IN fprintf (stderr, "F:%s:%d> IN\n", __func__, __LINE__)
#else
#define DEBUG_IN
#endif
#ifdef __DEBUG_OUT
#define DEBUG_OUT fprintf (stderr, "F:%s:%d> OUT\n", __func__, __LINE__)
#else
#define DEBUG_OUT
#endif
#ifdef __DEBUG_EXT_INFO
#define DEBUG_EINFO(...)  fprintf (stderr, "X:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#else
#define DEBUG_EINFO(...)
#endif
#ifdef __DEBUG_INFO
#define DEBUG_INFO(...)  fprintf (stderr, "I:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#else
#define DEBUG_INFO(...)
#endif
#ifdef __DEBUG
#define DEBUG(...)        fprintf (stderr, "D:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#else
#define DEBUG(...)
#endif
#define DEBUG_MIL(...)   fprintf (stderr, "M:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#define DEBUG_WARN(...)  fprintf (stderr, "W:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#define DEBUG_ERROR(...) fprintf (stderr, "E:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#define DEBUG_FATAL(...) fprintf (stderr, "F:%s:%d> ", __func__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n")
#endif

/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

typedef enum
{
    NetworkDiscovery = 0,
    NetworkTime      = 1,
    NetworkPing      = 2,
    BS_Password      = 3,
    BS_SoftwareUpdate= 4,
    BS_Error         = 5,    // free
    Gen_Reserved06   = 6,    // free
    Gen_Reserved07   = 7,    // free
    Gen_Reserved08   = 8,    // free
    Gen_Reserved09   = 9,    // free
    Gen_Reserved10   = 10,   // free
    ServerDataSync   = 11,
    ServerAudio      = 12,
    ServerGameSetup  = 13,
    ServerBroadcast  = 14,
    ServerTeamcast   = 15,
    ServerGameMap    = 16,
    ServerReserved17 = 17,   // free
    ServerReserved18 = 18,   // free
    ServerReserved19 = 19,   // free
    ServerReserved20 = 20,   // free
    ClientDataSync   = 21,
    ClientAudio      = 22,
    ClientReserved23 = 23,   // free
    ClientReserved24 = 24,   // free
    ClientReserved25 = 25,   // free
    GameData         = 26    // can originate from server or client
} ProtocolId;

typedef enum
{
    SYNC_INTERNAL_ERROR,
    SYNC_ALREADY_CONNECTED,
    DISCOVERY_NOT_CONNECTED,
    DISCOVERY_ALREADY_CONNECTED,
    PING_NO_FREE_CONNECTIONS
} NetworkErrors;

typedef enum
{
    SYSTEM_NO_ERROR,
    SYSTEM_PARAM_ERROR,
    SYSTEM_INTERNAL_ERROR,    
    CLIENT_DISCONNECTED_ERROR,
} Error_t;

enum
{
	NW_DATA_TYPE_EVENT,
	NW_DATA_TYPE_VOICE,
	NUM_DATA_TYPES
};
// Protocol header structure - this must match the structure in the client side (recoilnetwork.cs)
// <pid><version><id><length><structure>
#ifdef ENABLE_BIG_ENDIAN_BUILD

typedef struct // NBOL
{
    uint32_t length:12;     // length of the structure to follow the header
    uint32_t id:6;          // protocol id
    uint32_t version:6;     // protocol version
    uint32_t pid:8;         // product id
} ProtocolHeaderV1_write;   // this only defines the fields before structure

typedef struct // NBOL
{
    uint32_t length:12;     // length of the structure to follow the header
    uint32_t id:6;          // protocol id
    uint32_t version:6;     // protocol version
    uint32_t pid:8;         // product id
} ProtocolHeaderV1_read;    // this only defines the fields before structure

#else

typedef struct // NBOL
{
    uint32_t pid:8;         // product id
    uint32_t version:6;     // protocol version
    uint32_t id:6;          // protocol id
    uint32_t length:12;     // length of the structure to follow the header
} ProtocolHeaderV1_write;   // this only defines the fields before structure

typedef struct // NBOL
{
    uint32_t pid:8;         // product id
    uint32_t version:6;     // protocol version
    uint32_t id:6;          // protocol id
    uint32_t length:12;     // length of the structure to follow the header
} ProtocolHeaderV1_read;    // this only defines the fields before structure

#endif // ENABLE_BIG_ENDIAN_BUILD

#define SIZE_OF_HEADER                  4       // bytes
#define ProtocolHeaderV1_PID_MASK       0xFF
#define ProtocolHeaderV1_VERSION_MASK   0x3F
#define ProtocolHeaderV1_ID_MASK        0x3F
#define ProtocolHeaderV1_LENGTH_MASK    0xFFF

typedef struct
{
    ProtocolHeaderV1_write  header;         // protocol packet header
    uint8_t             	id;             // client id
    uint8_t                 error;          // server error
    uint16_t                reserved;       // reserved for future use
} ErrorPacket_t;
#define ERROR_PACKET_SIZE_NO_HEADER     2   // id and error field length

bool RECOIL_SendError (int handle, uint8_t id, NetworkErrors err);

// ------------------------------------------- NETWORK SYNC FEATURE --------------------------------------------------

// Client Sync data structure - this must match the structure in the client side (recoilnetwork.cs)
typedef struct
{
    // client sync data
    uint32_t clientid;
    uint32_t syncData0;
    uint32_t syncData1;
    uint32_t syncData2;
    uint32_t syncData3;
    uint32_t syncData4;
    uint32_t syncData5;
    uint32_t syncData6;
    uint32_t syncData7;
} syncClientData_t;

typedef struct
{
    ProtocolHeaderV1_read   header;    
    syncClientData_t        body;
} SyncClientPacket_t;

typedef struct
{
    uint32_t                NoOfClients;  // no of clients data in the body structure
} SyncServerHeader_t;

typedef struct
{
    ProtocolHeaderV1_write  header;         // protocol header
    SyncServerHeader_t      syncHeader;     // packet header
    syncClientData_t        syncData[MAX_NETWORK_CLIENTS];     // clients data array; size = count * sizeof(syncClientData_t)
} SyncServerPacket_t;

bool NWSYNC_Start (void);
bool NWSYNC_Stop (void);
int  NWSYNC_GetClientCount (void);

// ------------------------------------------- NETWORK DISCOVERY FEATURE --------------------------------------------------

typedef enum
{
    NWDISCOVERY_JOIN = 1,
    NWDISCOVERY_WELCOME = 2,
    NWDISCOVERY_REJOIN = 3,
    NWDISCOVERY_NWCLIENTS_REQUEST = 4,
    NWDISCOVERY_NWCLIENTS = 5,
    NWDISCOVERY_NWCLIENT_ADD = 6,
    NWDISCOVERY_NWCLIENT_REMOVE = 7,
    NWDISCOVERY_GOODBYE = 8,
} DiscoveryPacketType;

typedef struct
{
    uint8_t                 IP[16];             // xxx.xxx.xxx.xxx with NULL termination
    uint8_t                 mac[6];             // mac address in byte array
    bool                    connected;          // connection status
    uint32_t                clientId;           // unique network client id
} DiscoveryClientInfo_t;

typedef struct
{
    ProtocolHeaderV1_read   header;
    uint8_t                 type;                       // discovery packet type - NWDISCOVERY_JOIN/NWDISCOVERY_REJOIN
    uint8_t                 clientId;                   // unique network client id
    uint16_t                reserverd16;
} ClientDiscoveryInfo_t;                                // update SIZE_OF_NETWORK_DISCOVERY_PACKET accordingly

typedef struct
{
    ProtocolHeaderV1_write  header;
    uint8_t                 type;                       // discovery packet type - DISCOVERY_NWCLIENTS
    uint8_t                 Ids[MAX_NETWORK_CLIENTS];   // current active clients in the network
} ServerNWClientsInfo_t;

typedef struct
{
    ProtocolHeaderV1_write  header;
    uint8_t                 type;                       // discovery packet type - NWDISCOVERY_NWCLIENT_ADD / NWDISCOVERY_NWCLIENT_REMOVE
    uint8_t                 ClientId;
} ServerNWClientAddRemove_t;

#define SIZE_OF_NETWORK_DISCOVERY_PACKET    6           // size of the JOIN/REJOIN request packet from the client
#define SIZE_OF_NWDISCOVERY_PACKET_TYPE     1           // byte
#define SIZE_OF_NWDISCOVERY_CLIENT_ID       1           // byte
#define NWDISCOVERY_INVALID_CLIENT_ID       0xFF

// When a GAME CLIENT connects to the GAME SERVER.
typedef void (*RECOILAPP_OnStartClient_fn)(uint32_t newClientId, uint32_t oldClientId);

// When a GAME CLIENT disconnects from the GAME SERVER.
typedef void (*RECOILAPP_OnClientDisconnect_fn)(uint32_t oldClientId);

bool NWDISCOVERY_SetOnStartClient(RECOILAPP_OnStartClient_fn func);
bool NWDISCOVERY_SetOnClientDisconnect(RECOILAPP_OnClientDisconnect_fn func);
bool NWDISCOVERY_Start (void);
bool NWDISCOVERY_Stop (void);
bool NWDISCOVERY_GetClientInfoFromClientId (uint32_t id, DiscoveryClientInfo_t *info); // pass client id and get client info
bool NWDISCOVERY_GetClientInfoFromIP (uint8_t *IP, DiscoveryClientInfo_t *info); // pass client ip and get client info
bool NWDISCOVERY_ClientDisconnectCb (uint32_t id);

// ------------------------------------------- NETWORK PING FEATURE --------------------------------------------------

typedef enum
{
    NWPING_REQUEST = 1,
    NWPING_REPLY = 2
} PingPacketType;

typedef enum
{
    PING_REQUEST_TIME_INVALID,
    PING_REQUEST_TIME_VALID
} PingRequestTimeValid;

typedef struct
{
    int                     handle;         // copy of socket handle - owned by discovery module
    bool                    connected;      // is the client connected ?
    uint32_t                latency;        // latency info
} ClientPingStats_t;

typedef struct
{
    ProtocolHeaderV1_read   header;         // protocol packet header
    uint8_t                 type;           // PingPacketType coded in 8 bits ECHO_REQUEST
    uint8_t                 sequence;       // sequence no of the ping packet
    uint16_t                t_secs;         // client network time - seconds resolution (if '0' ignored)
    uint32_t                t_nsecs;        // client network time - nano seconds resolution (if '0' ignored)
} PingClientPacket_t;

typedef struct
{
    ProtocolHeaderV1_write  header;         // protocol packet header
    uint8_t                 type;           // ping packet type - ECHO_REPLY
    uint8_t                 sequence;       // sequence no of the ping packet
    uint16_t                t_secs;         // server network time - seconds resolution
    uint32_t                t_nsecs;        // server network time - nano seconds resolution
} PingServerPacket_t;

#define ProtocolHeaderV1_PING_NWTIME_SECS_MASK      0xFFFF
#define ProtocolHeaderV1_PING_NWTIME_NSECS_MASK     0xFFFFFFFF
#define ProtocolHeaderV1_PING_NWTIME_FREQUENCY      1000    // in milli seconds

bool NWPING_Start (void);
bool NWPING_Stop (void);

// ------------------------------------------- NETWORK TIME FEATURE --------------------------------------------------

typedef enum
{
    NWPING_HEALTH_BAD,
    NWPING_HEALTH_GOOD,
    NWPING_HEALTH_EXCELLENT
} PingHealth;

typedef enum
{
    NWTIME_DISCONNECT = 1,      // network client is disconnected from nwtime server system
    NWTIME_TIME_REQUEST = 2     // network client has requested for network time
} NWTIME_CallbackType;

typedef enum
{
    NWTIME_REQUEST = 1,
    NWTIME_REPLY = 2
} TimePacketType;

typedef struct
{
    ProtocolHeaderV1_read   header;         // protocol packet header
    uint8_t                 id;             // clientId
    uint8_t                 type;           // time packet type - TimePacketType
    uint8_t                 sequence;       // time request sequence - for udp ordering
    uint8_t                 reserved;       // wont be there in the packet - just for alignment
} TimeClientPacket_t;

#define MAX_SIZE_OF_TIME_REQUEST        ((uint32_t)(sizeof(TimeClientPacket_t) - 1))

typedef struct
{
    ProtocolHeaderV1_write  header;         // protocol packet header
    uint8_t                 sequence;       // time reply sequence - MUST match request sequence no
    uint8_t                 type;           // time packet type - TimePacketType
    uint16_t                t_secs;         // server network time - seconds resolution
    uint32_t                t_nsecs;        // server network time - nano seconds resolution
} TimeServerPacket_t;

#define MAX_SIZE_OF_TIME_REPLY          ((uint32_t)sizeof(TimeClientPacket_t))

typedef bool (*NWTIME_Callback_fn)(uint32_t);

bool NWTIME_Start();
bool NWTIME_Stop (void);
bool NWTIME_AddClient (uint32_t clientId, int hSock, NWTIME_Callback_fn cb);
bool NWTIME_GetClientStatus (uint32_t clientId, ClientPingStats_t stat);
bool NWTIME_RegisterCallback(NWTIME_CallbackType type, NWTIME_Callback_fn cb);

// ------------------------------------------- UTILS FUNCTIONS --------------------------------------------------
typedef void* (*UTILS_ThreadFunction)(void *pArg);

// Generic thread structure
typedef struct
{
    volatile bool           run;            // kill flag to signal thread to exit
    pthread_t               thread;         // thread id returned on the creation
} UTILS_Thread_t;

// Header specific
void UTILS_GetHeaderDefaults (ProtocolHeaderV1_write *hdr);
void UTILS_SetHeader (ProtocolHeaderV1_write *hdr, uint8_t id, uint16_t len);
bool UTILS_VerifyPacketHeader (uint8_t* buffer, uint32_t size, ProtocolId exp_id, uint32_t exp_size, uint32_t *payload_size);

// Thread specific
int32_t UTILS_CreateThread (pthread_t *pThread, uint32_t threadsize, UTILS_ThreadFunction func, void* pData);

// TCP specific
int32_t UTILS_TCP_Create (char* port, bool shared, uint32_t maxConns, int32_t *hSocket);
bool UTILS_TCP_SendData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t *sent, bool *connected);
bool UTILS_TCP_ReadData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t expected_size, uint32_t *readsize, bool *connected);
bool UTILS_TCP_SnoopData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t expected_size, uint32_t *readsize, bool *connected);
bool UTILS_TCP_ReadNBytes (int32_t handle, uint8_t* buffer, uint32_t size, bool *connected);
bool UTILS_TCP_Destroy (int32_t hSocket);

// ------------------------------------------- EVENT FUNCTIONS --------------------------------------------------
#define MAX_SIZE_OF_EVENT_PACKET            1024    // 4 bytes of network header and 1020 bytes of payload

typedef bool (*NWEVENT_Receive_fn)(uint32_t clientId, uint8_t *buffer, uint32_t buffer_size); // one event at a time

bool    NWEVENT_Start (void);
Error_t NWEVENT_Attach (uint32_t clientId, NWEVENT_Receive_fn func);

#ifdef __cplusplus
extern "C"
{
#endif

Error_t NWEVENT_Send (uint32_t clientId, uint8_t *buffer, uint32_t buffer_size, uint32_t *sent);

#ifdef __cplusplus
};
#endif

Error_t NWEVENT_Detach (uint32_t clientId);
bool    NWEVENT_IsConnected (uint32_t clientId);
bool    NWEVENT_Stop (void);

// ------------------------------------------- EVENT FUNCTIONS --------------------------------------------------
bool RECOILAPP_Start (void);
bool RECOILAPP_Stop (void);

#endif // RECOIL_NETWORK_H
