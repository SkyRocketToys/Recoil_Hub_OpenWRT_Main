
#ifndef RECOIL_NETWORK_H
#define RECOIL_NETWORK_H

#include <syslog.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

// ------------ HIGH LEVEL FLAGS TO ENABLE

//#define ENABLE_TEST_APP                     // enable test app to be embedded in the product
#define ENABLE_RECOIL_APP                     // enable recoil app to be embedded in the product
// (From makefile) #define ENABLE_PRODUCTION                   // enable this to use port range 500XX for product
// (From makefile) #define ENABLE_BIG_ENDIAN_BUILD             // enable this to make the product run in the AR9331 base station hardware
//#define ENABLE_SYSLOG_LOGGING               // enable this to route logging to syslogd otherwise it will be printf
//#define USE_EXTRA_PORT_FOR_UDP_RECEIVE      // enable this if you want to use seperate handle to send (will be removed soon)
//#define DISABLE_PAYLOAD
//#define ENABLE_TEST_RESET_PASSWORD_AND_REBOOT // enable the testing of the functionality of the GAME SERVER that
                                              // resets the PASSWORD of the WIRELESS ROUTER or BASE STATION
#define DEBUG_BUILD                           // enable this to see DEBUGGING information displayed
//#define ENABLE_SAME_IP                        // enable multiple CLIENTS to connect from the same IP ADDRESS
                                              // (useful for DEBUGGING the GAME SERVER on UBUNTU 16.04 on a PC,
                                              // connected to multiple GAME CLIENTS on MICROSOFT WINDOWS 10 on the same PC)

// this app gets launched from the bootup script, sleep few seconds to
// allow the network to settle down before we start using them
#define APP_WAIT_UNTIL_WIFI_IS_UP           60 // seconds 

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
extern char *gNetworkUpgradePort;                         // TCP

extern char *gNetworkPingReturnPort;                     // UDP
extern char *gNetworkTimeReturnPort;                     // UDP

#ifdef ENABLE_PRODUCTION

#define NETWORK_DISCOVERY_PORT_RECOILAPP    "50000"     // TCP
#define NETWORK_SYNC_PORT_RECOILAPP         "50001"     // TCP
#define NETWORK_PING_PORT_RECOILAPP         "50002"     // UDP
#define NETWORK_TIME_PORT_RECOILAPP         "50003"     // UDP
#define NETWORK_EVENT_PORT_RECOILAPP        "50004"     // TCP
#define NETWORK_UPGRADE_PORT_RECOILAPP      "50005"     // TCP

#define NETWORK_PING_RETURN_PORT_RECOILAPP  "50007"     // UDP
#define NETWORK_TIME_RETURN_PORT_RECOILAPP  "50008"     // UDP

#define NETWORK_DISCOVERY_PORT_TESTAPP      "51000"     // TCP
#define NETWORK_SYNC_PORT_TESTAPP           "51001"     // TCP
#define NETWORK_PING_PORT_TESTAPP           "51002"     // UDP
#define NETWORK_TIME_PORT_TESTAPP           "51003"     // UDP
#define NETWORK_EVENT_PORT_TESTAPP          "51004"     // TCP
#define NETWORK_UPGRADE_PORT_TESTAPP        "51005"     // TCP

#define NETWORK_PING_RETURN_PORT_TESTAPP    "51007"     // UDP
#define NETWORK_TIME_RETURN_PORT_TESTAPP    "51008"     // UDP

#else

#define NETWORK_DISCOVERY_PORT_RECOILAPP    "60000"     // TCP
#define NETWORK_SYNC_PORT_RECOILAPP         "60001"     // TCP
#define NETWORK_PING_PORT_RECOILAPP         "60002"     // UDP
#define NETWORK_TIME_PORT_RECOILAPP         "60003"     // UDP
#define NETWORK_EVENT_PORT_RECOILAPP        "60004"     // TCP
#define NETWORK_UPGRADE_PORT_RECOILAPP      "60005"     // TCP
#define NETWORK_PING_RETURN_PORT_RECOILAPP  "60007"     // UDP
#define NETWORK_TIME_RETURN_PORT_RECOILAPP  "60008"     // UDP

#define NETWORK_DISCOVERY_PORT_TESTAPP      "61000"     // TCP
#define NETWORK_SYNC_PORT_TESTAPP           "61001"     // TCP
#define NETWORK_PING_PORT_TESTAPP           "61002"     // UDP
#define NETWORK_TIME_PORT_TESTAPP           "61003"     // UDP
#define NETWORK_EVENT_PORT_TESTAPP          "61004"     // TCP
#define NETWORK_UPGRADE_PORT_TESTAPP        "61005"     // TCP
#define NETWORK_PING_RETURN_PORT_TESTAPP    "61007"     // UDP
#define NETWORK_TIME_RETURN_PORT_TESTAPP    "61008"     // UDP

#endif

// Recoil System network specific
#define SOCKET_INVALID_HANDLE               -1
#define SYSTEM_CALL_ERROR                   -1
#define INVALID_RETURN_VALUE                0xFFFFFFFF
#define INVALID_CLIENT_ID                   0xFF
#define MAX_NETWORK_CLIENTS                 16
#define MIN_CLIENT_ID                       1                                     // Minimum ID available for a new CLIENT
#define MAX_CLIENT_ID                       (MIN_CLIENT_ID+MAX_NETWORK_CLIENTS-1) // Maximum ID available for a new CLIENT
#define MAX_NETWORK_CLIENT_IDS              (MIN_CLIENT_ID+MAX_NETWORK_CLIENTS)   // Number of slots required to hold all the IDs
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
    EVENT_ALREADY_CONNECTED,
    DISCOVERY_NOT_CONNECTED,
    DISCOVERY_ALREADY_CONNECTED,
    UPGRADE_VERSION_ERROR,
    UPGRADE_ALREADY_CONNECTED,
    UPGRADE_PAYLOAD_ERROR,
    PING_NO_FREE_CONNECTIONS,
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

// The different criteria used to search for unused CLIENT IDs or HANDLES
// using the IP ADDRESS of a CLIENT
typedef enum
{
	CRITERIA_NONE,
	CRITERIA_LAST_MATCH,
	CRITERIA_FIRST_INVALID_HANDLE,
	NUM_CRITERIA
} ClientCriteriaMatch_t;

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
    syncClientData_t        syncData[MAX_NETWORK_CLIENT_IDS];     // clients data array; size = count * sizeof(syncClientData_t)
} SyncServerPacket_t;

bool NWSYNC_Start (void);
bool NWSYNC_Stop (void);
int  NWSYNC_GetClientCount (void);

// ------------------------------------------- NETWORK DISCOVERY FEATURE --------------------------------------------------

typedef enum
{
	NWDISCOVERY_INVALID = 0,
    NWDISCOVERY_JOIN = 1,
    NWDISCOVERY_WELCOME = 2,
    NWDISCOVERY_REJOIN = 3,
    NWDISCOVERY_NWCLIENTS_REQUEST = 4,
    NWDISCOVERY_NWCLIENTS = 5,
    NWDISCOVERY_NWCLIENT_ADD = 6,
    NWDISCOVERY_NWCLIENT_REMOVE = 7,
    NWDISCOVERY_GOODBYE = 8,
    NWDISCOVERY_NWSERVER_VERSION = 9,
    NWDISCOVERY_NWSERVER_VERSION_INFO = 10,
    NWDISCOVERY_NWSERVER_UPGRADE = 11,
    NWDISCOVERY_NWSERVER_UPGRADE_ACK = 12,
    NWDISCOVERY_NWSERVER_UPGRADE_FAILED = 13,
    NWDISCOVERY_NWSERVER_UPGRADE_PAYLOAD = 14,
    NWDISCOVERY_NWSERVER_UPGRADE_PAYLOAD_ACK = 15,
    NWDISCOVERY_NWSERVER_UPGRADE_UPLOAD_COMPLETE = 16,
    NWDISCOVERY_NWSERVER_UPGRADE_COMPLETE = 17,
    NWDISCOVERY_NWSERVER_UPGRADE_SHUTDOWN = 18,
    NWDISCOVERY_LAST
} DiscoveryPacketType;

#define NWUPGRADE_MIN_PACKET_TYPE   NWDISCOVERY_NWSERVER_VERSION
#define NWUPGRADE_MAX_PACKET_TYPE   NWDISCOVERY_NWSERVER_UPGRADE_COMPLETE

/// <summary>
/// A STRUCT or CLASS for storing data about GAME CLIENTS from which other
/// CLASSES are derived.
///
/// The derived CLASSES all store the HANDLE or 'file descriptor' of a
/// CLIENT that connects to a PORT of the GAME SERVER. The SERVER uses
/// the HANDLE to communicate with that CLIENT, unless that handle is
/// invalid (SOCKET_INVALID_HANDLE).
/// </summary>
typedef struct
{
	int                 handle;             // socket handle
} ClientInfoHandle_t;

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
    uint8_t                 Ids[MAX_NETWORK_CLIENT_IDS];   // current active clients in the network
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
#ifdef ENABLE_SAME_IP
bool NWDISCOVERY_GetClientInfoFromIP(uint8_t *ip, DiscoveryClientInfo_t *info,
	ClientCriteriaMatch_t additionalCriteria,
	char *clientHandles,
	size_t clientInfoHandleSize);
#else
bool NWDISCOVERY_GetClientInfoFromIP(uint8_t *IP, DiscoveryClientInfo_t *info); // pass client ip and get client info
#endif
bool NWDISCOVERY_ClientDisconnectCb (uint32_t id);
bool NWDISCOVERY_ResetClients (void);

#ifdef ENABLE_BIG_ENDIAN_BUILD
#define NWUPGRADE_VERSION_INFO_FILE         "/usr/lib/opkg/info/recoil.control"
#define NWUPGRADE_PACKAGE_BACKUP_FILE       "/upgrade/recoil.ipk"
#else
#define NWUPGRADE_VERSION_INFO_FILE         "recoil.control"
#define NWUPGRADE_PACKAGE_BACKUP_FILE       "recoil.ipk"
#endif
#define NWUPGRADE_PARAM_OFFSET              5   // byte after packet_type
#define NWUPGRADE_MAX_TAG_SIZE              128
#define NWUPGRADE_MD5SUM_SIZE               32
#define SIZE_OF_NETWORK_UPGRADE_RESPONSE    6           // size of the failure/response packet to the client

#define OTA_TYPE_FIRMWARE   "firmware"
#define OTA_TYPE_PACKAGE    "package"

typedef enum
{
    FAILED_NONE                 = 0,
    FAILED_INPROGRESS           = 1,
    FAILED_INVALID_PARAMS       = 2,
    FAILED_NO_UPGRADE           = 3,
    FAILED_SYSTEM_ERROR         = 4,
    FAILED_TRANSFER_ERROR       = 5,
    FAILED_MD5SUM_ERROR         = 6,
    FAILED_INSTALL_ERROR        = 7,
    FAILED_UNCOMPRESSION_ERROR  = 8
} UpgradeFailureReason;

typedef enum
{
    type_firmware,
    type_package,
    type_last
}ota_type;

typedef enum
{
    algo_none,
    algo_lzma,
    algo_last
}algorithm;

typedef enum
{
    ota_compressed,
    ota_uncompressed,
    ota_last
}compressed;

typedef enum
{
    crc_disabled,
    crc_enabled,
    crc_last
}ota_crc;

typedef struct
{
    ProtocolHeaderV1_write  header;
    uint8_t                 type;   // response discovery packet type - NWDISCOVERY_NWSERVER_UPGRADE_ACK / NWDISCOVERY_NWSERVER_UPGRADE_REJECTED
    uint8_t                 reason; // UpgradeFailureReason as uint8_t
} ServerUpgradeResponse_t;
#define SERVER_UPGRADE_RESPONSE_SIZE    6

typedef struct  // includes everything after the protocol header
{
    uint8_t                 type;       // discovery packet type
    uint8_t                 crc_flag;   // ota_crc as uint8_t
    uint16_t                num;        // current block number
    uint32_t                crc32;      // crc32 value
    uint32_t                size;       // payload size in the block
} UpgradePayloadHeader_t;

typedef struct
{
    char        filename[NWUPGRADE_MAX_TAG_SIZE];
    uint32_t    filesize;
    char        path[NWUPGRADE_MAX_TAG_SIZE];
    char        md5sum[NWUPGRADE_MD5SUM_SIZE];
    algorithm   algo;   
    FILE        *fhndl;
} file_info_t;

typedef struct
{
    uint8_t     client;
    uint8_t     type;       // ota_type stored as uint8_t
    uint16_t    next_block; // next block expected
    bool        reconnected;// flag to indicate whether upgrade link was dropped and reconnected
    bool        upgrade_complete_signal; // flag marking upgrade of payloads complete from client
    uint32_t    bcount;     // no of blocks to expect from the network client
    uint32_t    bsize;      // max block size, useful to allocate the buffer
    uint32_t    received;   // received bytes count from network client    
    uint32_t    partial;    // if a block has arrived partial, this contains the rest of the bytes to arrive
    volatile int32_t hServer;    // accept socket connection line
    volatile int32_t hClient;    // dowload socket for upgrade
    file_info_t file;       // details of the compressed / uncompressed file
    compressed  compress;   // ota is compressed or uncompressed
    file_info_t ufile;      // details of the uncompressed file - valid if 'compressed=true'
} OTA_Info_t;

bool NWUPGRADE_Start (void);
void NWUPGRADE_HandleNodePacket (int hSocket, uint32_t clientId, uint8_t* buffer, uint32_t size, bool *pConnected);
bool NWUPGRADE_Stop (void);

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

bool NWTIME_Start(void);
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
void RECOILNetwork_Start(void);
void RECOILNetwork_Stop(void);
void RECOILAPP_ResetBaseStationPassword(void);

#ifdef __cplusplus
};
#endif

Error_t NWEVENT_Detach (uint32_t clientId);
bool    NWEVENT_IsConnected (uint32_t clientId);
bool    NWEVENT_Stop (void);

// ------------------------------------------- EVENT FUNCTIONS --------------------------------------------------
bool TESTAPP_Start (void);
bool TESTAPP_Stop (void);

void RECOILAPP_Setup (void);
bool RECOILAPP_Start (void);
bool RECOILAPP_Stop (void);
void RECOIL_WantPowerOff(void);

// ------------------------------------------- RECOIL TOPLEVEL FUNCTIONS --------------------------------------------------
typedef struct
{
    ProtocolHeaderV1_write  header;         // protocol packet header
    uint8_t             	id;             // client id
    uint8_t                 error;          // server error
    uint16_t                reserved;       // reserved for future use
} ErrorPacket_t;

#define ERROR_PACKET_SIZE_NO_HEADER     2   // id and error field length

bool RECOIL_SendError (bool tcp, int handle, uint8_t id, NetworkErrors err, struct sockaddr *p_c_addr, int c_size);
void RECOIL_TermSignalHandler(int stat);

#endif // RECOIL_NETWORK_H
