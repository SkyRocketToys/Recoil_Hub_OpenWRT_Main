//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- sync.c
//  description :- Recoil Network Sync service
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
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <pwd.h>

#include "recoilnetwork.h"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/

#define TCP_CLIENT_THREAD_STACK             8096    // 8K stack for each client thread
#define MAX_INCOMING_BYTES                  (uint32_t)(sizeof(ProtocolHeaderV1_read) + sizeof(syncClientData_t))  // maximum bytes to expect from the clients on a single transfer
#define TIMEOUT_TO_CHECK_INCOMING_PACKETS   100000  // uSeconds
#define RECEIVER_THREAD_SLEEP               50000   // uSeconds
#define BROADCAST_THREAD_SLEEP              100000  // uSeconds

/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

// client global data structure
typedef struct
{
    long                BytesReceived;      // no of bytes received so far
    long                BytesTransferred;   // no of bytes transferred so far
    int                 handle;             // socket handle
    pthread_mutex_t     mutex;              // used to protect clients critical data    
    bool                valid;              // flag to say whether the data is valid
    syncClientData_t    sdata;              // client sync data
} ClientInfo_t;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/

static ClientInfo_t gClients[MAX_NETWORK_CLIENTS];  // global data structure for max clients supported
static volatile int gNoOfClients = 0;               // global clients count - must be protected with gNoOfClients_mutex to add or remove a client
static pthread_mutex_t gNoOfClients_mutex;          // protects addition or deletion of a client to the global data structure
static int ghMainSocket;                            // main socket to accept incoming connections for network sync
static UTILS_Thread_t gReceiverThread;             // thread id for the Receiver thread - receives all the data coming from the network clients
static UTILS_Thread_t gBroadcastThread;            // thread id for the Broadcast thread - broadcast the network sync data to all the network clients
static UTILS_Thread_t gHelperThread;               // thread id for the Helper thread - which is responsible for incoming request for sync and manages them

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

// Get a free client index
// SUCCESS - returns a free index from global client array on
// FAILURE - INVALID_RETURN_VALUE
int nwsync_GetFreeClientIndex (void)
{
    uint32_t node = INVALID_RETURN_VALUE;
    DEBUG_IN;

    pthread_mutex_lock(&gNoOfClients_mutex);
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
        DEBUG_ERROR("System reached clients maximum limit - gNoOfClients(%d)", gNoOfClients);
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_OUT;
    return (node);
}

// ensure this function is called with gNoOfClients_mutex taken/locked
void nwsync_ReleaseClientNode(uint32_t node, bool alive)
{
    DEBUG_IN;

    // claim the client mutex
    pthread_mutex_lock(&gClients[node].mutex);  // nested lock - ensure we are dead lock free

    DEBUG_MIL("Deleting Sync client(%d) socket(%d) Tx(%ld) Rx(%ld) bytes alive(%d)", 
        node, gClients[node].handle, gClients[node].BytesTransferred, gClients[node].BytesReceived, alive);

    gClients[node].BytesTransferred = 0;
    gClients[node].BytesReceived = 0;

    // free the socket
    if (SYSTEM_CALL_ERROR == close(gClients[node].handle)) 
    {
        DEBUG_ERROR("Socket(%d) close failed(%d, %s) for clientid(%d)", gClients[node].handle, errno, strerror(errno), node);
    }
    gClients[node].handle = SOCKET_INVALID_HANDLE;      // reset handle
    pthread_mutex_unlock(&gClients[node].mutex);
    pthread_mutex_destroy(&gClients[node].mutex);       // destroy the client mutex   
    --gNoOfClients; // decrement the clients count

    DEBUG_OUT;
    return;
}

// Terminates the system and free all the resources allocated
// If we fail to free a resource, throw an error and continue with other resources
// NO_RETURN_VALUE
void nwsync_SystemTerminate (void)
{
    uint32_t node, noOfClients, tmp_gNoOfClients;
    DEBUG_IN;

    // we are removing all the clients - take the global clients lock
    pthread_mutex_lock(&gNoOfClients_mutex);
    tmp_gNoOfClients = gNoOfClients;
    for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < tmp_gNoOfClients)); node++)
    {
        if (SOCKET_INVALID_HANDLE != gClients[node].handle)
        {
            nwsync_ReleaseClientNode(node, true);
            noOfClients++;
        }
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_OUT;
    return;      
}

// buffer - MUST be valid, no check is made inside the function
void nwsync_ParseNodePacket (uint32_t node, uint8_t* buffer, uint32_t size)
{
    SyncClientPacket_t *pkt = (SyncClientPacket_t*)buffer;

    if (!UTILS_VerifyPacketHeader (buffer, size, ClientDataSync, MAX_INCOMING_BYTES, NULL))    // handle only the full packets
    {
        DEBUG_ERROR("Network packet header verification failed");
    }
    else
    {
        // read the first uint32_t after the header field - client id is present here !!!
        uint32_t clientid = ntohl(*(uint32_t*)&pkt->body.clientid);

        if ((uint8_t)(clientid & 0xFF) != (uint8_t)(node & 0xFF)) // check the node id and client id matches - MUST match !!!
        {
            DEBUG_ERROR("Client id(%d) does not match the node id(%d)", clientid, (uint8_t)(node & 0xFF));
        }
        else
        {
            pthread_mutex_lock(&gClients[node].mutex);      // CRITICAL DATA START
            // no need for changing the order, we are going to send as we receive.
            // data are expected to be in htonl order and it will be sent in the same order
            gClients[node].sdata = pkt->body;
            gClients[node].valid = true;
            pthread_mutex_unlock(&gClients[node].mutex);    // CRITICAL DATA END
            DEBUG_EINFO("Client id(%d) sync packet updated...", clientid);
        }            
    }

    DEBUG_OUT;
    return;
}

// Helper thread to handle network clients incoming message
void* nwsync_ReceiverThread (void *pArg)
{
    uint8_t *buffer = NULL;
    int RxSize = 0;
    int activity;
    int noOfClients, node;

    int nfds = 0;       // nfds is the highest-numbered file descriptor in readfds/writefds/exceptfds plus 1. 
    fd_set fds;         // We only use readfds, fds is the handle used to watch for any characters become available for reading in the attached socket handles
    struct timeval tv;

    DEBUG_IN;

    // allocate buffer to hold only one packet at a time - as in the SyncClientPacket_t
    if (NULL == (buffer = (uint8_t*)malloc(MAX_INCOMING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)MAX_INCOMING_BYTES);
        gReceiverThread.run = false;
    }

    while (gReceiverThread.run)
    {
        DEBUG_EINFO ("Starting new receiver cycle...");
        /* wait for 100mS to check whether we got any traffic in the connected clients */
        tv.tv_sec = 0;
        tv.tv_usec = TIMEOUT_TO_CHECK_INCOMING_PACKETS;

        FD_ZERO(&fds);
        nfds = 0;
        pthread_mutex_lock(&gNoOfClients_mutex);
        for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
        {
            if (INVALID_RETURN_VALUE != gClients[node].handle)
            {
                DEBUG_EINFO ("Found a valid client(%d) handle(%d)...", node, gClients[node].handle);
                FD_SET (gClients[node].handle, &fds);
                if (gClients[node].handle > nfds)
                {
                    nfds = gClients[node].handle;
                }
            }
        }

        // we might have a stale socket in the wait queue (on unlock  we actually allow the system to remove a client) but otherwise we will block all other access on this wait. 
        // stale handle wont have any response or we dont need them - so safe to unlock now
        pthread_mutex_unlock(&gNoOfClients_mutex);

        // check whether we got any activity on any of the clients connected
        if (SYSTEM_CALL_ERROR == (activity = select(nfds+1, &fds, NULL, NULL, &tv)))
        {
            DEBUG_ERROR("select() failed (%d, %s) nfds(%d)", errno, strerror(errno), nfds);
        }
        else if (activity)
        {
            DEBUG_EINFO("Somedata is available to read from one of the clients..");

            pthread_mutex_lock(&gNoOfClients_mutex);
            for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
            {
                if ((INVALID_RETURN_VALUE != gClients[node].handle) && (FD_ISSET(gClients[node].handle, &fds)))
                {
                    DiscoveryClientInfo_t info;

                    // get client info - should not fail, if we fail send the packet anyway - but if not failed check the connection status and then send data
                    // on failure load an empty string
                    if (!NWDISCOVERY_GetClientInfoFromClientId(node, &info)) { 
                        strcpy ((char*)info.IP, "");
                        info.connected = true;
                    }

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
                            DEBUG_ERROR("recv() failed(%d, %s) for client(%s, %d)", errno, strerror(errno), info.IP, node);
                        }
                    }
                    else if (RxSize == 0)
                    {
                        DEBUG_ERROR("Client(%s, %d) disconnected unexpectedly", info.IP, node);
                        nwsync_ReleaseClientNode(node, false);
                    }
                    else
                    {
                        DEBUG_EINFO("received %4d bytes of data from client(%s, %2d) data[%2x %2x %2x %2x][%2x %2x %2x %2x]", RxSize, info.IP, node, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
                        DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[8],  buffer[9],  buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
                        DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[21], buffer[22], buffer[23]);
                        DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[24], buffer[25], buffer[26], buffer[27], buffer[28], buffer[29], buffer[30], buffer[31]);
                        DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[32], buffer[33], buffer[34], buffer[35], buffer[36], buffer[37], buffer[38], buffer[39]);

                        gClients[node].BytesReceived += RxSize;
                        if (info.connected)
                        {
                            nwsync_ParseNodePacket(node, buffer, (uint32_t)RxSize);
                        }
                        else
                        {
                            DEBUG_ERROR("Client(%d) discovery status is disconnected !! dropping sync data", node);
                            RECOIL_SendError(gClients[node].handle, (uint8_t)node, DISCOVERY_NOT_CONNECTED);
                        }
                    }
                }                    
            }
            pthread_mutex_unlock(&gNoOfClients_mutex);

        }
        else
        {
            DEBUG_EINFO ("Timeout on the select...");
        }
        usleep(RECEIVER_THREAD_SLEEP);
    }

    // free the buffer
    if (buffer)
        free(buffer);

    DEBUG_MIL("Network Sync [receiver thread] shutdown complete...");

    // terimate this thread
    DEBUG_OUT;    
    pthread_exit(NULL);
}

// broadcast thread to send data to the clients
// System can create upto MAX_NETWORK_CLIENTS threads to handle the incoming requests
void* nwsync_BroadcastThread (void *pArg)
{
    uint32_t noOfClients, node;
    uint8_t *buffer = NULL;
    SyncServerPacket_t *pkt;

    DEBUG_IN;

    // ensure we have allocated for the maximum usage (beware !! system can be a low memory system)
    if (NULL == (buffer = (uint8_t*)malloc(sizeof(SyncServerPacket_t))))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)sizeof(SyncServerPacket_t));
        gBroadcastThread.run = false;
    }

    // fill the values which dont change in the cycle
    pkt = (SyncServerPacket_t*)buffer;

    while (gBroadcastThread.run)
    {
        uint16_t len;

        pthread_mutex_lock(&gNoOfClients_mutex);

        DEBUG_EINFO ("Starting new broadcast cycle...");
        // NOTE: even though we have large enough buffer to hold max client supported data
        // when a broadcast packet is generated it only uses space upto current connected clients
        len = ((sizeof(syncClientData_t) * gNoOfClients) + sizeof(SyncServerHeader_t));        
        UTILS_SetHeader (&pkt->header, ServerDataSync, len);
        pkt->syncHeader.NoOfClients = gNoOfClients;
        for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
        {
            if (SOCKET_INVALID_HANDLE != gClients[node].handle)
            {
                DEBUG_EINFO ("Found a connected client = %d", node);
                if (gClients[node].valid)
                {
                    DEBUG_EINFO ("data is valid for client = %d", node);
                    pthread_mutex_lock(&gClients[node].mutex);      // second level lock here - ensure we are deadlock proof
                    memcpy (&pkt->syncData[noOfClients], &gClients[node].sdata, sizeof(syncClientData_t));
                    gClients[node].BytesTransferred += (pkt->header.length+sizeof(ProtocolHeaderV1_write)); // will be sent in a short while
                    pthread_mutex_unlock(&gClients[node].mutex);
                    noOfClients++;  // increment on every valid client
                }
            }
        }

        if (noOfClients)    // on atleast one client
        {
            DEBUG_EINFO("Broadcast data %4d bytes data[%2x %2x %2x %2x]", (int32_t)(len+sizeof(ProtocolHeaderV1_write)), buffer[0], buffer[1], buffer[2], buffer[3]);
            DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[4],  buffer[5],  buffer[6],  buffer[7],  buffer[8],  buffer[9],  buffer[10], buffer[11]);
            DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[12], buffer[13], buffer[14], buffer[15], buffer[16], buffer[17], buffer[18], buffer[19]);
            DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[20], buffer[21], buffer[22], buffer[23], buffer[24], buffer[25], buffer[26], buffer[27]);
            DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[28], buffer[29], buffer[30], buffer[31], buffer[32], buffer[33], buffer[34], buffer[35]);
            DEBUG_EINFO("data[%2x %2x %2x %2x][%2x %2x %2x %2x]", buffer[36], buffer[37], buffer[38], buffer[39], buffer[40], buffer[41], buffer[42], buffer[43]);

            // below debug prints first 3 clients clientid & syncdata7 on the broadcast buffer
            DEBUG_EINFO("data[%2x %2x %2x %2x]NC[%2x %2x %2x %2x]", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4],  buffer[5],  buffer[6],  buffer[7]);
            DEBUG_EINFO("-- data[%2x %2x %2x %2x]00[%2x %2x %2x %2x]", buffer[8], buffer[9], buffer[10], buffer[11], buffer[40], buffer[41], buffer[42], buffer[43]);
            DEBUG_EINFO("data[%2x %2x %2x %2x]01[%2x %2x %2x %2x]", buffer[44], buffer[45], buffer[46], buffer[47], buffer[76], buffer[77], buffer[78], buffer[79]);
            DEBUG_EINFO("data[%2x %2x %2x %2x]02[%2x %2x %2x %2x]---", buffer[80], buffer[81], buffer[82], buffer[83], buffer[112], buffer[113], buffer[114], buffer[115]);

            // broadcast to all the clients
            for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < gNoOfClients)); node++)
            {
                if (SOCKET_INVALID_HANDLE != gClients[node].handle)
                {
                    DiscoveryClientInfo_t info;

                    // get client info - should not fail, if we fail send the packet anyway - but if not failed check the connection status and then send data
                    if (!NWDISCOVERY_GetClientInfoFromClientId(node, &info)) { 
                        info.connected = true;
                    }

                    if (info.connected)
                    {
                        bool connected;
                        uint32_t sent;

                        // if we are not able to send the bytes - dont retry, with sync we can update this in the next cycle - socket must be busy if we get sent = 0 and return value is true
                        if ((UTILS_TCP_SendData(gClients[node].handle, buffer, (len+sizeof(ProtocolHeaderV1_write)), &sent, &connected)) && (sent == (len+sizeof(ProtocolHeaderV1_write))))
                        {
                            DEBUG_EINFO("Broadcast data %d bytes to client(%d) data[%2x %2x %2x %2x]", (int32_t)(len+sizeof(ProtocolHeaderV1_write)), node, buffer[0], buffer[1], buffer[2], buffer[3]);
                        }
                        else if (false == connected) // broken pipe ??
                        {
                            DEBUG_ERROR("Client(%s, %d) disconnected unexpectedly", info.IP, node);
                            nwsync_ReleaseClientNode(node, false);
                        }
                    }
                    else
                    {
                        DEBUG_ERROR("Client(%d) discovery status is disconnected !! dropping sync data", node);
                        RECOIL_SendError(gClients[node].handle, (uint8_t)node, DISCOVERY_NOT_CONNECTED);
                    }
                    noOfClients++;
                }
            }
        }

        pthread_mutex_unlock(&gNoOfClients_mutex);

        usleep(BROADCAST_THREAD_SLEEP);   // convert milli to micro seconds for the api
    }

    // free the buffer
    if (buffer)
        free(buffer);

    DEBUG_MIL("Network Sync [broadcast thread] shutdown complete...");

    // terimate this thread
    DEBUG_OUT;
    pthread_exit(NULL);
}

void* nwsync_HelperThread (void *pArg)
{
    int client_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client;    
    uint32_t node;
    int hSocket = *(int*)pArg; // socket to accept incoming connections
    int status;

    DEBUG_IN;

    DEBUG_EINFO("Listening for incoming connections(max:%d) on port %s", MAX_NETWORK_CLIENTS, gNetworkSyncPort);
    fcntl(hSocket, F_SETFL, O_NONBLOCK); /* configure the socket to work in non-blocking state	*/

    while (gHelperThread.run) // while the system is allowed to run
    {
        int tmphandle = 0;

        DEBUG_EINFO("calling accept");
        if (SYSTEM_CALL_ERROR != (tmphandle = accept (hSocket, (struct sockaddr *) &client, (socklen_t*) &client_size)))
        {
            DiscoveryClientInfo_t info;

            // get the client id from discovery service
            if (NWDISCOVERY_GetClientInfoFromIP ((uint8_t*)inet_ntoa(client.sin_addr), &info) && info.connected)
            {
                // quick sanity check whether we got this id free ???
                if (INVALID_RETURN_VALUE == gClients[info.clientId].handle)
                {
                    node = info.clientId;

                    // create the client mutex
                    if (0 == (status = pthread_mutex_init (&gClients[node].mutex, NULL)))
                    {
                        pthread_mutex_lock(&gNoOfClients_mutex);
                        gClients[node].handle = tmphandle;
                        gClients[node].valid = false;
                        gNoOfClients++; // we got an additional client
                        pthread_mutex_unlock(&gNoOfClients_mutex);
                        DEBUG_INFO("Connected successfully to IP(%s) socket(%d) on node(%d)", info.IP, gClients[node].handle, node);
                    }
                    else
                    {
                        DEBUG_FATAL("Client mutex create failed(%d, %s) on new connection.. very unlikely - investigate this", errno, strerror(errno));
                        DEBUG_FATAL("Since the connection is accepted we might end up with broken clientip(%s) - but this call should never fail anyway!!", inet_ntoa(client.sin_addr));
                        RECOIL_SendError(tmphandle, (uint8_t)node, SYNC_INTERNAL_ERROR);
                    }
                }
                else
                {
                    DEBUG_FATAL("Sync Connection is already active for this client(%d, %s) on socket handle(%d) BytesRxd(%ld) BytesTxd(%ld)", 
                        info.clientId, info.IP, gClients[info.clientId].handle, gClients[info.clientId].BytesReceived, gClients[info.clientId].BytesTransferred);
                    RECOIL_SendError(tmphandle, (uint8_t)info.clientId, SYNC_ALREADY_CONNECTED);
                }
            }
            else
            {
                DEBUG_FATAL("Client requesting for a network sync, without discovery complete !!!");
                RECOIL_SendError(tmphandle, (uint8_t)INVALID_CLIENT_ID, DISCOVERY_NOT_CONNECTED);
            }
        }
        else if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            DEBUG_EINFO("No new request, sleep until we recheck again...");
            usleep (RETRY_FOR_CONNECTION_REQUEST*1000); // we dont have new request - sleep before we go for a fresh wait       
        }
        else
        { 
            DEBUG_FATAL("Incoming connection accept() failed (%d, %s)", errno, strerror(errno));
            usleep (RETRY_FOR_CONNECTION_REQUEST*1000); // we dont have new request - sleep before we go for a fresh wait       
        }
    }

    DEBUG_INFO("System received a shut down req gNoOfClients(%d)", gNoOfClients);

    // Close the incoming socket
    if (SOCKET_INVALID_HANDLE != hSocket)
    {
        if (SYSTEM_CALL_ERROR == close(hSocket))
        {
            DEBUG_ERROR("Socket(%d) close failed(%d, %s)", hSocket, errno, strerror(errno));
        }
    }    

    // tear down the system
    nwsync_SystemTerminate();
    closelog ();

    DEBUG_MIL("Network Sync [helper thread] shutdown complete...");

    // terimate this thread
    DEBUG_OUT;
    pthread_exit(NULL);
}

// entry point for the network sync sub system
bool NWSYNC_Start (void)
{
    int status;
    uint32_t node;
    struct addrinfo hints, *server = NULL;
    int option = 1;
    pthread_attr_t attr;
    bool retval = false;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_SYNC", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // Configure the socket
    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;              // set ipv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    // broadcast and receiver thread must start once it is created
    gBroadcastThread.run = true;
    gReceiverThread.run = true;
    gHelperThread.run = true;

    for (node = 0; node < MAX_NETWORK_CLIENTS; node++) {
        // Reset all the socket handles - this is used to validate a node !!
        gClients[node].handle = SOCKET_INVALID_HANDLE;
    }

    // Get socket params and server IP details
    if (0 != (status = getaddrinfo(NULL, gNetworkSyncPort, &hints, &server)))
    {
        DEBUG_FATAL("getaddrinfo failed(%s:%d)", gai_strerror(status), status);
    }
    // Create a TCP socket
    else if (SYSTEM_CALL_ERROR == (ghMainSocket = socket (server->ai_family, server->ai_socktype, server->ai_protocol)))
    {
        DEBUG_FATAL("Socket open failed(%d, %d, %d)", server->ai_family, server->ai_socktype, server->ai_protocol);
    }
    // One port for all incoming connections - set SO_REUSEPORT to true
    //else if(SYSTEM_CALL_ERROR == (status = setsockopt(ghMainSocket, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option))))
    else if(SYSTEM_CALL_ERROR == (status = setsockopt(ghMainSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))))
    {
        DEBUG_FATAL("Socket config failed(%d, %s) hndl(%d) option(%d)", errno, strerror(errno), ghMainSocket, option);
    }
    // bind sock addr to socket
    else if (SYSTEM_CALL_ERROR == (status = bind(ghMainSocket, server->ai_addr, server->ai_addrlen)))
    {
        DEBUG_FATAL("Binding failed on socket(%d) error(%d, %s)", ghMainSocket, errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (listen (ghMainSocket, MAX_NETWORK_CLIENTS))))
    {
        DEBUG_FATAL("Listening on socket failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_init(&attr))))
    {
        DEBUG_FATAL("pthread_attr_init failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_setstacksize(&attr, TCP_CLIENT_THREAD_STACK))))
    {
        DEBUG_FATAL("pthread_attr_setstacksize failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = pthread_mutex_init (&gNoOfClients_mutex, NULL)))
    {
        DEBUG_FATAL("Global client mutex create failed(%d, %s)", errno, strerror(errno));
    }
    // create the receiver thread
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (&gReceiverThread.thread, &attr, nwsync_ReceiverThread, (void*)0)))    
    {
        DEBUG_FATAL("Receiver thread creation failed(%d, %s)", errno, strerror(errno));
    }
    // create the broadcast thread
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (&gBroadcastThread.thread, &attr, nwsync_BroadcastThread, (void*)0)))    
    {
        DEBUG_FATAL("Broadcast thread creation failed(%d, %s)", errno, strerror(errno));
    }
    // create the helper thread
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (&gHelperThread.thread, &attr, nwsync_HelperThread, (void*)&ghMainSocket)))    
    {
        DEBUG_FATAL("Receiver thread creation failed(%d, %s)", errno, strerror(errno));
    }  
    else
    {
        DEBUG_MIL("Network Sync started successfully on port(%s)...", gNetworkSyncPort);
        retval = true;
    }

    DEBUG_OUT;
    return (retval);
}

bool NWSYNC_Stop (void)
{
    void *pRetval = NULL;
    int status;
    bool retval = false;  

    DEBUG_IN;
    DEBUG_MIL ("Request to shut down [network sync] arrived...");

    // signal helper thread to terminate
    gHelperThread.run = false;
    if (0 != (status = pthread_join(gHelperThread.thread, &pRetval)))
    {
        DEBUG_ERROR("Unable to wait for Helper thread to exit...err(%d, %s)", errno, strerror(errno));
    }

    // signal receiver thread to terminate
    gReceiverThread.run = false;
    if (0 != (status = pthread_join(gReceiverThread.thread, &pRetval)))
    {
        DEBUG_ERROR("Unable to wait for Receiver thread to exit...err(%d, %s)", errno, strerror(errno));
    }

    // signal broadcast thread to terminate
    gBroadcastThread.run = false;
    if (0 != (status = pthread_join(gBroadcastThread.thread, &pRetval)))
    {
        DEBUG_ERROR("Unable to wait for Broadcast thread to exit...err(%d, %s)", errno, strerror(errno));
    }
    else
    {
        retval = true;
        DEBUG_MIL ("Request to shut down [network sync] is complete...");
    }    

    DEBUG_OUT;
    return retval;
}

int NWSYNC_GetClientCount (void)
{
    int NoOfClients;
    DEBUG_IN;    
    pthread_mutex_lock(&gNoOfClients_mutex);
    NoOfClients = gNoOfClients;
    pthread_mutex_unlock(&gNoOfClients_mutex);
    DEBUG_OUT;
    return (NoOfClients);
}
