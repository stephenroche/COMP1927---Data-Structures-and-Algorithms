// HunterView.c ... HunterView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
// #include "Map.h" ... if you decide to use the Map ADT
// #include <stdio.h>   for debugging only
     
struct hunterView {
    GameView game;
    int dracIsland[NUM_ISLANDS]; // Contains 1 if Dracula could possibly be in that island group, otherwise 0
};
     

// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[])
{
    HunterView hunterView = malloc(sizeof(struct hunterView));
    assert (hunterView != NULL); // Check malloc was successful 
    hunterView->game = newGameView (pastPlays, messages);
    
    // Initialise dracIsland array; Drac could be anywhere
    int i;
    for (i = EUROPE; i < NUM_ISLANDS; i++) {
        hunterView->dracIsland[i] = 1;
    }
    
    // ***I think I've eliminated the need for messages***
    // Copy dracIsland info from previous hunter's message
    /*int prevIndex = 5 * getRound(hunterView->game) + getCurrentPlayer(hunterView->game) - 1 - (getCurrentPlayer(hunterView->game) == PLAYER_LORD_GODALMING);
    if (prevIndex >= 0) {
        //printf("prevMessage = '%s'\n", messages[prevIndex]);
        for (i = EUROPE; messages[prevIndex][i] != '\0' && i < NUM_ISLANDS; i++) {
            if (messages[prevIndex][i] == '0') {
                hunterView->dracIsland[i] = 0;
            }
        }
    }*/
    
    // Update dracIsland array with any info gained since previous hunter's turn
    int movesAgoLastSeen;
    LocationID dracLastSeen = getDracLastSeen(hunterView->game, &movesAgoLastSeen);
    
    if (validPlace(dracLastSeen) && idToType(dracLastSeen) == LAND) {
        for (i = 0; i < NUM_ISLANDS; i++) {
            if (i == idToIsland(dracLastSeen)) {
                hunterView->dracIsland[i] = 1;
            } else {
                hunterView->dracIsland[i] = 0;
            }
        }
    }
    
    // Has Dracula travelled by sea and has he had enough time to change continents
    //int dracSinceAtSea = 0;
    int dracSinceAtSea = getDracSinceAtSea(hunterView->game);
    LocationID dracTrail[TRAIL_SIZE];
    getRealHistory(hunterView->game, PLAYER_DRACULA, dracTrail);
    
    /*for (i = 0; i < TRAIL_SIZE; i++) {
        if (dracTrail[i] == SEA_UNKNOWN || (validPlace(dracTrail[i]) && idToType(dracTrail[i]) == SEA)) {
            dracSinceAtSea = 1;
        }
    }*/
    
    if (dracSinceAtSea) {
        if (hunterView->dracIsland[EUROPE] == 0) {
            if (distanceBetween(hunterView->game, dracLastSeen, ROME, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, LISBON, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, LE_HAVRE, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, AMSTERDAM, 1, 0, 1) <= movesAgoLastSeen + 1) {
                hunterView->dracIsland[EUROPE] = 1;
            }
        }
            
        if (hunterView->dracIsland[BRITAIN] == 0) {
            if (distanceBetween(hunterView->game, dracLastSeen, LIVERPOOL, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, LONDON, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, EDINBURGH, 1, 0, 1) <= movesAgoLastSeen + 1) {
                hunterView->dracIsland[BRITAIN] = 1;
            }
        }
            
        if (hunterView->dracIsland[IRELAND] == 0) {
            if (distanceBetween(hunterView->game, dracLastSeen, DUBLIN, 1, 0, 1) <= movesAgoLastSeen + 1 || distanceBetween(hunterView->game, dracLastSeen, GALWAY, 1, 0, 1) <= movesAgoLastSeen + 1) {
                hunterView->dracIsland[IRELAND] = 1;
            }
        }
            
        if (hunterView->dracIsland[SARDINIA] == 0) {
            if (distanceBetween(hunterView->game, dracLastSeen, CAGLIARI, 1, 0, 1) <= movesAgoLastSeen + 1) {
                hunterView->dracIsland[SARDINIA] = 1;
            }
        }
    }
    
    return hunterView;
}
     
     
// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted){
    
    disposeGameView (toBeDeleted->game);

    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView)
{
    return getRound(currentView->game);
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView)
{
    return getCurrentPlayer(currentView->game);
}

// Get the current score
int giveMeTheScore(HunterView currentView)
{
    return getScore(currentView->game);
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player)
{
    return getHealth(currentView->game, player);
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player)
{
    return getRealLocation(currentView->game, player); // Change(d) to getRealLocation
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    return getHistory(currentView->game, player, trail);
}

// Fills the trail array with the realLocation ids of the last 6 turns
void giveMeTheRealTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    return getRealHistory(currentView->game, player, trail);
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations,
                        int road, int rail, int sea)
{
    return connectedLocations(currentView->game, numLocations, whereIs(currentView, whoAmI(currentView)), whoAmI(currentView), giveMeTheRound(currentView), road, rail, sea);
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    // Currently returns an empty array for invalid inputs (C?, HI, D1-5, etc.)
    
    return connectedLocations(currentView->game, numLocations, whereIs(currentView, player), player, giveMeTheRound(currentView), road, rail, sea);
}

int huntDistanceBetween (HunterView currentView, LocationID src, LocationID dest, int road, int rail, int sea) {
    return distanceBetween (currentView->game, src, dest, road, rail, sea);
}

// Returns copy of the dracIsland array
void giveMeDracIslands(HunterView currentView, int dracIsland[NUM_ISLANDS])
{
    int i;
    for (i = 0; i < NUM_ISLANDS; i++) {
        dracIsland[i] = currentView->dracIsland[i];
    }
}

// Returns Drac's last known location and how long ago he was there
LocationID hunterGetDracLastSeen(HunterView currentView, int *movesAgoLastSeen) {
    return getDracLastSeen(currentView->game, movesAgoLastSeen);
}

// Returns 1 if Drac has been at sea since he was last spotted
int hunterGetDracSinceAtSea(HunterView currentView) {
    return getDracSinceAtSea(currentView->game);
}
