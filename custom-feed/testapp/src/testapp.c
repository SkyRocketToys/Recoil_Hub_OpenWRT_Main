//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- testapp.c
//  description :- Recoil Network Event - Server Test app
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
#include <semaphore.h>
#include <pthread.h>

// Enable here for file level debug message
//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"

#define MAX_INCOMING_BYTES  1028

typedef struct {
    uint8_t     data[MAX_INCOMING_BYTES];
    uint32_t    data_size;
    bool        valid;
} Data_t;

static UTILS_Thread_t gHelperThread;
static pthread_mutex_t gDatamutex;
Data_t gData[MAX_NETWORK_CLIENTS];

// receive function
bool TESTAPP_CmdServerReceiveJsonData(uint32_t node, uint8_t *buffer, uint32_t buffer_size)
{
    pthread_mutex_lock(&gDatamutex);
    if (NULL == buffer || 0 == buffer_size)
    {
        DEBUG_ERROR("Invalid data on receive callback buffer(%p) buffer_size(%d)", buffer, buffer_size);
    }
    else if ((buffer_size + 4) > MAX_INCOMING_BYTES) // add the header, since we use the same buffer to send it out
    {
        DEBUG_ERROR("Incoming buffer(%d) is too large to copy with our buffer(%d)", buffer_size, MAX_INCOMING_BYTES);
    }
    else
    {
        DEBUG_EINFO("received bytes from client(%d) size(%d) pkt[%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x][%2x]", 
            node, buffer_size, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11], buffer[12]);

        memcpy (gData[node].data, buffer, buffer_size);
        gData[node].data_size = buffer_size;
        gData[node].valid = true;

        // signal helper about new data
        //sem_post(&gSignalSem);
    }
    pthread_mutex_unlock(&gDatamutex);

    return true;
}

// send function
void* TESTAPP_event_HelperThread (void *pArg)
{
    Error_t status;
    uint32_t sent, node, loop;

    gHelperThread.run = true;

    while (gHelperThread.run)
    {
        for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
        {
            pthread_mutex_lock(&gDatamutex);            
            if (gData[node].valid)
            {
                uint8_t *data = gData[node].data;
                DEBUG_EINFO("Received fresh data from client[%d]...", node);

                for (loop = 0; loop < MAX_NETWORK_CLIENTS; loop++)
                {                    
                    sent = 0;
                    if (NWEVENT_IsConnected(loop)) // only for clients who are connected !!!
                    {
                        if (SYSTEM_NO_ERROR != (status = NWEVENT_Send (loop, gData[node].data, gData[node].data_size, &sent)))
                        {
                            if (CLIENT_DISCONNECTED_ERROR != status)
                            {
                                DEBUG_ERROR("Unable to send data to client(%d) error(%d)", node, status);
                            }
                        }
                        else if (sent == gData[node].data_size) // TODO - potential race condition - but it is a test app so dont over complicate !!!
                        {
                            DEBUG_EINFO("Sent %d bytes to Client(%d) via NWEVENT pkt[%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x][%2x]", gData[node].data_size, loop, 
                                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12]);
                        }
                        else
                        {
                            DEBUG_WARN("Partial data(%d) sent Expected(%d) bytes to Client(%d) via NWEVENT", sent, gData[node].data_size, loop);
                        }
                    }
                }
                gData[node].valid = false;
                DEBUG_EINFO("d..");
            }
            pthread_mutex_unlock(&gDatamutex);            
        }

        usleep (10000); // 10mS
    }

    DEBUG_INFO("System received a shut down req");

    for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
    {
        gData[node].valid = false;  
        if (SYSTEM_NO_ERROR != (status = NWEVENT_Detach (node)))
        {
            DEBUG_ERROR("Unable to detach Client(%d) from NWEVENT error(%d)", node, status);
        }        
    }

    DEBUG_MIL("Network Test App [helper thread] shutdown complete...");

    // terimate this thread
    DEBUG_OUT;
    pthread_exit(NULL);
}

void TESTAPP_Setup (void)
{
    gNetworkDiscoveryPort = NETWORK_DISCOVERY_PORT_TESTAPP;
    gNetworkSyncPort = NETWORK_SYNC_PORT_TESTAPP;
    gNetworkPingPort = NETWORK_PING_PORT_TESTAPP;
    gNetworkTimePort = NETWORK_TIME_PORT_TESTAPP;
    gNetworkEventPort = NETWORK_EVENT_PORT_TESTAPP;

    gNetworkPingReturnPort = NETWORK_PING_RETURN_PORT_TESTAPP;
    gNetworkTimeReturnPort = NETWORK_TIME_RETURN_PORT_TESTAPP;
}

bool TESTAPP_Start (void)
{
    int32_t status, node;
    bool retval = false;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_TAPP", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    if (SYSTEM_CALL_ERROR == (status = UTILS_CreateThread (&gHelperThread.thread, DEFAULT_THREAD_STACK_SIZE, TESTAPP_event_HelperThread, NULL)))
    {
        DEBUG_ERROR("Event thread create failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = pthread_mutex_init (&gDatamutex, NULL)))
    {
        DEBUG_ERROR("Global data client mutex create failed(%d, %s)", errno, strerror(errno));
    } 
    else
    {
        for (node = 0; node < MAX_NETWORK_CLIENTS; node++)
        {
            gData[node].valid = false;            
            if (SYSTEM_NO_ERROR != (status = NWEVENT_Attach (node, TESTAPP_CmdServerReceiveJsonData)))
            {
                DEBUG_ERROR("Unable to attach Client(%d) to NWEVENT error(%d)", node, status);
            }
        }
        DEBUG_MIL("Event Test App started successfully...");
        retval = true;
    } 

    DEBUG_OUT;
    return retval;
}

bool TESTAPP_Stop (void)
{
    void *pRetval = NULL;
    int status;
    bool retval = false;  

    DEBUG_IN;
    DEBUG_MIL ("Request to shut down [test app] arrived...");

    // signal helper thread to terminate
    gHelperThread.run = false;
    if (0 != (status = pthread_join(gHelperThread.thread, &pRetval)))
    {
        DEBUG_ERROR("Unable to wait for Helper thread to exit...err(%d, %s)", errno, strerror(errno));
    }  
    else
    {
        retval = true;
        DEBUG_MIL ("Request to shut down [test app] is complete...");
    }
    pthread_mutex_destroy(&gDatamutex);

    DEBUG_OUT;
    return retval;
}
