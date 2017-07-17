//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- time.c
//  description :- Recoil Network time service
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

// Enable here for file level debug message
//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/
#define MAX_INCOMING_BYTES                  (sizeof(TimeClientPacket_t) - 2)  // maximum bytes to expect from the clients on a single transfer (2 bytes extra in the structure is added for padding)
#define RECEIVER_THREAD_SLEEP               100000  // uSeconds
#define TIMEOUT_TO_CHECK_INCOMING_PACKETS   100000  // uSeconds
/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

// time latency stats
typedef struct
{
    uint64_t        sum;        // sum of all the latency calculated
    uint32_t        count;      // latency sample count
    uint32_t        last;       // last latency
    uint16_t        last_secs;  // last network time - seconds resolution
    uint32_t        last_nsecs; // last network time - nano seconds resolution
} Latency_t;

// client global data structure
typedef struct
{
    int                 handle;             // copy of socket handle - owned by discovery module
    bool                connected;          // indicates whether we are still connected to the client or not
    Latency_t           latency;            // latency info
    NWTIME_Callback_fn  nwtime_cb;
} TimeInfo_t;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
static TimeInfo_t gClients[MAX_NETWORK_CLIENTS];    // global time stats structure for max clients supported
static uint32_t gNoOfClients = 0;
static pthread_mutex_t gNoOfClients_mutex;          // protects addition or deletion of a client to the global data structure
static NWTIME_Callback_fn gDisconnect_cb = NULL;
static NWTIME_Callback_fn gNwTime_cb = NULL;
static UTILS_Thread_t gHelperThread;               // time helper thread

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

// must be called with this mutex:gNoOfClients_mutex taken
// this call is not REENTRANT, gDisconnect_cb MUST not call any NWTIME_ api's
void nwtime_ReleaseClientNode (uint32_t node)
{
    if (NULL != gDisconnect_cb)
    {
        if (true == gDisconnect_cb(node))
        {
            DEBUG_INFO("Callback for disconnect(id:%u) is complete", node);
        }
    }

    gClients[node].handle = INVALID_RETURN_VALUE;
    gClients[node].connected = false;
    // TODO is it necessary ?
    //memset(&gClients[node].latency, 0,  sizeof(Latency_t));    
    //gClients[node].nwtime_cb = NULL;
    gNoOfClients--;
}

// Terminates the system and free all the resources allocated
// If we fail to free a resource, throw an error and continue with other resources
// NO_RETURN_VALUE
void nwtime_SystemTerminate (void)
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
            nwtime_ReleaseClientNode(node);
            noOfClients++;
        }
    }
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_OUT;        
}

// buffer - MUST be valid, no check is made inside the function
void nwtime_ParseNodePacket (uint32_t node, uint8_t* buffer, int size)
{
    TimeClientPacket_t *pkt_in = (TimeClientPacket_t*)buffer;

    if (!UTILS_VerifyPacketHeader (buffer, size, NetworkTime, MAX_INCOMING_BYTES, NULL))    // handle only the full packets
    {
        DEBUG_ERROR("Network packet header verification failed");
    }
    else if (pkt_in->id != (uint8_t)(node & 0xFF)) // check the node id and client id matches - MUST match !!!
    {
        DEBUG_ERROR("Client id(%d) does not match the node id(%d)", pkt_in->id, (uint8_t)(node & 0xFF));
    }
    else if (pkt_in->type != (uint8_t)(NWTIME_REQUEST & 0xFF)) // check the time packet type - MUST match NWTIME_REQUEST !!!
    {
        DEBUG_ERROR("Ping packet type(%d) does not match expected time packet type(%d)", pkt_in->type, (uint8_t)(NWTIME_REQUEST & 0xFF));
    }
    else
    {
        // send the response the client
        TimeServerPacket_t pkt_out;
        struct timespec t;

        // prepare the header
        UTILS_SetHeader(&pkt_out.header, NetworkPing, (uint16_t)(sizeof(PingServerPacket_t) - sizeof(ProtocolHeaderV1)));
        clock_gettime(CLOCK_MONOTONIC, &t);

        pkt_out.id      = (uint8_t)pkt_in->id;
        pkt_out.type    = (uint8_t)NWTIME_REPLY;
        pkt_out.t_secs  = (uint16_t)(t.tv_sec & ProtocolHeaderV1_PING_NWTIME_SECS_MASK);
        pkt_out.t_nsecs = (uint32_t)(t.tv_nsec & ProtocolHeaderV1_PING_NWTIME_NSECS_MASK);

        // send the response
        DEBUG_EINFO("Time response (%x %x %x) to client(%d)", pkt_out.type, pkt_out.t_secs, pkt_out.t_nsecs, node);
#ifdef __DEBUG
        {        
            uint8_t *p = (uint8_t *)&pkt_out;
            DEBUG_INFO("pkt[%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x]", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]);
        }
#endif
        (void) send (gClients[node].handle, (const void *)&pkt_out, sizeof(TimeServerPacket_t), 0);
    }

    return;
}


// Helper thread to handle network clients incoming time message
void* nwtime_HelperThread (void *pArg)
{
    uint8_t *buffer = NULL;
    int RxSize = 0;
    int activity;
    uint32_t noOfClients, node;

    int nfds = 0;       // nfds is the highest-numbered file descriptor in readfds/writefds/exceptfds plus 1. 
    fd_set fds;         // We only use readfds, fds is the handle used to watch for any characters become available for reading in the attached socket handles
    struct timeval tv;

    DEBUG_IN;

    // allocate buffer to hold only one packet at a time - as in the SyncClientPacket_t
    if (NULL == (buffer = (uint8_t*)malloc(MAX_INCOMING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", (uint32_t)MAX_INCOMING_BYTES);
        gHelperThread.run = false;
    }

    while (gHelperThread.run)
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
            if ((INVALID_RETURN_VALUE != gClients[node].handle) && (false != gClients[node].connected))
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

        // only when we have active clients connected
        if (0 != nfds)
        {
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
                        if (!NWDISCOVERY_GetClientInfoFromClientId(node, &info)) {
                            strcpy ((char*)info.IP, ""); 
                        } // on failure load an empty string
                        
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
                            nwtime_ReleaseClientNode(node);
                        }
                        else
                        {
                            DEBUG_EINFO("received %4d bytes of data from client(%s, %2d) data[%2x %2x %2x %2x %2x %2x]", 
                                RxSize, info.IP, node, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                            nwtime_ParseNodePacket(node, buffer, RxSize);

                            // if global callback is registered - call it now
                            if (NULL != gNwTime_cb)
                            {
                                gNwTime_cb(node);
                            }

                            // if client specific callback is registered - call it now
                            if (NULL != gClients[node].nwtime_cb)
                            {
                                gClients[node].nwtime_cb(node);
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
        }            
        usleep(RECEIVER_THREAD_SLEEP);
    }

    // free the buffer
    if (buffer)
        free(buffer);

    DEBUG_MIL("Network Sync [receiver thread] shutdown complete...");

    // terimate this thread
    pthread_exit(NULL);

    DEBUG_OUT;
}

bool NWTIME_Start()
{
    bool retval = false;
    pthread_attr_t attr;
    uint16_t node;
    int status;

    // reset all the clients internal data
    for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
    {
        gClients[node].handle = INVALID_RETURN_VALUE;
        gClients[node].connected = false;
        gClients[node].nwtime_cb = NULL;
    }

    // helper thread must start once it is created    
    gHelperThread.run = true;

    if (SYSTEM_CALL_ERROR == (status = (pthread_attr_init(&attr))))
    {
        DEBUG_ERROR("pthread_attr_init failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_setstacksize(&attr, PING_HELPER_THREAD_STACK))))
    {
        DEBUG_ERROR("pthread_attr_setstacksize failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = pthread_mutex_init (&gNoOfClients_mutex, NULL)))
    {
        DEBUG_ERROR("Global client mutex create failed(%d, %s)", errno, strerror(errno));
    }    
    // create the helper thread
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (&gHelperThread.thread, &attr, nwtime_HelperThread, NULL)))
    {
        DEBUG_ERROR("Helper thread creation failed(%d, %s)", errno, strerror(errno));
    }
    else
    {
        DEBUG_MIL("Network Time started successfully on port(%s)...", gNetworkDiscoveryPort);        
        retval = true;
    }

    return retval;
}

bool NWTIME_Stop (void)
{
    void *pRetval = NULL;
    int status;
    bool retval = false;

    DEBUG_MIL ("Request to shut down [network time] arrived...");

    // signal helper thread to terminate
    gHelperThread.run = false;
    if (0 != (status = pthread_join(gHelperThread.thread, &pRetval)))
    {
        DEBUG_ERROR("Unable to wait for helper thread to exit...err(%d, %s)", errno, strerror(errno));
    }
    else
    {
        retval = true;
        DEBUG_MIL ("Request to shut down [network time] is complete...");
    }

    return retval;
}

// type and cb must be valid - no explicit check here !!!
bool NWTIME_RegisterCallback(NWTIME_CallbackType type, NWTIME_Callback_fn cb)
{
    bool retval = true;
    if (type == NWTIME_DISCONNECT)
    {
        gDisconnect_cb = cb;
    }
    else if (type == NWTIME_TIME_REQUEST)
    {
        gNwTime_cb = cb;
    }
    else
    {
        retval = false;
    }

    return retval;
}

// NOTE: ClientId and callback functions must be valid - no explicit check here !!!
bool NWTIME_AddClient (uint32_t clientId, int hSock, NWTIME_Callback_fn cb)
{
    pthread_mutex_lock(&gNoOfClients_mutex);
    gClients[clientId].handle = hSock;
    gClients[clientId].connected = true;
    memset(&gClients[clientId].latency, 0,  sizeof(Latency_t));
    gClients[clientId].nwtime_cb = cb;
    gNoOfClients++;
    pthread_mutex_unlock(&gNoOfClients_mutex);

    DEBUG_EINFO("Successfully added client(%d) to nw-time listener", clientId);

    return true;
}

bool NWTIME_GetClientStatus (uint32_t clientId, ClientPingStats_t stat)
{
    pthread_mutex_lock(&gNoOfClients_mutex);

    stat.handle = gClients[clientId].handle;
    stat.connected = gClients[clientId].connected;
    stat.latency = (uint32_t)(gClients[clientId].latency.sum / gClients[clientId].latency.count);

    pthread_mutex_unlock(&gNoOfClients_mutex);

    return true;
}
