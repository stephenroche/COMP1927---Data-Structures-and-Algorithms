// DracView.c ... DracView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
#include "Map.h"
#include <stdio.h>

#define PLAY_STRING_SIZE 8
#define DOUBLE_BACK_OFFSET 102
     
static EncounterID abbrevToEncounter (char encounter);
static void addEncounter (DracView g, EncounterID id, LocationID location);
static void removeEncounter (DracView g, EncounterID id, LocationID location);
static PlayerID abbrevToPlayer(char * player);

typedef struct _encounter {
    int trap; // Range from 0 to 3
    int vampire; // Value 0 or 1
    
} encounter;

struct dracView {
    GameView game;
    encounter encounters[NUM_MAP_LOCATIONS]; // Access using LocationID as index
    int lastEncountered; // Stores index in Drac's trail of when he was last encountered, contains -1 if not encountered yet the encounter has left his trail
};

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    DracView dracView = malloc(sizeof(struct dracView));
    assert (dracView != NULL); // Check malloc was successful 
    dracView->game = newGameView(pastPlays, messages);
    
    // Initialise encounters to 0
    int i;
    for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dracView->encounters[i].trap = 0;
        dracView->encounters[i].vampire = 0;
    }
    
    // Process Encounters
    dracView->lastEncountered = -1;
    LocationID realTrail[TRAIL_SIZE];
    getRealHistory(dracView->game, PLAYER_DRACULA, realTrail);
    
    int length = strlen(pastPlays) + 1; // + 1 to account for lack of spacebar at the end
    int startTrail;
    
    startTrail = length - NUM_PLAYERS * PLAY_STRING_SIZE * TRAIL_SIZE; // Start at Dracula trail[5]
    if (startTrail < 0) {
        startTrail = (NUM_PLAYERS - 1) * PLAY_STRING_SIZE; // Dracula's first turn
    }
    
    for (i = startTrail; i < length; i += PLAY_STRING_SIZE) {
        PlayerID curr = abbrevToPlayer(&pastPlays[i]);
        LocationID currLoc;
        int start = 0;
        
        if (curr == PLAYER_DRACULA) {
            currLoc = realTrail[(length - i) / (NUM_PLAYERS * PLAY_STRING_SIZE) - 1]; // Turns any D1-5s, etc. into location IDs
            for (start = 0; start < 2; start++) {
                EncounterID id = abbrevToEncounter(pastPlays[i + 3 + start]);
                if (id >= 0) {
                    addEncounter(dracView, id, currLoc);
                    //printf("Adding encounter to %s\n", idToName(currLoc));
                }
            }
                
        } else { // Hunter
            currLoc =  abbrevToID(&pastPlays[i + 1]);
            for (start = 0; start < 4; start++) {
                EncounterID id = abbrevToEncounter(pastPlays[i + 3 + start]);
                if (id >= 0 && id <= 1) {
                    removeEncounter(dracView, id, currLoc);
                    //printf("Removing encounter from %s\n", idToName(currLoc));
                } else if (id == ENCOUNTER_DRACULA) {
                    // Set lastEncountered to index in Dracula's trail when he was there
                    dracView->lastEncountered = (length - i) / (NUM_PLAYERS * PLAY_STRING_SIZE);
                }
            }
        }
    }
    
    return dracView;
}

// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    disposeGameView(toBeDeleted->game);
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView)
{
    return getRound(currentView->game);
}

// Get the current score
int giveMeTheScore(DracView currentView)
{
    return getScore(currentView->game);
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player)
{
    return getHealth(currentView->game, player);
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player)
{
    return getRealLocation(currentView->game, player);
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end)
{
    LocationID trail[TRAIL_SIZE];
    giveMeTheTrail(currentView, player, trail);
   
    *start = trail[1];
    *end = trail[0];
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps)
{
    if (!validPlace(where)) {
        *numTraps = 0;  
        *numVamps = 0;
    } else {
        *numTraps = currentView->encounters[where].trap;
        *numVamps = currentView->encounters[where].vampire;
    }
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    getHistory (currentView->game, player, trail);
}

// Fills the trail array with the realLocation ids of the last 6 turns
void giveMeTheRealTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    return getRealHistory(currentView->game, player, trail);
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
    // Need to remove locations already in Drac's trail from array before returning
    
    LocationID *legalMoves = connectedLocations(currentView->game, numLocations, whereIs(currentView, PLAYER_DRACULA), PLAYER_DRACULA, giveMeTheRound(currentView), road, 0, sea);
    
    int hideUsed = 0;
    int doubleBackUsed = 0;
    int i, j, k;
    LocationID trail[TRAIL_SIZE];
    giveMeTheTrail(currentView, PLAYER_DRACULA, trail);

    for (i = 0; i < TRAIL_SIZE - 1; i++) { // trail[5] is ignored as it 'falls off' the trail before Drac makes his move
        if (trail[i] == HIDE) {
            hideUsed = 1;
        } else if (trail[i] >= DOUBLE_BACK_1 && trail[i] <= DOUBLE_BACK_5) {
            doubleBackUsed = 1;
        }
    }
    
    for (i = 0; i < TRAIL_SIZE - 1; i++) {
        j = 0;
        while (j < *numLocations) {
            if (trail[i] == legalMoves[j] && doubleBackUsed && (hideUsed || legalMoves[j] != getRealLocation(currentView->game, PLAYER_DRACULA) || isSea(legalMoves[j]))) {
                
                // Remove location from legalMoves[]
                for (k = j; k < *numLocations - 1; k++) {
                    legalMoves[k] = legalMoves[k + 1];
                }
                (*numLocations)--;
            } else {
                j++;
            }
        }
    }
    
    return legalMoves;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    if (player == PLAYER_DRACULA) {
        return whereCanIgo(currentView, numLocations, road, sea);
    } else {
        return connectedLocations(currentView->game, numLocations, whereIs(currentView, player), player, giveMeTheRound(currentView) + 1, road, rail, sea);
    }
}

/* abbrevToEncounter 
 * takes single character and converts to encounter ID
 * returns -1 if invalid character input
 */
static EncounterID abbrevToEncounter (char encounter)
{
    EncounterID enc = -1;
    
    if (encounter == 'T') {
       enc = ENCOUNTER_TRAP; 
    } else if (encounter == 'V') {
       enc = ENCOUNTER_VAMPIRE; 
    } else if (encounter == 'D') {
       enc = ENCOUNTER_DRACULA; 
    } 
    
    return enc;
}

static void addEncounter (DracView g, EncounterID id, LocationID location) {
     
     if (id == ENCOUNTER_TRAP) {
        g->encounters[location].trap++;
     } else {
        g->encounters[location].vampire++; 
     }
}

static void removeEncounter (DracView g, EncounterID id, LocationID location) {
    
    if (id == ENCOUNTER_TRAP) {
        g->encounters[location].trap--;
        
    } else if (id == ENCOUNTER_VAMPIRE) {
        g->encounters[location].vampire--;
    }
}

/* abbrevToPlayer
 * converts string to player ID
 * assumes string length is 1. Will return NULL if invalid player
 */
static PlayerID abbrevToPlayer(char * player) 
{
    PlayerID id = NULL;
    switch (player[0]) {
        case 'G':
            id = PLAYER_LORD_GODALMING;
        break;
        
        case 'S':
            id = PLAYER_DR_SEWARD;
        break;
            
        case 'H':
            id = PLAYER_VAN_HELSING;
        break;
            
        case 'M':
            id = PLAYER_MINA_HARKER;
        break;
            
        case 'D':
            id = PLAYER_DRACULA;
        break;
    }
    
    return id;
}

int dracDistanceBetween (DracView currentView, LocationID src, LocationID dest, int road, int rail, int sea) {
    return distanceBetween (currentView->game, src, dest, road, rail, sea);
}

// Returns trail index in Drac's trail of when he was most recently encountered
int lastFoundMe(DracView currentView) {
    return currentView->lastEncountered;
}
