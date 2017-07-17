//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- discovery.c
//  description :- Recoil Network Discovery service
//  author      :- Rajesh Gunasekaran   (rajg@hotgen.com)
//

/*
* ------------------------------------ Header files  -----------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <pwd.h>
#include <netinet/in.h>

//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"
//#include "UnityGameNetworkServer/GameNetworkServerWrapper.h"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/
#define TCP_CLIENT_THREAD_STACK                 8096        // 8K stack for each client thread
#define LENGTH_USED_FOR_IP_VALIDATION           12          // 11.11.11.XXX  <= 'x' is compared and 'X' is dynamic value
#define WELCOME_MESSAGE_TEMPLATE                "Welcome to RECOIL v%d.%d-%d ID%03d"  //  Template is <header><type>"Welcome to RECOIL v1.0-0 IDnnn"  nnn is 3 digit player id
#define WELCOME_MESSAGE_LENGTH                  35          // 4 bytes header + packet_type + 31 bytes string
#define WELCOME_MESSAGE_OFFSET_CLIENT_ID        32
#define GOODBYE_MESSAGE_TEMPLATE                "RECOIL v%d.%d-%d Shutting down..."   //  Template is <header><type>"RECOIL v1.0-0  Shutting down.."
#define GOODBYE_MESSAGE_LENGTH                  35          // 4 bytes header + packet_type + 31 bytes string
#define TIMEOUT_TO_CHECK_INCOMING_TRAFFIC       100000      // uSeconds
#define MAX_INCOMING_BYTES                      (uint32_t)SIZE_OF_NETWORK_DISCOVERY_PACKET
/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

// client global data structure
typedef struct
{
    int                 handle;             // socket handle
    bool                bWelcome;           // is the client been provided with a valid network client id through welcome message
    uint8_t             IP[16];             // xxx.xxx.xxx.xxx with NULL termination
    uint8_t             mac[6];             // mac address in byte array
} ClientInfo_t;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
// allocate and reserve all the memory needed by the system. since hardware is a low memory system, dynamic memory usage is not appropriate.

static ClientInfo_t gClients[MAX_NETWORK_CLIENTS];      // global data structure for max clients supported
static volatile int gNoOfClients = 0;                   // global clients count - must be protected with gNoOfClients_mutex to add or remove a client
static volatile int gNoOfClientsDisconnected = 0;       // global clients count who were given a client id, but disconnected now
static pthread_mutex_t gNoOfClients_mutex;              // protects addition or deletion of a client to the global data structure
static UTILS_Thread_t gHelperThread;                    // thread id for the Helper thread - which is responsible for incoming request for discovery and manages them
static int ghMainSocket;                                // main socket to accept incoming connections for network discovery
static uint8_t gBasestationIP[NO_OF_BYTES_IP_ADDRESS] = {0};// base station ip
static uint8_t gGoodByeMessage[GOODBYE_MESSAGE_LENGTH+1];   // standard bye message for the clients; +1 for null char, but not sent to the network
static uint8_t gWelcomeMessage[WELCOME_MESSAGE_LENGTH+1];   // standard welcome message for the clients; +1 for null char, but not sent to the network
static ServerNWClientsInfo_t gPkt_NWClients;                // NWClients packet structure
volatile bool enable = false;

static RECOILAPP_OnStartClient_fn gOnStartClient = 0;
static RECOILAPP_OnClientDisconnect_fn gOnClientDisconnect = 0;

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/


bool NWDISCOVERY_SetOnStartClient(RECOILAPP_OnStartClient_fn func)
{
    gOnStartClient = func;
}

bool NWDISCOVERY_SetOnClientDisconnect(RECOILAPP_OnClientDisconnect_fn func)
{
    gOnClientDisconnect = func;
}

void nwdiscovery_dumpCurrentClientsInfo (void)
{
    uint32_t node;

    DEBUG_WARN("No of clients connected gNoOfClients(%d)", gNoOfClients);
    DEBUG_WARN("No of clients registered but disconnected gNoOfClientsDisconnected(%d)", gNoOfClientsDisconnected);
    for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
    {
        DEBUG_WARN("Client(%2d) handle[%3d] bWelcome[%d] IP[%12s] HWaddr[%02x:%02x:%02x:%02x:%02x:%02x]", 
            node, gClients[node].handle, gClients[node].bWelcome,
            (gClients[node].IP[0] != 0 && gClients[node].IP[1] != 0) ? (char*)gClients[node].IP : (char*)"",
            gClients[node].mac[0],gClients[node].mac[1],gClients[node].mac[2],
            gClients[node].mac[3],gClients[node].mac[4],gClients[node].mac[5]);
    }

    DEBUG_OUT;
    return;
}

// ensure this function is called with gNoOfClients_mutex taken/locked
// this function send the below packet(s)
// NWCLIENTS -> to newly joined clientid
void nwdiscovery_SendUpdatePacket (uint32_t clientId)
{
    uint16_t node, noOfClients, NoOfActiveClients = 0;

    DEBUG_EINFO("Update list request for client(%d)", clientid);
    for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
    {
        if (SOCKET_INVALID_HANDLE != gClients[node].handle)
        {
            DEBUG_EINFO("Found a valid client(%d) handle(%d)...", node, gClients[node].handle);

            // build the new NWCLIENTS packet in the global buffer
            gPkt_NWClients.Ids[NoOfActiveClients++] = node;
        }
    }

    if (NoOfActiveClients)
    {
        gPkt_NWClients.type = NWDISCOVERY_NWCLIENTS;
        UTILS_SetHeader(&gPkt_NWClients.header, NetworkDiscovery, (SIZE_OF_NWDISCOVERY_PACKET_TYPE + NoOfActiveClients));   // length of the client array plus packet_type

        {
            uint8_t *p = (uint8_t*)&gPkt_NWClients;
            DEBUG_INFO("NWCLIENTS packet [%2x %2x %2x %2x] [%2x] [%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x]",
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20]);
        }

        if (!UTILS_TCP_SendData(gClients[clientId].handle, (uint8_t*)&gPkt_NWClients, (SIZE_OF_HEADER + (SIZE_OF_NWDISCOVERY_PACKET_TYPE + NoOfActiveClients)), NULL, NULL))
        {
            DEBUG_ERROR("Unable to send NWCLIENTS packet to the client(%d)", clientId);
        }
        else
        {
            DEBUG_INFO("Successfully sent NWCLIENTS packet to the client(%d)", clientId);
        }
    }
}

// ensure this function is called with gNoOfClients_mutex taken/locked
// this function send the below packet(s)
// NWCLIENTS -> to newly joined clientid
// NWCLIENT_ADD -> to all active clients in the network
void nwdiscovery_OnClientJoinLeave (bool bJoin, uint32_t clientId)
{
    uint16_t node, noOfClients;
    ServerNWClientAddRemove_t JoinLeavePacket;

    DEBUG_INFO ("NWCLIENT_%s of Client#%d gNoOfClients=%d", (bJoin) ? "ADD" : "REMOVE", clientId, gNoOfClients);

    // fill with invalid client id first
    memset( gPkt_NWClients.Ids, 0xFF, MAX_NETWORK_CLIENTS);

    UTILS_SetHeader(&JoinLeavePacket.header, NetworkDiscovery, (SIZE_OF_NWDISCOVERY_CLIENT_ID + SIZE_OF_NWDISCOVERY_PACKET_TYPE));   // length of the client id plus packet_type    
    JoinLeavePacket.ClientId  = clientId;
    JoinLeavePacket.type = (bJoin) ? NWDISCOVERY_NWCLIENT_ADD : NWDISCOVERY_NWCLIENT_REMOVE;

    for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
    {
        if (SOCKET_INVALID_HANDLE != gClients[node].handle)
        {
            DEBUG_EINFO("Found a valid client(%d) handle(%d)...", node, gClients[node].handle);

            // dont send the NWDISCOVERY_NWCLIENT_ADD/REMOVE packet to the join/rejoin client
            if (node != clientId)
            {
                if (!UTILS_TCP_SendData(gClients[node].handle, (uint8_t*)&JoinLeavePacket, (SIZE_OF_HEADER + (SIZE_OF_NWDISCOVERY_CLIENT_ID + SIZE_OF_NWDISCOVERY_PACKET_TYPE)), NULL, NULL))
                {
                    DEBUG_ERROR("Unable to send NWCLIENT_%s packet to the client(%d)", (bJoin) ? "ADD" : "REMOVE", node);
                }
                else
                {
                    DEBUG_MIL("Successfully sent NWCLIENT_%s packet to the client(%d)", (bJoin) ? "ADD" : "REMOVE", node);
                }
            }
        }
    }

    if (bJoin) // for the join/rejoin client send the client list
    {
        nwdiscovery_SendUpdatePacket(clientId);
        //--------------------------------------------------------------------
        // Connect a new GAME CLIENT on the GAME SERVER.
        //
        // Initialise the player's properties and replay the EVENTS on the SERVER so far.
        //--------------------------------------------------------------------
        if (gOnStartClient)
        {
            gOnStartClient(clientId, -1);
        }
    }
    else
    {
        //--------------------------------------------------------------------
        // Disconnect a GAME CLIENT from the GAME SERVER.
        //
        // Remove the player's properties from the SERVER.
        //--------------------------------------------------------------------
        if (gOnClientDisconnect)
        {
            gOnClientDisconnect(clientId);
        }
    }

    return;
}

// Get a free client index
// SUCCESS - returns a free index from global client array on
// FAILURE - INVALID_RETURN_VALUE
// NOTE :- MUST be called with the gNoOfClients_mutex lock taken
int nwdiscovery_GetFreeClientIndex (void)
{
    uint32_t node = INVALID_RETURN_VALUE;
    DEBUG_IN;

    if (gNoOfClients < MAX_NETWORK_CLIENTS)
    {
        for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
        {
            if (SOCKET_INVALID_HANDLE == gClients[node].handle)
            {
                break;
                DEBUG_INFO ("Found a free client index = %d", node);
            }
        }

        // failed to get a node - unexpected system behaviour
        if (node >= MAX_NETWORK_CLIENTS)
        {
            node = INVALID_RETURN_VALUE;
            DEBUG_FATAL("System internal error - Unable to get a free client node - gNoOfClients(%d)", gNoOfClients);
        }            
    }
    else // failed to get a node - are we full on ?
    {
        DEBUG_ERROR("System reached clients maximum limit");
        nwdiscovery_dumpCurrentClientsInfo();
    }

    DEBUG_OUT;
    return (node);
}

// ensure this function is called with gNoOfClients_mutex taken/locked
void nwdiscovery_ReleaseClientNode(uint32_t node, bool alive)
{
    DEBUG_IN;

    // Send a GoodBye message
    if (alive)
    {
        DEBUG_INFO("Sending goodbye to client(%s, %d) msg(%s)", gClients[node].IP, node, &gGoodByeMessage[SIZE_OF_HEADER+1]);

        // if we are not able to send the bytes - dont retry we are going down anyway
        UTILS_TCP_SendData(gClients[node].handle, gGoodByeMessage, GOODBYE_MESSAGE_LENGTH, NULL, NULL);
    }

    // free the socket
    if (SYSTEM_CALL_ERROR == close(gClients[node].handle)) 
    {
        DEBUG_ERROR("Socket(%d) close failed(%d, %s) for IP(%s)", gClients[node].handle, errno, strerror(errno), gClients[node].IP);
    }

    // reset the handle
    gClients[node].handle = SOCKET_INVALID_HANDLE;

    // reset the welcome flag
    gClients[node].bWelcome = false;

    if (!alive)
    {
        gNoOfClientsDisconnected++;
        nwdiscovery_OnClientJoinLeave (false, node);
        // NOTE :-  gClients[node].IP and gClients[node].mac are still valid, it is used for future reconnection 
        // from the same client to provide same client id for a previously connected client
        DEBUG_INFO("Client(%d, %s) disconnected from the discovery network", node, gClients[node].IP);
    }
    else
    {
        // we only destroy connection if the system is going down, so reset all the internal data about clients
        gNoOfClients--;
        memset (gClients[node].IP, 0, sizeof(gClients[node].IP));
        memset (gClients[node].mac, 0, sizeof(gClients[node].mac));
        DEBUG_INFO("Client(%d, %s) removed successfully from the discovery network", node, gClients[node].IP);
    }

    DEBUG_OUT;
    return;
}

// Terminates the system and free all the resources allocated
// If we fail to free a resource, throw an error and continue with other resources
// NO_RETURN_VALUE
void nwdiscovery_SystemTerminate (void)
{
    uint32_t node, noOfClients, tmp_gNoOfClients;
    DEBUG_IN;

    // we are removing all the clients - take the global clients lock
    pthread_mutex_lock(&gNoOfClients_mutex);
    tmp_gNoOfClients = gNoOfClients;
    for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < tmp_gNoOfClients)); node++)
    {
        uint8_t *p = gClients[node].IP;
        if (SOCKET_INVALID_HANDLE != gClients[node].handle)
        {
            nwdiscovery_ReleaseClientNode(node, true);
            noOfClients++;
        }
        else if (p[0] != 0 && p[1] != 0 && p[2] != 0)    // valid ip = ip with first 3 characters having a valid number (xxx.xxx.xxx.xxx format)
        {
            // clientId is of a disconnected client - a client who was connected during this run but later disconnected from the BS
            gNoOfClientsDisconnected--;
            gNoOfClients--;
            memset (gClients[node].IP, 0, sizeof(gClients[node].IP));
            memset (gClients[node].mac, 0, sizeof(gClients[node].mac));
            noOfClients++;         
        }
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_OUT;
    return;  
}

bool NWDISCOVERY_ClientDisconnectCb (uint32_t id)
{
    DEBUG_IN;
    pthread_mutex_lock(&gNoOfClients_mutex);
    nwdiscovery_ReleaseClientNode(id, false);
    pthread_mutex_unlock(&gNoOfClients_mutex);
    DEBUG_OUT;
    return true;
}

// MUST be called with gNoOfClients_mutex lock taken
bool nwdiscovery_GetClientIdFromIP (uint8_t *ip, uint32_t *id)
{
    bool retval = false;
    uint32_t node;
    
    DEBUG_IN;
    // TODO - optimise this
    //for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
    for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
    {
        uint8_t *p = gClients[node].IP;
        if (p[0] != 0 && p[1] != 0 && p[2] != 0)    // valid ip = ip with first 3 characters having a valid number (xxx.xxx.xxx.xxx format)
        {
            if (0 == strcmp ((const char *)gClients[node].IP, (const char *)ip)) // ip match ??
            {
                *id = node;
                retval = true;
                break;
            }
        }
    }

    DEBUG_OUT;
    return retval;
}

// ip ptr must be valid and got enough space for ip addr string (16 bytes) and the ifr_name must be a null terminated string
bool nwdiscovery_GetInterfaceIP (uint8_t *ip, char *ifname)
{
    int fd, status;
    struct ifreq ifr;
    bool retval = false;

    DEBUG_IN;
    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, ifname);

    if (SYSTEM_CALL_ERROR == (fd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        DEBUG_ERROR("Socket open failed for AF_INET, SOCK_DGRAM, 0");
    }
    else if (SYSTEM_CALL_ERROR == (status = ioctl(fd, SIOCGIFADDR, &ifr)))
    {
        DEBUG_ERROR("ioctl failed to get SIOCGIFADDR err(%d,%s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = close(fd)))
    {
        DEBUG_ERROR("Socket close failed err(%d,%s)", errno, strerror(errno));
    }
    else
    {
        strcpy ((char*)ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        retval = true;
    }

    DEBUG_OUT;
    return retval;
}

void nwdiscovery_SendWelcomeMessage(uint32_t node, int handle)
{
    bool connected;
    uint32_t sent;

    // update the client id to the template string
    sprintf((char*)&gWelcomeMessage[WELCOME_MESSAGE_OFFSET_CLIENT_ID], "%03d", node);

    DEBUG_MIL("Sending welcome message to client(%s, %d) msg(%s, %d)", gClients[node].IP, node, &gWelcomeMessage[SIZE_OF_HEADER+1], (int32_t)strlen((char*)gWelcomeMessage)+1);

    // if we are not able to send the bytes - dont retry, with sync we can update this in the next cycle - socket must be busy if we get sent = 0 and return value is true
    if ((UTILS_TCP_SendData(handle, gWelcomeMessage, WELCOME_MESSAGE_LENGTH, &sent, &connected)) && (sent == WELCOME_MESSAGE_LENGTH))
    {
        DEBUG_EINFO("Welcome message sent client(%d)",node);
        gClients[node].bWelcome = true;
    }
    else if (false == connected) // broken pipe ??
    {
        DEBUG_ERROR("Client(%s, %d) disconnected unexpectedly", gClients[node].IP, node);
        nwdiscovery_ReleaseClientNode(node, false);
    }

    return;
}

// buffer - MUST be valid, no check is made inside the function
void nwdiscovery_HandleNodePacket (int hSocket, uint32_t clientId, uint8_t* buffer, uint32_t size)
{
    ClientDiscoveryInfo_t *pkt_in = (ClientDiscoveryInfo_t*)buffer;

    DEBUG_EINFO("received %4d bytes of data from client(%s, %2d) data[%2x %2x %2x %2x][%2x %2x %2x %2x]", 
        size, gClients[clientId].IP, clientId, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);        
    DEBUG_EINFO ("type(%d) clientid(%d)", pkt_in->type, pkt_in->clientId);

    if (!UTILS_VerifyPacketHeader (buffer, size, NetworkDiscovery, MAX_INCOMING_BYTES, NULL))    // handle only the full packets
    {
        DEBUG_ERROR("Network packet header verification failed");
    }
    else if ((pkt_in->type == (uint8_t)(NWDISCOVERY_JOIN & 0xFF)) || (pkt_in->type == (uint8_t)(NWDISCOVERY_REJOIN & 0xFF)))
    {
        if ((pkt_in->type == (uint8_t)(NWDISCOVERY_JOIN & 0xFF)) && (pkt_in->clientId != (uint8_t)(NWDISCOVERY_INVALID_CLIENT_ID & 0xFF)))
        {
            DEBUG_ERROR("JOIN packet is invalid exp(%x) recv(%x)", (uint8_t)(NWDISCOVERY_INVALID_CLIENT_ID & 0xFF), pkt_in->clientId);
        }    
        else if ((pkt_in->type == (uint8_t)(NWDISCOVERY_REJOIN & 0xFF)) && (pkt_in->clientId != (uint8_t)(clientId & 0xFF)))        // correct node id or invalid id are allowed
        {
            DEBUG_ERROR("REJOIN packet is invalid exp(%x) recv(%x)", clientId, pkt_in->clientId);
        }
        else if ((pkt_in->type == (uint8_t)(NWDISCOVERY_JOIN & 0xFF)) || (pkt_in->type == (uint8_t)(NWDISCOVERY_REJOIN & 0xFF)))    // JOIN/REJOIN request
        {
            nwdiscovery_SendWelcomeMessage(clientId, hSocket);
            nwdiscovery_OnClientJoinLeave(true, clientId);
        }
    }
    else if ((pkt_in->type == (uint8_t)(NWDISCOVERY_NWCLIENTS_REQUEST & 0xFF)) && (pkt_in->clientId == (uint8_t)(clientId & 0xFF))) // UPDATE request
    {
        nwdiscovery_SendUpdatePacket(clientId);
    }    
    else
    {
        DEBUG_ERROR("received %4d bytes of an unknown packet from client(%s, %2d) data[%2x %2x %2x %2x][%2x %2x %2x %2x]", 
            size, gClients[clientId].IP, clientId, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    }

    DEBUG_OUT;
    return;
}

void* nwdiscovery_HelperThread (void *pArg)
{
    int client_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client;    
    uint32_t node, noOfClients;
    int hSocket = *(int*)pArg; // socket to accept incoming connections
    int status;
    uint8_t *buffer = NULL;

    int nfds = 0;       // nfds is the highest-numbered file descriptor in readfds/writefds/exceptfds plus 1. 
    fd_set fds;         // We only use readfds, fds is the handle used to watch for any characters become available for reading in the attached socket handles
    struct timeval tv;
    int activity, RxSize;

    DEBUG_IN;

    DEBUG_EINFO("Listening for incoming connections(max:%d) on port %s", MAX_NETWORK_CLIENTS, gNetworkDiscoveryPort);
    fcntl(hSocket, F_SETFL, O_NONBLOCK); /* configure the socket to work in non-blocking state	*/

    // allocate buffer to hold only one packet at a time - as in the SyncClientPacket_t
    if (NULL == (buffer = (uint8_t*)malloc(MAX_INCOMING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)MAX_INCOMING_BYTES);
        gHelperThread.run = false;
    }

    while (gHelperThread.run) // while the system is allowed to run
    {
        DEBUG_EINFO("No new request, sleep until we recheck again...");

        /* wait for 100mS to check whether we got any traffic in the connected clients */
        tv.tv_sec = 0;
        tv.tv_usec = TIMEOUT_TO_CHECK_INCOMING_TRAFFIC;

        FD_ZERO(&fds);

        pthread_mutex_lock(&gNoOfClients_mutex);

        // configure main socket for activity
        FD_SET (hSocket, &fds);
        nfds = hSocket;
        for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
        {
            if (SOCKET_INVALID_HANDLE != gClients[node].handle)
            {
                DEBUG_EINFO ("Found a valid client(%d) handle(%d)...", node, gClients[node].handle);
                FD_SET (gClients[node].handle, &fds);
                if (gClients[node].handle > nfds)
                {
                    nfds = gClients[node].handle;
                }
            }
        }

        // check whether we got any activity from the network
        if (SYSTEM_CALL_ERROR == (activity = select(nfds+1, &fds, NULL, NULL, &tv)))
        {
            DEBUG_ERROR("select() failed (%d, %s) nfds(%d)", errno, strerror(errno), nfds);
        }
        else if (activity)
        {
            DEBUG_EINFO("Somedata is available on the network");

            // check the main handle - for new connections
            if (FD_ISSET(hSocket, &fds))
            {
                int tmphandle;

                if (SYSTEM_CALL_ERROR != (tmphandle = accept (hSocket, (struct sockaddr *) &client, (socklen_t*) &client_size)))
                {
                    DEBUG_INFO("Client(%s) Incoming request to join the network !!", inet_ntoa(client.sin_addr));

                    // check whether it is an existing client
                    if (nwdiscovery_GetClientIdFromIP ((uint8_t*)inet_ntoa(client.sin_addr), &node))
                    {
                        if (gClients[node].handle == INVALID_RETURN_VALUE) // disconnected client
                        {
                            DEBUG_MIL("Client(%s) reestablished discovery connection !!", inet_ntoa(client.sin_addr));
                            gClients[node].handle = tmphandle;                            
                        }
                        else
                        {
                            // this client already got a discovery link throw an error
                            DEBUG_ERROR("Client(%s) already got active discovery connection !!", inet_ntoa(client.sin_addr));
                            RECOIL_SendError(tmphandle, (uint8_t)node, DISCOVERY_ALREADY_CONNECTED);
                        }
                    }
                    else if (INVALID_RETURN_VALUE != (node = nwdiscovery_GetFreeClientIndex())) // new client
                    {
                        bool valid_mac = true;
                        struct arpreq areq;
                        struct sockaddr_in *sin;

                        // get mac address - using arp
                        memset(&areq, 0, sizeof(areq));
                        sin = (struct sockaddr_in *) &areq.arp_pa;
                        sin->sin_family = AF_INET;
                        sin->sin_addr = client.sin_addr;
                        sin = (struct sockaddr_in *) &areq.arp_ha;
                        sin->sin_family = ARPHRD_ETHER;
                        strncpy(areq.arp_dev, WIRELESS_INTERFACE_NAME, sizeof(WIRELESS_INTERFACE_NAME));
                        if (SYSTEM_CALL_ERROR == (status = ioctl(tmphandle, SIOCGARP, (caddr_t) &areq)))
                        {
                            DEBUG_ERROR("Unable to get mac addr of the connected client IP(%s) err(%d, %s)", inet_ntoa(client.sin_addr), errno, strerror(errno));
                            valid_mac = false;
                        }

                        gClients[node].handle = tmphandle;
                        strcpy ((char *)gClients[node].IP, inet_ntoa(client.sin_addr));
                        if (valid_mac)
                        {  
                            memcpy(gClients[node].mac, ((struct sockaddr *)&areq.arp_ha)->sa_data, sizeof(gClients[node].mac)); 
                        }
                        else
                        { 
                            memset(gClients[node].mac, 0, sizeof(gClients[node].mac)); 
                        }
                        gNoOfClients++; // we got an additional client

                        DEBUG_INFO("Connected successfully to IP(%s) socket(%d) on node(%d)", gClients[node].IP, tmphandle, node);
                    }
                    else // either an internal error or max connections reached - both way we cant do much except retry after after few seconds
                    {
                        DEBUG_WARN("Network discovery running with max connection's supported in the system !!");
                    }
                }
                else if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
                    DEBUG_FATAL("No new connection request did the select fail !!! err(%d, %s)", errno, strerror(errno));
                }
                else
                { 
                    DEBUG_FATAL("Incoming connection accept() failed (%d, %s)", errno, strerror(errno));
                }
            }

            for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
            {
                if ((INVALID_RETURN_VALUE != gClients[node].handle) && (FD_ISSET(gClients[node].handle, &fds)))
                {
                    DEBUG_EINFO("client(%s, %d) got some data", gClients[node].IP, node);

                    // read the incoming data
                    if (SYSTEM_CALL_ERROR == (RxSize = recv(gClients[node].handle, buffer, MAX_INCOMING_BYTES, 0)))
                    {
                        // TODO - check this is valid ?? we got an activity but no data, could this be valid ?
                        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                        {
                            DEBUG_WARN("Nothing to read from the socket..");
                        }
                        else
                        {
                            DEBUG_ERROR("recv() failed(%d, %s) for client(%s, %d)", errno, strerror(errno), gClients[node].IP, node);
                        }
                    }
                    else if (RxSize == 0)
                    {
                        DEBUG_ERROR("Client(%s, %d) disconnected unexpectedly", gClients[node].IP, node);
                        nwdiscovery_ReleaseClientNode(node, false);
                    }                    
                    else
                    {
                        DEBUG_EINFO("Client(%s, %d) has sent a packet...", gClients[node].IP, node);
                        nwdiscovery_HandleNodePacket(gClients[node].handle, node, buffer, (uint32_t)RxSize);
                    }                    
                }
            }
        }
        else
        {
            DEBUG_EINFO ("Timeout on the select...");
        }

        pthread_mutex_unlock(&gNoOfClients_mutex);

        usleep (RETRY_FOR_CONNECTION_REQUEST*1000); // we dont have new request - sleep before we go for a fresh wait               
    }    

    DEBUG_INFO("System received a shut down req gNoOfClients(%d) gNoOfClientsDisconnected(%d)", gNoOfClients, gNoOfClientsDisconnected);

    // Close the incoming socket
    if (SOCKET_INVALID_HANDLE != hSocket)
    {
        if (SYSTEM_CALL_ERROR == close(hSocket))
        {
            DEBUG_ERROR("Socket(%d) close failed(%d, %s)", hSocket, errno, strerror(errno));
        }
    }

    // tear down the system
    nwdiscovery_SystemTerminate();
    closelog ();

    DEBUG_MIL("Network Discovery [helper thread] shutdown complete...");

    DEBUG_OUT;
    pthread_exit(NULL);
}

// entry point for the network discovery sub system
bool NWDISCOVERY_Start (void)
{
    int status;
    uint32_t node;
	struct addrinfo hints;// , *server = NULL;
    int option = 1;
    pthread_attr_t attr;
    bool retval = false;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_DISC", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // helper thread must start once it is created
    gHelperThread.run = true;

    for (node = 0; node < MAX_NETWORK_CLIENTS; node++) {
        // Reset all the socket handles - this is used to validate a node !!
        gClients[node].handle = SOCKET_INVALID_HANDLE;
        memset (gClients[node].IP, 0, sizeof(gClients[node].IP));
        memset (gClients[node].mac, 0, sizeof(gClients[node].mac));
        gClients[node].bWelcome = false;
    }

    // build the template for welcome and bye message for clients
    {
        ProtocolHeaderV1_write *header;
        uint8_t *p;        

        header = (ProtocolHeaderV1_write*)gWelcomeMessage;
        UTILS_SetHeader(header, NetworkDiscovery, (WELCOME_MESSAGE_LENGTH - sizeof(ProtocolHeaderV1_write)));
        gWelcomeMessage[SIZE_OF_HEADER] = (uint8_t)(NWDISCOVERY_WELCOME & 0xFF);
        sprintf ((char*)&gWelcomeMessage[SIZE_OF_HEADER+1], WELCOME_MESSAGE_TEMPLATE, RECOIL_MAJOR_VERSION, RECOIL_MINOR_VERSION, RECOIL_PACKAGE_VERSION, 0);
#if 0
        p = gWelcomeMessage;
        DEBUG_INFO("Welcome[%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x]", 
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],  p[8], p[9], p[10], p[11]);         
#endif            

        header = (ProtocolHeaderV1_write*)gGoodByeMessage;
        UTILS_SetHeader(header, NetworkDiscovery, (GOODBYE_MESSAGE_LENGTH - sizeof(ProtocolHeaderV1_write)));
        gGoodByeMessage[SIZE_OF_HEADER] = (uint8_t)(NWDISCOVERY_GOODBYE & 0xFF);
        sprintf ((char*)&gGoodByeMessage[SIZE_OF_HEADER+1], GOODBYE_MESSAGE_TEMPLATE, RECOIL_MAJOR_VERSION, RECOIL_MINOR_VERSION, RECOIL_PACKAGE_VERSION);
#if 0
        p = gGoodByeMessage;
        DEBUG_INFO("Welcome[%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x]", 
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],  p[8], p[9], p[10], p[11]);         
#endif            
        
    }

    // Create a TCP socket
    if (0 != (status = UTILS_TCP_Create(gNetworkDiscoveryPort, true, MAX_NETWORK_CLIENTS, &ghMainSocket)))
    {
        DEBUG_FATAL("UTILS_TCP_Create failed(%s:%d) for port(%s)", gai_strerror(status), status, gNetworkDiscoveryPort);
    }
    else if (SYSTEM_CALL_ERROR == (status = pthread_mutex_init (&gNoOfClients_mutex, NULL)))
    {
        DEBUG_FATAL("Global client mutex create failed(%d, %s)", errno, strerror(errno));
    }
    // create the helper thread    
    else if (SYSTEM_CALL_ERROR == (status = UTILS_CreateThread (&gHelperThread.thread, DEFAULT_THREAD_STACK_SIZE, nwdiscovery_HelperThread, (void*) &ghMainSocket)))
    {
        DEBUG_FATAL("Helper thread create failed (%d, %s)", errno, strerror(errno));
    }    
    else if (!nwdiscovery_GetInterfaceIP(gBasestationIP, WIRELESS_INTERFACE_NAME))
    {
        DEBUG_FATAL("Unable to get the Basestation IP with interface(%s)", WIRELESS_INTERFACE_NAME);
    }
    else
    {
        DEBUG_MIL("Network Discovery started successfully on IP(%s) port(%s) interface(%s)...", gBasestationIP, gNetworkDiscoveryPort, WIRELESS_INTERFACE_NAME);
        retval = true;
    }

    DEBUG_OUT;

    return (retval);
}

bool NWDISCOVERY_Stop (void)
{
    void *H_retval = NULL;
    int status;
    bool retval = false;    
    
    DEBUG_IN;
    DEBUG_MIL ("Request to shut down [network discovery] arrived...");

    // signal helper thread to terminate
    gHelperThread.run = false;
    if (0 != (status = pthread_join(gHelperThread.thread, &H_retval)))
    {
        DEBUG_ERROR("Unable to wait for helper thread to exit...err(%d, %s)", errno, strerror(errno));
    }
    else
    {
        DEBUG_MIL ("Request to shut down [network discovery] is complete...");        
        retval = true;
    }

    DEBUG_OUT;

    return (retval);
}

bool NWDISCOVERY_GetClientInfoFromClientId (uint32_t id, DiscoveryClientInfo_t *info)
{
    bool retval = false;
    DEBUG_IN;

    if (id < MAX_NETWORK_CLIENTS)
    {
        if (gBasestationIP[0] != 0 && gBasestationIP[1] != 0 && gBasestationIP[2] != 0) // check if we have a valid Basestation ip
        {
            pthread_mutex_lock(&gNoOfClients_mutex);
            info->connected = (SOCKET_INVALID_HANDLE != gClients[id].handle) ? true : false;
            
            if (0 != memcmp (gBasestationIP, gClients[id].IP, LENGTH_USED_FOR_IP_VALIDATION))
            {
#ifdef __DEBUG_EXT_INFO                
                uint8_t *p = gClients[id].IP;
#endif                
                DEBUG_EINFO("gBasestationIP [%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x]", 
                    gBasestationIP[0], gBasestationIP[1], gBasestationIP[2], gBasestationIP[3], 
                    gBasestationIP[4], gBasestationIP[5], gBasestationIP[6], gBasestationIP[7],  
                    gBasestationIP[8], gBasestationIP[9], gBasestationIP[10], gBasestationIP[11]);
                DEBUG_EINFO("NC_ip [%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x]", 
                    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],  p[8], p[9], p[10], p[11]);                
            }
            else
            {
                memcpy (&info->IP, &gClients[id].IP, sizeof(gClients[id].IP));
                memcpy (&info->mac, &gClients[id].mac, sizeof(gClients[id].mac));
                info->clientId = id;
                retval = true;
            }

            pthread_mutex_unlock(&gNoOfClients_mutex);            
        }
    }
    else
    {
        DEBUG_ERROR("Invalid client id (%u) passed as input", id);
    }

    DEBUG_OUT;
    return retval;
}

bool NWDISCOVERY_GetClientInfoFromIP (uint8_t *ip, DiscoveryClientInfo_t *info)
{
    bool retval = false;
    uint32_t node;
    
    DEBUG_IN;

    pthread_mutex_lock(&gNoOfClients_mutex);
    if ((retval = nwdiscovery_GetClientIdFromIP (ip, &node)))
    {
        memcpy (&info->IP, &gClients[node].IP, sizeof(gClients[node].IP));
        memcpy (&info->mac, &gClients[node].mac, sizeof(gClients[node].mac));
        info->clientId = node;
        info->connected = (SOCKET_INVALID_HANDLE != gClients[node].handle) ? true : false;        
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);    

    DEBUG_OUT;
    return retval;
}
