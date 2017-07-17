//
//  Copyright (c) 2016, Hotgen Ltd (www.hotgen.com)
//  filename    :- tcp.c
//  description :- Recoil Network TCP utils
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

/*
* ------------------------------------ User data types -----------------------------------------------------------
*/

/*
* ------------------------------------ Global Variables------------------------------------------------------------
*/

/*
* ------------------------------------ Functions ------------------------------------------------------------------
*/

