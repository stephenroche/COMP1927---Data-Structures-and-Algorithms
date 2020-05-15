/**
 * Runs a player's game turn ...
 *
 * Can produce either a Hunter player or a Dracula player
 * depending on the setting of the I_AM_DRACULA #define
 *
 * This is a dummy version of the real player.c used when
 * you submit your AIs. It is provided so that you can
 * test whether your code is likely to compile ...
 *
 * Note that it used to drive both Hunter and Dracula AIs.
 * It first creates an appropriate view, and then invokes the
 * relevant decide-my-move function, which should use the 
 * registerBestPlay() function to send the move back.
 *
 * The real player.c applies a timeout, so that it will halt
 * decide-my-move after a fixed amount of time, if it doesn't
 * finish first. The last move registered will be the one
 * used by the game engine. This version of player.c won't stop
 * your decide-my-move function if it goes into an infinite
 * loop. Sort that out before you submit.
 *
 * Based on the program by David Collien, written in 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
#include "DracView.h"
#include "dracula.h"
#else
#include "HunterView.h"
#include "hunter.h"
#endif

// moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

int main(int argc, char *argv[])
{
#ifdef I_AM_DRACULA
   DracView gameState;
   //char *plays = "GLO.... STO.... HVE.... MSZ....";
   //char *plays = "GLO.... STO.... HVE.... MSZ.... DLE.V.. GLO.... STO.... HSZ.... MCO....";
   //char *plays = "GLO.... STO.... HVE.... MSZ.... DLE.V.. GLEVD.. STO.... HSZ.... MCO.... DLOT... GLE.... STO.... HSZ.... MLOTD..";
   char *plays = "GTO.... SSW.... HMU.... MKL.... DNP.V.. GMR.... SIR.... HVE.... MKL.... DD1T... GMS.... SAO.... HFL.... MKL.... DBIT... GCG.... SMS.... HRO.... MGA.... DAS.... GCG.... SFL.... HBIT... MKL.... DIO.... GMS.... SGO.... HNPTV.. MGA.... DATT... GAL.... SVE.... HRO.... MKL.... DVAT... GSN.... SMU.... HNP.... MGA.... DD2T... GLS.... SNU.... HBI.... MKL.... DHIT...";
   PlayerMessage msgs[20] = { "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" };
   gameState = newDracView(plays,msgs);
   decideDraculaMove(gameState);
   disposeDracView(gameState);
#else
   HunterView gameState;
   //char *plays = "GED.... SED.... HED.... MED.... DC?.V.. GED.... SED.... HED.... MED.... DC?.... GLOV... SED.... HED.... MED.... DC?....";
   //char *plays = "GZA.... SED.... HZU.... MLO.... DLO.... GZA.... SED.... HZU.... MLO.... DC?.... GZA.... SED.... HZU.... MLO.... DS?....";
   char *plays = "GTO.... SSW.... HMU.... MKL.... DNP.V.. GMR.... SIR.... HVE.... MKL.... DD1T... GMS.... SAO.... HFL.... MKL.... DC?T... GCG.... SMS.... HRO.... MGA.... DS?.... GCG.... SFL.... HBIT... MKL.... DS?.... GMS.... SGO.... HNPTV.. MGA.... DC?T... GAL.... SVE.... HRO.... MKL.... DC?T... GSN.... SMU.... HNP.... MGA.... DD2T... GLS.... SNU.... HBI.... MKL.... DHIT...";
   PlayerMessage msgs[20] = { "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" };
   gameState = newHunterView(plays,msgs);
   decideHunterMove(gameState);
   disposeHunterView(gameState);
#endif 
   printf("Moves to %s\n", idToName(abbrevToID(latestPlay)));
   printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
   return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay (char *play, PlayerMessage message) {
   strncpy(latestPlay, play, MOVE_SIZE-1);
   latestPlay[MOVE_SIZE-1] = '\0';
     
   strncpy(latestMessage, message, MESSAGE_SIZE);
   latestMessage[MESSAGE_SIZE-1] = '\0';
}


