//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- event.c
//  description :- Recoil Network Event based comms
//  author      :- Rajesh Gunasekaran  (rajg@hotgen.com)
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

// Enable here for file level debug message
//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/
#warning fix_this_with_correct_value
#define MAX_INCOMING_BYTES                  MAX_SIZE_OF_EVENT_PACKET        // Maximum payload size expected on this protocol packet payload
#define TIMEOUT_TO_CHECK_INCOMING_TRAFFIC   100000      // uSeconds
#define EVENT_THREAD_SLEEP_BETWEEN_CYCLES   1

/*
* ------------------------------------ User data types -----------------------------------------------------------
*/
// client global data structure
typedef struct
{
    int                 handle;             // socket handle
    NWEVENT_Receive_fn  callback;
} ClientInfo_t;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
static ClientInfo_t gClients[MAX_NETWORK_CLIENTS];  // global data structure for max clients supported
static pthread_mutex_t gNoOfClients_mutex;          // protects addition or deletion of a client to the global data structure
static volatile int gNoOfClients = 0;               // global clients count - must be protected with gNoOfClients_mutex to add or remove a client
static UTILS_Thread_t gHelperThread;                // thread id for the Helper thread - which is responsible for incoming request for discovery and manages them
static int ghMainSocket = 0;                        // main socket to accept incoming connections for network discovery
static uint8_t gSendBuffer[MAX_INCOMING_BYTES];     // buffer to copy the send request

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

// ensure this function is called with gNoOfClients_mutex taken/locked
void nwevent_ReleaseClientNode(uint32_t node, bool alive)
{
    DEBUG_IN;

    // close the socket
    if (!UTILS_TCP_Destroy(gClients[node].handle)) 
    {
        DEBUG_ERROR("Socket(%d) close failed(%d, %s) for clientid(%d)", gClients[node].handle, errno, strerror(errno), node);
    }
    gClients[node].handle = SOCKET_INVALID_HANDLE;
    // dont reset the callback - game server needs this to read the data, if the connection is reconnected automatically from client side
    //gClients[node].callback = NULL; 
    gNoOfClients--;

    DEBUG_OUT;
    return;
}

// Terminates the system and free all the resources allocated
// If we fail to free a resource, throw an error and continue with other resources
// NO_RETURN_VALUE
void nwevent_SystemTerminate (void)
{
    uint32_t node, noOfClients, tmp_gNoOfClients;
    DEBUG_IN;

    // we are removing all the clients - take the global clients lock
    pthread_mutex_lock(&gNoOfClients_mutex);

    // destroy existing connections
    tmp_gNoOfClients = gNoOfClients;
    for (node = 0, noOfClients = 0; ((node < MAX_NETWORK_CLIENTS) && (noOfClients < tmp_gNoOfClients)); node++)
    {
        nwevent_ReleaseClientNode(node, true);
        gClients[node].callback = NULL;
        noOfClients++;
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_OUT;
    return;    
}


// buffer - MUST be valid, no check is made inside the function
void nwevent_HandleNodePacket (uint32_t node, uint8_t* buffer, uint32_t size)
{
    uint32_t payload_size;

    // we have only read packet header so far, so set 0 for the header size in 'size'
    // event packet carrys dynamic payload size - so send 0 for the 'exp_size'
    if (!UTILS_VerifyPacketHeader (buffer, 0, GameData, 0, &payload_size))
    {
        DEBUG_ERROR("Network packet header verification failed");
    }
    else if ((payload_size + SIZE_OF_HEADER) > size)
    {
        DEBUG_FATAL("Incoming packet is large for the buffer");
        // if this happends increase the size of the receiver buffer to cater this or 
        // check we got client which sends wrong data in the header
    }
    else
    {
        bool connected;
        DEBUG_EINFO("Read the rest of the data from clientId(%d)", node);

        // this call attempts to read the entire packet in a single call, 
        // if this call fails we dont have sufficient data in the socket, come and try later
        if (UTILS_TCP_ReadNBytes (gClients[node].handle, buffer, (payload_size+SIZE_OF_HEADER), &connected))
        {
            DEBUG_EINFO("Client(%d) has received the event payload...RxSize(%d) pkt[%2x %2x %2x %2x]", 
                    node, RxSize, buffer[0], buffer[1], buffer[2], buffer[3]);

            if (gClients[node].callback)
            {
                NWEVENT_Receive_fn client_receive = gClients[node].callback;

                pthread_mutex_unlock(&gNoOfClients_mutex);

                // call the client receive callback
                // we might send one extra callback if there was detach after 
                // we unlocked the mutex and use the callback function, but that is okay.
                if (gClients[node].callback && client_receive)
                {
                    client_receive(node, &buffer[SIZE_OF_HEADER], payload_size);
                }

                pthread_mutex_lock(&gNoOfClients_mutex);
            }
        }
        // call failed
        else if (!connected)
        {
            DEBUG_ERROR("Client(%d) disconnected unexpectedly", node);
            nwevent_ReleaseClientNode(node, false);
        }
        else
        {
            DEBUG_WARN("Client(%d) not enough data in the socket for NWEvent packet... retry later", node);
        }
    }

    DEBUG_OUT;
    return;
}

void* nwevent_HelperThread (void *pArg)
{
    int client_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client;    
    uint32_t node, noOfClients;
    int hSocket = *(int*)pArg; // socket to accept incoming connections
    uint8_t *buffer = NULL;

    int nfds = 0;       // nfds is the highest-numbered file descriptor in readfds/writefds/exceptfds plus 1. 
    fd_set fds;         // We only use readfds, fds is the handle used to watch for any characters become available for reading in the attached socket handles
    struct timeval tv;
    int activity, RxSize;

    DEBUG_IN;

    DEBUG_EINFO("Listening for incoming connections(max:%d) on port %s", MAX_NETWORK_CLIENTS, gNetworkEventPort);
    fcntl(hSocket, F_SETFL, O_NONBLOCK); /* configure the socket to work in non-blocking state	*/

    // allocate buffer to hold only one packet at a time - as in the SyncClientPacket_t
    if (NULL == (buffer = (uint8_t*)malloc(MAX_INCOMING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)MAX_INCOMING_BYTES);
        gHelperThread.run = false;
    }

    while (gHelperThread.run) // while the system is allowed to run
    {
        DEBUG_EINFO("Listen for any activity on the handles...");

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

        pthread_mutex_unlock(&gNoOfClients_mutex);

        // check whether we got any activity from the network
        if (SYSTEM_CALL_ERROR == (activity = select(nfds+1, &fds, NULL, NULL, &tv)))
        {
            DEBUG_ERROR("select() failed (%d, %s) nfds(%d)", errno, strerror(errno), nfds);
        }
        else if (activity)
        {
            pthread_mutex_lock(&gNoOfClients_mutex);
            DEBUG_EINFO("Somedata is available on the network");

            // check the main handle - for new connections
            if (FD_ISSET(hSocket, &fds))
            {
                int tmphandle;

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
                            gClients[node].handle = tmphandle;
                            gNoOfClients++; // we got an additional client
                            DEBUG_INFO("Connected successfully to IP(%s) socket(%d) on node(%d)", info.IP, gClients[node].handle, node);
                        }
                        else
                        {
                            DEBUG_ERROR("Event Connection is already active for this client(%d, %s) on socket handle(%d)", 
                                info.clientId, info.IP, gClients[info.clientId].handle);
                            RECOIL_SendError(tmphandle, (uint8_t)info.clientId, SYNC_ALREADY_CONNECTED);
                        }
                    }
                    else
                    {
                        DEBUG_ERROR("Client requesting for a network event, without discovery complete !!!");
                        RECOIL_SendError(tmphandle, (uint8_t)INVALID_CLIENT_ID, DISCOVERY_NOT_CONNECTED);
                    }
                }
                else if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
                    DEBUG_WARN("No new connection request found!! did the select fail ? err(%d, %s)", errno, strerror(errno));
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
                    bool connected;

                    DEBUG_EINFO("client(%d) got some data", node);
                    if (UTILS_TCP_SnoopData (gClients[node].handle, buffer, SIZE_OF_HEADER, SIZE_OF_HEADER, (uint32_t *)&RxSize, &connected))
                    {
                        if (RxSize == 0) // this should never happen, we got activity on handle but no data
                        {
                            DEBUG_WARN("No data, but got signal for data !!! on clientId(%d)", node);
                        }
                        else
                        {
                            DEBUG_EINFO("Client(%d) has received some data...", node);
                            nwevent_HandleNodePacket(node, buffer, MAX_INCOMING_BYTES);
                        }
                    }
                    // call failed
                    else if (!connected)    // client got disconnected
                    {
                        DEBUG_ERROR("Client(%d) disconnected unexpectedly", node);
                        nwevent_ReleaseClientNode(node, false);
                    }
                    else if (RxSize != 0)   // partial packet not handled right now - TODO fix it
                    {
                        DEBUG_WARN("Broken data from the clientId(%d)", node);
                    }
                }
            }
            pthread_mutex_unlock(&gNoOfClients_mutex);            
        }
        else
        {
            DEBUG_EINFO ("Timeout on the select...");
        }

        usleep (EVENT_THREAD_SLEEP_BETWEEN_CYCLES*1000); // we dont have new request - sleep before we go for a fresh wait

    }

    DEBUG_INFO("System received a shut down req gNoOfClients(%d)", gNoOfClients);

    // stop accepting new incoming connections
    if (!UTILS_TCP_Destroy (hSocket))
    {
        DEBUG_ERROR("Socket(%d) close failed(%d, %s)", hSocket, errno, strerror(errno));
    }

    // tear down the system
    nwevent_SystemTerminate();
    closelog ();

    DEBUG_MIL("Network Discovery [helper thread] shutdown complete...");

    DEBUG_OUT;
    pthread_exit(NULL);
}

// entry point for the network event sub system
bool NWEVENT_Start (void)
{
    int status;
    uint32_t node;
    bool retval = false;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_EVENT", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // helper thread must start once it is created
    gHelperThread.run = true;

    for (node = 0; node < MAX_NETWORK_CLIENTS; node++) {
        // Reset all the socket handles - this is used to validate a node !!
        gClients[node].handle = SOCKET_INVALID_HANDLE;
        gClients[node].callback = NULL;
    }

    if (0 != (status = UTILS_TCP_Create(gNetworkEventPort, true, MAX_NETWORK_CLIENTS, &ghMainSocket)))
    {
        DEBUG_FATAL("UTILS_TCP_Create failed(%s:%d) for port(%s)", gai_strerror(status), status, gNetworkEventPort);
    }
    else if (SYSTEM_CALL_ERROR == (status = pthread_mutex_init (&gNoOfClients_mutex, NULL)))
    {
        DEBUG_FATAL("Global client mutex create failed(%d, %s)", errno, strerror(errno));
    }    
    else if (SYSTEM_CALL_ERROR == (status = UTILS_CreateThread (&gHelperThread.thread, DEFAULT_THREAD_STACK_SIZE, nwevent_HelperThread, (void*) &ghMainSocket)))
    {
        DEBUG_FATAL("Helper thread create failed (%d, %s)", errno, strerror(errno));
    }
    else
    {
        DEBUG_MIL("Network Event started successfully on port(%s)...", gNetworkEventPort);
        retval = true;
    }

    DEBUG_OUT;
    return (retval);
}

bool NWEVENT_Stop (void)
{
    void *H_retval = NULL;
    int status;
    bool retval = false;    
    
    DEBUG_IN;
    DEBUG_MIL ("Request to shut down [network event] arrived...");

    // signal helper thread to terminate
    gHelperThread.run = false;
    if (0 != (status = pthread_join(gHelperThread.thread, &H_retval)))
    {
        DEBUG_ERROR("Unable to wait for helper thread to exit...err(%d, %s)", errno, strerror(errno));
    }
    else
    {
        retval = true;
    }
    pthread_mutex_destroy(&gNoOfClients_mutex);
    DEBUG_MIL ("Request to shut down [network event] is complete...");

    DEBUG_OUT;
    return (retval);
}

Error_t NWEVENT_Attach (uint32_t clientId, NWEVENT_Receive_fn func)
{
    Error_t retval = SYSTEM_PARAM_ERROR;
    DEBUG_IN;

    if (clientId < MAX_NETWORK_CLIENTS)
    {
        pthread_mutex_lock(&gNoOfClients_mutex);
            gClients[clientId].callback = func;
        pthread_mutex_unlock(&gNoOfClients_mutex);
        retval = SYSTEM_NO_ERROR;
    }

    DEBUG_OUT;
    return (retval);
}

Error_t NWEVENT_Detach (uint32_t clientId)
{
    Error_t retval = SYSTEM_PARAM_ERROR;
    DEBUG_IN;

    if (clientId < MAX_NETWORK_CLIENTS)
    {
        pthread_mutex_lock(&gNoOfClients_mutex);
            gClients[clientId].callback = NULL;
        pthread_mutex_unlock(&gNoOfClients_mutex);
        retval = SYSTEM_NO_ERROR;
    }

    DEBUG_OUT;
    return (retval);
}

// clientId - x (single client event) or 0xFF - broadcast event to all available clients
Error_t NWEVENT_Send (uint32_t clientId, uint8_t *buffer, uint32_t buffer_size, uint32_t *sent)
{
    Error_t retval = SYSTEM_PARAM_ERROR;
    bool connected = true;

    DEBUG_IN;

    *sent = 0;

    if ((clientId < MAX_NETWORK_CLIENTS) && ((SIZE_OF_HEADER + buffer_size) <= MAX_INCOMING_BYTES))
    {
        pthread_mutex_lock(&gNoOfClients_mutex);

            if (gClients[clientId].handle != SOCKET_INVALID_HANDLE)
            {
                UTILS_SetHeader((ProtocolHeaderV1_write*)&gSendBuffer, GameData, (uint16_t)buffer_size);

                // after a decent debate to avoid this copy, app/game server guys decided to make this copy necessary to avoid game server overhead
                // this is inefficient but because of this agreement i've agreed to do this copy
                memcpy (&gSendBuffer[SIZE_OF_HEADER], buffer, buffer_size);

                if (UTILS_TCP_SendData (gClients[clientId].handle, gSendBuffer, (SIZE_OF_HEADER + buffer_size), sent, &connected))
                {
                    *sent = *sent - SIZE_OF_HEADER;
                    retval = SYSTEM_NO_ERROR;
                }
                else if (!connected)
                {
                    nwevent_ReleaseClientNode(clientId, false);
                    retval = CLIENT_DISCONNECTED_ERROR;
                }
                else
                {
                    retval = SYSTEM_INTERNAL_ERROR;
                }
            }
            else
            {
                retval = CLIENT_DISCONNECTED_ERROR;
            }

        pthread_mutex_unlock(&gNoOfClients_mutex);
    }

    DEBUG_OUT;
    return (retval);
}

bool NWEVENT_IsConnected (uint32_t clientId)
{
    bool retval = false;
    DEBUG_IN;

    if (clientId < MAX_NETWORK_CLIENTS)
    {
        pthread_mutex_lock(&gNoOfClients_mutex);
            if (gClients[clientId].handle != SOCKET_INVALID_HANDLE)
            {
                retval = true;
            }
        pthread_mutex_unlock(&gNoOfClients_mutex);
    }

    DEBUG_OUT;
    return (retval);
}


