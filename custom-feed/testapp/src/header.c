#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "recoilnetwork.h"


//----------------------------------------------------------------------------
// HEADER:
//
// Description:
// Displays DEBUGGING information about an erroneous message or packet received.
// Whenever an error is caused by an incoming packet, some HEADER information will
// be displayed in HEXIDECIMAL FORMAT. This will convert that HEADER information
// into more meaningful information.
//
// Created: ??/??/??
// Authors: Raj
//
// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
//----------------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    uint32_t input;

    //printf ("\nargc=%d", argc);
    //printf ("\narg[0]=%s", argv[0]);
    //printf ("\narg[1]=%s", argv[1]);
    
    if ( argc == 2) /* argc should be 1 for correct execution */
    {
        // print header input
        input = (uint32_t)strtol(argv[1], NULL, 16);
        printf( "\ninput header (32bits): 0x%X\n", input);

        ProtocolHeaderV1_read *str = (ProtocolHeaderV1_read*)&input;

        printf ("\n Product ID      : hex(%3X) dec(%3d)", str->pid, str->pid);
        printf ("\n Product Version : hex(%3X) dec(%3d)", str->version, str->version);
        printf ("\n Protocol ID     : hex(%3X) dec(%3d)", str->id, str->id);
        printf ("\n Payload length  : hex(%3X) dec(%3d)", str->length, str->length);

        if (str->pid == 1)
            printf ("\n\n Product Name    : RECOIL");
        else
            printf ("\n\n Product Name    : Unknown !!!");

        printf ("\n Protocol Name   : ");

        switch (str->id)
        {
            case 0: printf ("NetworkDiscovery\n"); break;
            case 1: printf ("NetworkTime\n"); break;
            case 2: printf ("NetworkPing\n"); break;
            case 3: printf ("BS_Password\n"); break;
            case 4: printf ("BS_SoftwareUpdate\n"); break;
            case 5: printf ("BS_Error\n"); break;
            case 6: printf ("Gen_Reserved06\n"); break;
            case 7: printf ("Gen_Reserved07\n"); break;
            case 8: printf ("Gen_Reserved08\n"); break;
            case 9: printf ("Gen_Reserved09\n"); break;
            case 10: printf ("Gen_Reserved10\n"); break;

            case 11: printf ("ServerDataSync\n"); break;
            case 12: printf ("ServerAudio\n"); break;
            case 13: printf ("ServerGameSetup\n"); break;
            case 14: printf ("ServerBroadcast\n"); break;
            case 15: printf ("ServerTeamcast\n"); break;
            case 16: printf ("ServerGameMap\n"); break;
            case 17: printf ("ServerReserved17\n"); break;
            case 18: printf ("ServerReserved18\n"); break;
            case 19: printf ("ServerReserved19\n"); break;
            case 20: printf ("ServerReserved20\n"); break;   

            case 21: printf ("ClientDataSync\n"); break;
            case 22: printf ("ClientAudio\n"); break;
            case 23: printf ("ClientReserved23\n"); break;
            case 24: printf ("ClientReserved24\n"); break;
            case 25: printf ("ClientReserved25\n"); break;            
	    case 26: printf ("GameData\n"); break;

            default: printf ("\nUnused position :- %d", str->id); break;
        }            
    }
    else 
    {
        printf ("Expected format :- ./header 808101  or  ./header dead0010");
    }

    printf ("\n");    
}
