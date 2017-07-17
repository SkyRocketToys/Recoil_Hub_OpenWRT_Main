// ----------------------------------------------------------------------------
//  Copyright (c) 2017, Hotgen Ltd (www.hotgen.com)
//  filename    :- recoil.c
//  description :- Recoil Network service
//  author      :- Rajesh Gunasekaran   (rajg@hotgen.com)
//  modified    :- RQ (so that you can restart the GAME SERVER)
// ----------------------------------------------------------------------------

/*
* ------------------------------------ Header files  -----------------------------------------------------------
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "recoilnetwork.h"
#include "power.h"

#ifndef BUILD_USER
#define BUILD_USER "UnknownUser"
#endif
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
char *gNetworkUpgradePort = 0;                         // TCP
char *gNetworkPingReturnPort = 0;                      // UDP
char *gNetworkTimeReturnPort = 0;                      // UDP

// ------------------------------------ Functions -----------------------------

// ----------------------------------------------------------------------------
// This is called from the power helper thread
void RECOIL_TermSignalHandler(int stat)
{
	DEBUG_IN;
	DEBUG_MIL("Received signal(%d) to shutdown...", stat);

//	RECOILNetwork_Stop(); // CPM do this in the main thread, cos this thread needs to do stuff!
	sem_post(&gShutdownSem);    // all the sub systems are down, signal main thread to terminate now

	DEBUG_OUT;
}

// ----------------------------------------------------------------------------
// Shutdown the network gracefully
// (called from the main thread - shuts down various helper threads)
// (triggered by the user pressing and releasing the power button in the power thread)
void RECOILNetwork_Stop(void)
{
	DEBUG_IN;
	DEBUG_MIL("Request to shut down [recoil network] arrived...");

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
    NWUPGRADE_Stop();     // network discovery
	NWDISCOVERY_Stop();     // network discovery
	POWER_SendShutdownCompleteToTritan(); // Tell the power manager on the Tritan chip it is safe to cut the CPU now
	POWER_Stop(); // Wait for the power manager to have told the Tritan.
	
	DEBUG_OUT;
}

// ----------------------------------------------------------------------------
void RECOILNetwork_Start(void)
{
	if (!NWDISCOVERY_Start())
	{
		DEBUG_FATAL("Unable to start the network discovery sub system ...");
	}
    else if (!NWUPGRADE_Start())
    {
        DEBUG_FATAL("Unable to start the network upgrade sub system ...");
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
	if (POWER_Start()) // Tell the Tritan chip that I have booted
	{
		POWER_SetStatus(PWR_CMD_SOLID);
	}
}

// ----------------------------------------------------------------------------
void recoil_SocketErrorHandler (int stat)
{
    DEBUG_IN;
    DEBUG_MIL("Received signal(%d) for a socket failure... do nothing - handled in the send/read call in system ...", stat);
    DEBUG_OUT;
}

// ----------------------------------------------------------------------------
void recoil_InstallSignalHandlers (void)
{
    DEBUG_IN;

    // install signal handler
    if (SIG_ERR == signal(SIGINT, RECOIL_TermSignalHandler)) // Ctrl+C
    {
        DEBUG_ERROR("Installing signal handler failed");
    }
    else if (SIG_ERR == signal(SIGTERM, RECOIL_TermSignalHandler)) // Quit    
    {
        DEBUG_ERROR("Installing signal handler failed");
    }
    else if (SIG_ERR == signal(SIGHUP, RECOIL_TermSignalHandler)) // Restart   
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

// ----------------------------------------------------------------------------
bool recoil_WaitUntilWifiComesUp (char *ifname, int timeout)
{
    int fd, status;
    struct ifreq ifr;
    bool retval = false;
    time_t start;

    DEBUG_IN;
    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, ifname);
    start = time(NULL);

    DEBUG_MIL ("Waiting for the System Wireless Interface(%s) to come up...timeout(%d) seconds", ifname, timeout);
    if (SYSTEM_CALL_ERROR != (fd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        while (true)
        {
            if (SYSTEM_CALL_ERROR != (status = ioctl(fd, SIOCGIFADDR, &ifr)))
            {
                DEBUG_MIL ("System Wireless Interface(%s) is up and running on ip(%s)...", ifname, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)); 
                retval = true;
                break;
            }

            if (time(NULL) > (start + timeout))
            {
                DEBUG_MIL ("System Wireless Interface(%s) is still down after %d seconds...", ifname, timeout);
                break;
            }

            usleep(250*1000); // poll for every 250 milliseconds
        }
        close(fd);        
    }
    else
    {
        DEBUG_FATAL ("Unable to check whether wifi is up & running..."); 
    }

    return (retval);
}

// ----------------------------------------------------------------------------
bool RECOIL_SendError (bool tcp, int handle, uint8_t id, NetworkErrors err, struct sockaddr *p_c_addr, int c_size)
{
    ErrorPacket_t pkt;

    DEBUG_IN;
    DEBUG_ERROR("SENDING NETWORK ERROR TO CLIENT err(%d)", err);

    // send the response
    pkt.id = id;
    pkt.error = err;    
    UTILS_SetHeader(&pkt.header, BS_Error, (uint16_t)ERROR_PACKET_SIZE_NO_HEADER);
    DEBUG_EINFO("Error response (%x %x %x) to client(%d)", pkt.type, pkt.id, pkt.error, node);
#ifdef __DEBUG
    {
        uint8_t *p = (uint8_t *)&pkt;
        DEBUG_INFO("pkt[%2x %2x %2x %2x][%2x %2x]", p[0], p[1], p[2], p[3], p[4], p[5]);
    }
#endif
    if (tcp)
    {
	    (void) send (handle, (const void *)&pkt, (sizeof(ProtocolHeaderV1_write) + ERROR_PACKET_SIZE_NO_HEADER), 0);
    }
    else
    {
         (void)sendto (handle, (const void *)&pkt, (sizeof(ProtocolHeaderV1_write) + ERROR_PACKET_SIZE_NO_HEADER), 0, p_c_addr, c_size);
    }

    // dont free the socket - it is clients responsibility to close the connection
    //if (SYSTEM_CALL_ERROR == close(handle)) 
    {
    //    DEBUG_ERROR("Socket(%d) close failed(%d, %s)", handle, errno, strerror(errno));
    }

    DEBUG_OUT;
    return true;
}

// ----------------------------------------------------------------------------
// Do we want the system to turn the power off?
// Only do this when the tritan asks us to do so, not when we our process is killed

bool gWantPowerOff = false;

void RECOIL_WantPowerOff(void)
{
	gWantPowerOff = true;
}
				
// ----------------------------------------------------------------------------
// Main entry point to this process
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

#if 000 // CPM - is this required?
#ifdef ENABLE_BIG_ENDIAN_BUILD
    if (!recoil_WaitUntilWifiComesUp (WIRELESS_INTERFACE_NAME, APP_WAIT_UNTIL_WIFI_IS_UP))
    {
        // on failure or timeout - wait for extra 60 seconds
        sleep (APP_WAIT_UNTIL_WIFI_IS_UP);
    }
#endif
#endif

#ifdef ENABLE_TEST_APP
    // Set the PORTS that should be used
    TESTAPP_Setup();
#endif
#ifdef ENABLE_RECOIL_APP
    // Set the PORTS that should be used
    RECOILAPP_Setup();
#endif

	gWantPowerOff = false;
    recoil_InstallSignalHandlers();
	RECOILNetwork_Start();
    if (SYSTEM_CALL_ERROR == sem_init(&gShutdownSem, 0, 0))
    {
        DEBUG_FATAL("Unable to create the shutdown sempahore ...");
    }

    // waiting for the shutdown request
    sem_wait(&gShutdownSem);
    DEBUG_MIL ("RECOIL System main thread shutting down other threads...");
	RECOILNetwork_Stop();
    DEBUG_MIL ("RECOIL System shutdown complete...");    
    if (gWantPowerOff)
    {
		DEBUG_MIL ("RECOIL System halting due to poweroff...");    
		system("halt");
	}

    DEBUG_OUT;
    return 0;
}
