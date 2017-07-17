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
#define HELPER_THREAD_STACK                 8096    // 8K stack for each client thread
#define MAX_INCOMING_BYTES                  (uint32_t)MAX_SIZE_OF_TIME_REQUEST  // maximum bytes to expect from the clients on a single transfer (2 bytes extra in the structure is added for padding)
#define MAX_OUTGOING_BYTES                  (uint32_t)MAX_SIZE_OF_TIME_REPLY  // maximum bytes to be sent by the server to the clients on a single transfer
#define RECEIVER_THREAD_SLEEP               100000  // uSeconds
#define TIMEOUT_TO_CHECK_INCOMING_PACKETS   100000  // uSeconds

/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/
static UTILS_Thread_t gHelperThread;        // time helper thread
static volatile int ghReceiveSocket;        // main socket to receive incoming datagrams from network ping
#ifdef USE_EXTRA_PORT_FOR_UDP_RECEIVE
static volatile int ghSendSocket;           // main socket to send outgoing datagrams to network ping
#endif
/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

// buffer - MUST be valid, no check is made inside the function
void nwtime_HandleNodePacket (int hSocket, uint8_t* buffer, int size, struct sockaddr_in *p_c_addr, int c_size, char* clientip)
{
    TimeClientPacket_t *pkt_in = (TimeClientPacket_t*)buffer;

    if (!UTILS_VerifyPacketHeader (buffer, size, NetworkTime, MAX_INCOMING_BYTES, NULL))    // handle only the full packets
    {
        DEBUG_ERROR("Network packet header verification failed");
    }
    else if (pkt_in->type != (uint8_t)(NWTIME_REQUEST & 0xFF)) // check the time packet type - MUST match NWTIME_REQUEST !!!
    {
        DEBUG_ERROR("Time packet type(%d) does not match expected time packet type(%d)", pkt_in->type, (uint8_t)(NWTIME_REQUEST & 0xFF));
    }
    else
    {
        // send the response the client
        TimeServerPacket_t pkt_out;
        struct timespec t;

        // prepare the header
        UTILS_SetHeader(&pkt_out.header, NetworkTime, (uint16_t)(sizeof(TimeServerPacket_t) - sizeof(ProtocolHeaderV1_write)));
        clock_gettime(CLOCK_MONOTONIC, &t);

        pkt_out.type    = (uint8_t)NWTIME_REPLY;
        pkt_out.t_secs  = (uint16_t)htons(t.tv_sec & ProtocolHeaderV1_PING_NWTIME_SECS_MASK);
        pkt_out.t_nsecs = (uint32_t)htonl(t.tv_nsec & ProtocolHeaderV1_PING_NWTIME_NSECS_MASK);
        pkt_out.sequence = pkt_in->sequence;    // we must copy the request sequence to the reply

        // send the response
        DEBUG_EINFO("Time response (%x %x %x %x) to client(%s)", pkt_out.type, pkt_out.sequence, pkt_out.t_secs, pkt_out.t_nsecs, clientip);
#ifdef __DEBUG
        {
            uint8_t *p = (uint8_t *)&pkt_out;
            DEBUG_EINFO("pkt[%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x]", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]);
        }
#endif
        {
            ssize_t sent = 0;

#ifdef USE_EXTRA_PORT_FOR_UDP_RECEIVE
            struct sockaddr_in addr;

            memset(&addr, 0, sizeof(addr));
            addr.sin_family         = AF_INET;
            addr.sin_addr.s_addr    = inet_addr(clientip);
            addr.sin_port           = htons((unsigned short)(atoi(gNetworkTimeReturnPort)));

            if (SYSTEM_CALL_ERROR == (sent = sendto(ghSendSocket, (const void *)&pkt_out, sizeof(TimeServerPacket_t), 0, (struct sockaddr *) &addr, sizeof(addr))))
            {
                if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
                    DEBUG_ERROR("Unable to send may be socket is busy..");
                }
                else
                {
                    DEBUG_ERROR("recvfrom() failed(%d, %s) for incoming datagram", errno, strerror(errno));
                }
            }
            else if (sent == sizeof(TimeServerPacket_t))
            {
                DEBUG_EINFO ("Sent time response successfully to client(%s)", clientip);
            }
            else
            {
                DEBUG_WARN ("Only partial packet is sent(%d) expected(%d) to client(%s)", (uint32_t)sent, (uint32_t)sizeof(TimeServerPacket_t), clientip);
            }
        
#else
            if (SYSTEM_CALL_ERROR == (sent = sendto (hSocket, (const void *)&pkt_out, sizeof(TimeServerPacket_t), 0, (struct sockaddr *)p_c_addr, c_size)))
            {
                if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
                    DEBUG_ERROR("Unable to send may be socket is busy..");
                }
                else
                {
                    DEBUG_ERROR("recvfrom() failed(%d, %s) for outgoing datagram", errno, strerror(errno));
                }
            }
            else if (sent != sizeof(TimeServerPacket_t))
            {
                DEBUG_ERROR("Unable to send all the bytes request(%lu) sent(%ld)", sizeof(TimeServerPacket_t), sent);
            }
#endif
        }
    }

    DEBUG_OUT;
    return;
}

// Helper thread to handle network clients incoming time request and sending reply to them
void* nwtime_HelperThread (void *pArg)
{
    uint8_t *buffer_in = NULL, *buffer_out = NULL;
    int hSocket = *((int*)pArg) , RxSize = 0;
    socklen_t c_size = sizeof(struct sockaddr_in);
    struct sockaddr_in c_addr; /* client addr */
    struct hostent *hostp; /* client host info */
    char *hostaddrp; /* dotted decimal host addr string */

    DEBUG_IN;

    fcntl(hSocket, F_SETFL, O_NONBLOCK); /* configure the socket to work in non-blocking state	*/

    // allocate buffer to hold only one packet at a time - as in the SyncClientPacket_t
    if (NULL == (buffer_in = (uint8_t*)malloc(MAX_INCOMING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", MAX_INCOMING_BYTES);
        gHelperThread.run = false;
    }
    else if (NULL == (buffer_out = (uint8_t*)malloc(MAX_OUTGOING_BYTES)))
    {
        DEBUG_ERROR("Out of memory error, requested bytes(%d)", MAX_OUTGOING_BYTES);
        gHelperThread.run = false;
    }

    while (gHelperThread.run)
    {
        DEBUG_EINFO ("Starting new time helper cycle...");

        if (SYSTEM_CALL_ERROR == (RxSize = recvfrom(hSocket, buffer_in, MAX_INCOMING_BYTES, 0, (struct sockaddr *) &c_addr, &c_size)))
        {
            if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
            {
                DEBUG_EINFO("Nothing to read from the socket..");
            }
            else
            {
                DEBUG_ERROR("recvfrom() failed(%d, %s) for incoming datagram", errno, strerror(errno));
            }
        }
        else if (RxSize == 0)
        {
            DEBUG_ERROR("Client disconnected unexpectedly");
        }
        else
        {
            DiscoveryClientInfo_t info;

            if (NULL == (hostp = gethostbyaddr((const char *)&c_addr.sin_addr.s_addr, sizeof(c_addr.sin_addr.s_addr), AF_INET)))
            {
                //DEBUG_ERROR("gethostbyaddr failed(%d, %s) ", errno, strerror(errno));
            }

            if (NULL == (hostaddrp = inet_ntoa(c_addr.sin_addr)))
            {
                DEBUG_ERROR("Unable to get the sender IP dropping time request packet - error(%d, %s)", errno, strerror(errno));
            }
            else
            {
                //DEBUG_INFO("received %d bytes from client(%s, %s) data[%2x %2x %2x %2x][%2x %2x %2x %2x][%2x %2x %2x %2x]", RxSize, (hostp) ? hostp->h_name : "", hostaddrp, buffer_in[0], buffer_in[1], buffer_in[2], buffer_in[3], buffer_in[4], buffer_in[5], buffer_in[6], buffer_in[7], buffer_in[8], buffer_in[9], buffer_in[10], buffer_in[11]);

                if (NWDISCOVERY_GetClientInfoFromIP ((uint8_t *)hostaddrp, &info))
                {
                    nwtime_HandleNodePacket(hSocket, buffer_in, RxSize, &c_addr, c_size, hostaddrp);
                }
                else
                {
                    DEBUG_ERROR("Unable to find the client details from discovery - dropping request");
                }
            }                
        }

        usleep(RECEIVER_THREAD_SLEEP);
    }

    // free the buffer
    if (buffer_in)
        free(buffer_in);
    if (buffer_out)
        free(buffer_out);

    DEBUG_MIL("Network Time [helper thread] shutdown complete...");

    // terimate this thread
    DEBUG_OUT;    
    pthread_exit(NULL);
}

// Start the network time sub system
bool NWTIME_Start (void)
{
    int status;
    int option = 1;
    pthread_attr_t attr;
    bool retval = false;
    struct sockaddr_in addr;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_TIME", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // helper thread must start once it is created
    gHelperThread.run = true; 

    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)(atoi(gNetworkTimePort)));

    // Create a UDP socket
    if (SYSTEM_CALL_ERROR == (ghReceiveSocket = socket (AF_INET, SOCK_DGRAM, 0)))
    {
        DEBUG_FATAL("Socket open failed(%d, %s)", errno, strerror(errno));
    }
#ifdef USE_EXTRA_PORT_FOR_UDP_RECEIVE    
    else if (SYSTEM_CALL_ERROR == (ghSendSocket = socket (AF_INET, SOCK_DGRAM, 0)))
    {
        DEBUG_FATAL("Socket open failed(%d, %s)", errno, strerror(errno));
    }
#endif   

    // One port for all incoming connections - set SO_REUSEPORT to true
    //else if(SYSTEM_CALL_ERROR == (status = setsockopt(ghReceiveSocket, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option))))
    else if(SYSTEM_CALL_ERROR == (status = setsockopt(ghReceiveSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))))
    {
        DEBUG_FATAL("Socket config failed(%d, %s) hndl(%d) option(%d)", errno, strerror(errno), ghReceiveSocket, option);
    }
    // bind sock addr to socket
    else if (SYSTEM_CALL_ERROR == (status = bind(ghReceiveSocket, (struct sockaddr *)&addr, sizeof(addr))))
    {
        DEBUG_FATAL("Binding failed on socket(%d) error(%d, %s)", ghReceiveSocket, errno, strerror(errno));
    }  
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_init(&attr))))
    {
        DEBUG_FATAL("pthread_attr_init failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_setstacksize(&attr, HELPER_THREAD_STACK))))
    {
        DEBUG_FATAL("pthread_attr_setstacksize failed(%d, %s)", errno, strerror(errno));
    } 
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (&gHelperThread.thread, &attr, nwtime_HelperThread, (void*)&ghReceiveSocket)))    
    {
        DEBUG_FATAL("Helper thread creation failed(%d, %s)", errno, strerror(errno));
    }  
    else
    {
        DEBUG_MIL("Network Time started successfully on port(%s)...", gNetworkTimePort);
        retval = true;
    }

    DEBUG_OUT;
    return (retval);
}

// Shut the network time sub system
bool NWTIME_Stop (void)
{
    void *pRetval = NULL;
    int status;
    bool retval = false;  
    
    DEBUG_IN;
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

    DEBUG_OUT;
    return retval;
}
