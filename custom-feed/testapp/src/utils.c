//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- utils.c
//  description :- Recoil Network service - utilts functions
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

#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

//#define __DEBUG
//#define __DEBUG_EXT_INFO
//#define __DEBUG_IN
//#define __DEBUG_OUT
#include "recoilnetwork.h"

/*
* ------------------------------------ User macros ---------------------------------------------------------------
*/


/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

// --------------------------------- HEADER RELATED ---------------------------------

extern volatile bool enable;

void UTILS_GetHeaderDefaults (ProtocolHeaderV1_write *hdr)
{
    hdr->pid = (uint8_t)(RECOIL_PRODUCT_ID & 0xFF);
    hdr->version = (uint8_t)(RECOIL_PROTOCOL_VERSION & 0x3F);
    return;
}

//set the dynamic header fields and return the value in network byte order
void UTILS_SetHeader (ProtocolHeaderV1_write *hdr, uint8_t id, uint16_t len)
{
    uint32_t *ptr = (uint32_t*)hdr;
    char* p = (char *)hdr;

    DEBUG_IN;

    UTILS_GetHeaderDefaults(hdr);
    hdr->id = (id & ProtocolHeaderV1_ID_MASK);
    hdr->length = (len & ProtocolHeaderV1_LENGTH_MASK);

    if (enable)
    {DEBUG_INFO("b4 %02X %02X %02X %02X", p[0], p[1], p[2], p[3]);}

    // change the byte ordering to network byte order
    *ptr = htonl(*(uint32_t*)hdr);

    if (enable)
    {DEBUG_INFO("ar %02X %02X %02X %02X", p[0], p[1], p[2], p[3]);}

    DEBUG_OUT;

    return;
}

bool UTILS_VerifyPacketHeader (uint8_t *buffer, uint32_t size, ProtocolId exp_id, uint32_t exp_size, uint32_t *payload_size)
{
    bool retval = false;
    ProtocolHeaderV1_read exp_header = {0};

    uint32_t endian_swap = ntohl(*(uint32_t*)buffer);    
    ProtocolHeaderV1_read *in_header = (ProtocolHeaderV1_read*)&endian_swap;

    DEBUG_IN;

    UTILS_GetHeaderDefaults((ProtocolHeaderV1_write*)&exp_header);

    DEBUG_EINFO("Incoming pid(%d) version(%d) id(%d) len(%d)", in_header->pid, in_header->version, in_header->id, in_header->length);
    DEBUG("Incoming bytes data[%2x %2x %2x %2x]", buffer[0], buffer[1], buffer[2], buffer[3]);
    DEBUG("params[%p %d %d %d %p]", buffer, size, exp_id, exp_size, payload_size);

    // TODO handle partial packets - buffer locally and handle fragmented packets
    if ((exp_size != 0) && (size != exp_size))    // currently handles only full packets
    {
        DEBUG_ERROR("System only handles full packets, buffer size(%d) does not match expected packet size(%d)", size, exp_size);
    }
    else if (exp_header.pid != in_header->pid)
    {
        DEBUG_ERROR("Packet header mismatch ExPID(%d) RxPID(%d)", exp_header.pid, in_header->pid);
    }
    else if (exp_header.version != in_header->version)
    {
        DEBUG_ERROR("Packet header mismatch ExVersion(%d) RxVersion(%d)", exp_header.version, in_header->version);
    }
    else if ((uint8_t)(exp_id & ProtocolHeaderV1_ID_MASK) != in_header->id)
    {
        DEBUG_ERROR("Packet header mismatch ExPacketId(%d) RxPacketId(%d)", (uint8_t)(exp_id & ProtocolHeaderV1_ID_MASK), in_header->id);
    }
    // for fixed size packets, size will be valid so check with the coded length in the header
    else if ((size != 0) && (in_header->length != (size - sizeof(ProtocolHeaderV1_read))))
    {
        DEBUG_ERROR("Packet length(%d) does not match the coded packet length(%d)", (uint32_t)(size - sizeof(ProtocolHeaderV1_read)), in_header->length);
    }
    else
    {
        if (payload_size) // only if user interested - this pointer is valid
        {
            *payload_size = in_header->length;
        }            
        retval = true;
    }

    if (!retval)
    {
        DEBUG_INFO("Incoming pid(%d) version(%d) id(%d) len(%d)", in_header->pid, in_header->version, in_header->id, in_header->length);
        DEBUG_INFO("Incoming bytes data[%2x %2x %2x %2x]", buffer[0], buffer[1], buffer[2], buffer[3]);
    }

    DEBUG_OUT;
    return retval;
}

// --------------------------------- THREADS ---------------------------------

int32_t UTILS_CreateThread (pthread_t *pThread, uint32_t threadsize, UTILS_ThreadFunction func, void* pData)
{
    pthread_attr_t attr;
    int32_t status;

    if (SYSTEM_CALL_ERROR == (status = (pthread_attr_init(&attr))))
    {
        DEBUG_ERROR("attr_init failed(%d, %s)", errno, strerror(errno));
    }
    else if (SYSTEM_CALL_ERROR == (status = (pthread_attr_setstacksize(&attr, threadsize))))
    {
        DEBUG_ERROR("setstacksize failed(%d, %s)", errno, strerror(errno));
    }    
    // create the helper thread
    else if (SYSTEM_CALL_ERROR == (status = pthread_create (pThread, &attr, func, pData)))    
    {
        DEBUG_ERROR("thread creation failed(%d, %s)", errno, strerror(errno));
    }
    else
    {
        status = 0;
        DEBUG_EINFO("thread created !!");
    }

    return status;
}

// --------------------------------- TCP RELATED ---------------------------------

// entry point for the network discovery sub system
int32_t UTILS_TCP_Create (char* port, bool shared, uint32_t maxConns, int32_t *hSocket)
{
    int32_t status;
    struct addrinfo hints, *server = NULL;
    int option = 1;

    DEBUG_IN;

    // configure system logs
    openlog ("RECOIL_TCP", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // Configure the socket
    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;              // set ipv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    // Get socket params and server IP details
    if (0 != (status = getaddrinfo(NULL, port, &hints, &server)))
    {
        DEBUG_ERROR("getaddrinfo failed(%s:%d)", gai_strerror(status), status);
    }
    // Create a TCP socket
    else if (SYSTEM_CALL_ERROR == (status = (*hSocket = socket (server->ai_family, server->ai_socktype, server->ai_protocol))))
    {
        DEBUG_ERROR("Socket open failed(%d, %d, %d)", server->ai_family, server->ai_socktype, server->ai_protocol);
    }
    else
    {
        if (shared) // if the port is shared - enable port reuse
        {
            // One port for all incoming connections - set SO_REUSEPORT to true
            //status = setsockopt(hSocket, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));            
            status = setsockopt(*hSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        }

        if (SYSTEM_CALL_ERROR == status)
        {
            DEBUG_ERROR("Socket config failed(%d, %s) hndl(%d) option(%d)", errno, strerror(errno), *hSocket, option);
        }
        else if (SYSTEM_CALL_ERROR == (status = bind(*hSocket, server->ai_addr, server->ai_addrlen)))
        {
            DEBUG_ERROR("Binding failed on socket(%d) error(%d, %s)", *hSocket, errno, strerror(errno));
        }
        else if (SYSTEM_CALL_ERROR == (status = (listen (*hSocket, maxConns))))
        {
            DEBUG_ERROR("Listening on socket failed(%d, %s)", errno, strerror(errno));
        }
        else
        {
            DEBUG_EINFO("TCP connection created successfully on port(%s)...", port);
        }
    }

    DEBUG_OUT;
    return (status);
}

bool UTILS_TCP_ReadData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t expected_size, uint32_t *readsize, bool *connected)
{
    bool retval = false;
    int32_t RxSize;

    *connected  = true;
    *readsize   = 0;

    // read the incoming data from tcp socket
    if (SYSTEM_CALL_ERROR == (RxSize = recv(handle, buffer, size, 0)))
    {
        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            DEBUG_EINFO("Nothing to read from the socket...");
            retval = true;
        }
        else
        {
            DEBUG_ERROR("recv() failed(%d, %s) for handle(%d)", errno, strerror(errno), handle);
        }
    }
    else if (RxSize == 0)
    {
        DEBUG_ERROR("Socket (%d) is disconnected", handle);
        *connected = false;
    }
    else
    {
        *readsize = (uint32_t)RxSize;

        if (0 == expected_size) // no comparision is needed
        {
            retval = true;
        }        
        else if (expected_size == (uint32_t)RxSize)
        {
            retval = true;
        }
        else
        {
            DEBUG_EINFO("received unexpected(%d) bytes from handle(%d) - expected(%d)", RxSize, handle, expected_size);
        }
    }

    return retval;
}

// This method causes the receive operation to return data from the beginning of the receive queue without removing that data from the queue. 
// Thus, a subsequent receive call will return the same data.
// This method is useful if the user dont know how much data to read and a header carries the length of the payload.
// So snoop the header data, find the size and read the entire payload
bool UTILS_TCP_SnoopData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t expected_size, uint32_t *readsize, bool *connected)
{
    bool retval = false;
    int32_t RxSize;

    *connected  = true;
    *readsize   = 0;

    // take a snapshot of the incoming data from the tcp socket
    if (SYSTEM_CALL_ERROR == (RxSize = recv(handle, buffer, size, MSG_PEEK)))
    {
        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            DEBUG_EINFO("Nothing to read from the socket...");
            retval = true;
        }
        else
        {
            DEBUG_ERROR("recv() failed(%d, %s) for handle(%d)", errno, strerror(errno), handle);
        }
    }
    else if (RxSize == 0)
    {
        DEBUG_ERROR("Socket (%d) is disconnected", handle);
        *connected = false;
    }
    else
    {
        *readsize = (uint32_t)RxSize;

        if (0 == expected_size) // no comparision is needed
        {
            retval = true;
        }        
        else if (expected_size == (uint32_t)RxSize)
        {
            retval = true;
        }
        else
        {
            DEBUG_EINFO("not enough bytes (%d) in the socket to read now - handle(%d) expected(%d)", RxSize, handle, expected_size);
        }
    }

    return retval;
}

// This method try to read the no of bytes from the tcp socket, if there is not sufficient data it will return error
bool UTILS_TCP_ReadNBytes (int32_t handle, uint8_t* buffer, uint32_t size, bool *connected)
{
    bool retval = false;
    int32_t RxSize;
    int bytes_in_socket;

    *connected  = true;

    // check how many bytes in the socket - if we got enough bytes read them otherwise return error
    if (SYSTEM_CALL_ERROR == ioctl(handle, FIONREAD, &bytes_in_socket))
    {
        DEBUG_ERROR("Socket FIONREAD failed(%d, %s) for handle(%d)", errno, strerror(errno), handle);
    }
    else if (bytes_in_socket < size)
    {
        DEBUG_INFO("Not enough bytes to read from the socket !! request(%d) available(%d) handle(%d)", size, bytes_in_socket, handle);
    }
    // read the incoming data from the tcp socket
    else if (SYSTEM_CALL_ERROR == (RxSize = recv(handle, buffer, size, 0)))
    {
        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            DEBUG_ERROR("FIONREAD returned a wrong value... We got nothing to read from the socket...");
        }
        else
        {
            DEBUG_ERROR("recv() failed(%d, %s) for handle(%d)", errno, strerror(errno), handle);
        }
    }
    else if (RxSize == 0)
    {
        DEBUG_ERROR("Socket (%d) is disconnected", handle);
        *connected = false;
    }
    else if (RxSize != size)
    {
        DEBUG_EINFO("received unexpected(%d) bytes from handle(%d) - expected(%d)", RxSize, handle, expected_size);
    }
    else
    {
        retval = true;
    }

    DEBUG("bytes_in_socket(%d)", bytes_in_socket);    

    return retval;
}

bool UTILS_TCP_SendData (int32_t handle, uint8_t* buffer, uint32_t size, uint32_t *sent, bool *connected)
{
    bool retval = false;
    bool conn = true;
    uint32_t sent_tmp;

    // read the incoming data from tcp socket
    if (SYSTEM_CALL_ERROR == (sent_tmp = send(handle, buffer, size, 0)))
    {
        sent_tmp = 0;
        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            DEBUG_EINFO("Socket is busy, this would block the socket...Try later");
            retval = true;
        }
        else if (EPIPE == errno)
        {
            DEBUG_ERROR("Socket (%d) is disconnected", handle);
            conn = false;
        }
        else
        {
            DEBUG_ERROR("send() failed(%d, %s) for handle(%d)", errno, strerror(errno), handle);
        }
    }
    else
    {
        DEBUG_EINFO("Socket sent %d bytes to (%d)", sent_tmp, handle);
        retval = true;
    }

    if (connected) {
        *connected = conn;
    }

    if (sent) {
        *sent = sent_tmp;
    }        

    return retval;    
}

bool UTILS_TCP_Destroy (int32_t hSocket)
{
    if (SOCKET_INVALID_HANDLE != hSocket)
    {
        if (SYSTEM_CALL_ERROR != close(hSocket))
        {
            return true;
        }
    }
    return false;
}
