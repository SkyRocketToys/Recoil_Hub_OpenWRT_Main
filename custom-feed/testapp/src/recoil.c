//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- recoil.c
//  description :- Recoil Network service
//  author      :- Rajesh Gunasekaran   (rajg@hotgen.com)
//

/*
* ------------------------------------ Header files  -----------------------------------------------------------
*/

#include <signal.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "recoilnetwork.h"

#define BUILD_USER "Raj"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/


/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

// client global data structure
typedef struct
{
    int                 handle;             // socket handle
    uint8_t             IP[16];             // xxx.xxx.xxx.xxx with NULL termination
    uint8_t             mac[6];             // mac address in byte array
} ClientInfo_t;

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
static sem_t gShutdownSem;

char *gNetworkDiscoveryPort = 0;                       // TCP
char *gNetworkSyncPort = 0;                            // TCP
char *gNetworkPingPort = 0;                            // UDP
char *gNetworkTimePort = 0;                            // UDP
char *gNetworkEventPort = 0;                           // TCP

char *gNetworkPingReturnPort = 0;                     // UDP
char *gNetworkTimeReturnPort = 0;                     // UDP

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/
void recoil_TermSignalHandler (int stat)
{
    DEBUG_IN;
    DEBUG_MIL("Received signal(%d) to shutdown...", stat);

    // shutdown all the subsytems
#ifdef ENABLE_TEST_APP    
    TESTAPP_Stop();         // test app
#endif
#ifdef ENABLE_RECOIL_APP    
    RECOILAPP_Stop();       // recoil app
#endif
    NWEVENT_Stop();         // network event
    NWPING_Stop();          // network ping
    NWSYNC_Stop();          // network sync
    NWTIME_Stop();          // network time
    NWDISCOVERY_Stop();     // network discovery

    sem_post(&gShutdownSem);    // all the sub systems are down, signal main thread to terminate now
    DEBUG_OUT;
}

void recoil_SocketErrorHandler (int stat)
{
    DEBUG_IN;
    DEBUG_MIL("Received signal(%d) for a socket failure... do nothing - handled in the send/read call in system ...", stat);
    DEBUG_OUT;
}

void recoil_InstallSignalHandlers (void)
{
    DEBUG_IN;

    // install signal handler
    if (SIG_ERR == signal(SIGINT, recoil_TermSignalHandler)) // Ctrl+C
    {
        DEBUG_ERROR("Installing signal handler failed");
    }
    else if (SIG_ERR == signal(SIGTERM, recoil_TermSignalHandler)) // Quit    
    {
        DEBUG_ERROR("Installing signal handler failed");
    }
    else if (SIG_ERR == signal(SIGHUP, recoil_TermSignalHandler)) // Restart   
    {
        DEBUG_ERROR("Installing signal handler failed");
    }
    else if (SIG_ERR == signal(SIGPIPE, recoil_SocketErrorHandler)) // broken pipe
    {
        DEBUG_ERROR("Installing signal handler failed");
    }

    DEBUG_OUT;
    return;
}

bool RECOIL_SendError (int handle, uint8_t id, NetworkErrors err)
{
    ErrorPacket_t pkt;

    DEBUG_IN;
    DEBUG_ERROR("SENDING NETWORK ERROR TO CLIENT err(%d)", err);

    // send the response
    UTILS_SetHeader(&pkt.header, BS_Error, (uint16_t)ERROR_PACKET_SIZE_NO_HEADER);
    DEBUG_EINFO("Error response (%x %x %x) to client(%d)", pkt.type, pkt.id, pkt.error, node);
#ifdef __DEBUG
    {
        uint8_t *p = (uint8_t *)&pkt;
        DEBUG_INFO("pkt[%2x %2x %2x %2x][%2x %2x]", p[0], p[1], p[2], p[3], p[4], p[5]);
    }
#endif
    (void) send (handle, (const void *)&pkt, (sizeof(ProtocolHeaderV1_write) + ERROR_PACKET_SIZE_NO_HEADER), 0);

    // free the socket
    if (SYSTEM_CALL_ERROR == close(handle)) 
    {
        DEBUG_ERROR("Socket(%d) close failed(%d, %s)", handle, errno, strerror(errno));
    }

    DEBUG_OUT;
    return true;
}


int main (void)
{
    DEBUG_IN;
    DEBUG_MIL ("Welcome to RECOIL v%d.%d-%d_%cE Build[%s %s %s@hotgen.com]", 
        RECOIL_MAJOR_VERSION, RECOIL_MINOR_VERSION, RECOIL_PACKAGE_VERSION, 
#ifdef ENABLE_BIG_ENDIAN_BUILD
    'B'
#else
    'L'
#endif
      ,__DATE__, __TIME__, BUILD_USER);

#ifdef ENABLE_TEST_APP
    // Set the PORTS that should be used
    TESTAPP_Setup();
#endif
#ifdef ENABLE_RECOIL_APP
    // Set the PORTS that should be used
    RECOILAPP_Setup();
#endif

    recoil_InstallSignalHandlers();
    if (!NWDISCOVERY_Start())
    {
        DEBUG_FATAL("Unable to start the network discovery sub system ...");
    }
    else if (!NWTIME_Start())
    {
        DEBUG_FATAL("Unable to start the network time sub system ...");
    }
    else if (!NWEVENT_Start())
    {
        DEBUG_FATAL("Unable to start the network event sub system ...");
    }    
    else if (!NWSYNC_Start())
    {
        DEBUG_FATAL("Unable to start the network sync sub system ...");
    }
    else if (!NWPING_Start())
    {
        DEBUG_FATAL("Unable to start the network ping sub system ...");
    }
#ifdef ENABLE_TEST_APP
    else if (!TESTAPP_Start())
    {
        DEBUG_FATAL("Unable to start the TESTAPP sub system ...");
    }
#endif    
#ifdef ENABLE_RECOIL_APP
    else if (!RECOILAPP_Start())
    {
        DEBUG_FATAL("Unable to start the RECOILAPP sub system ...");
    }
#endif    
    else if (SYSTEM_CALL_ERROR == sem_init(&gShutdownSem, 0, 0))
    {
        DEBUG_FATAL("Unable to create the shutdown sempahore ...");
    }

    // waiting for the shutdown request
    sem_wait(&gShutdownSem);
    DEBUG_MIL ("RECOIL System shutdown complete...");    

    DEBUG_OUT;
    return 0;
}
