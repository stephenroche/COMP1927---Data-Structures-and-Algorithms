// GameView.c ... GameView ADT implementation
/* Authors:
 * Anantnath Thakur 
 * Hogan Richardson 
 * Sen Le 
 * Stephen Roche 
 * Weiyang Fang 
 */

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include <string.h>
#include <stdio.h> // FOR DEBUGGING ONLY

#define MAX_MESSAGES GAME_START_SCORE*NUM_PLAYERS
#define PLAY_STRING_SIZE 8
#define GAIN 1
#define LOSS -1
#define DOUBLE_BACK_OFFSET 102

// Data Structures 
typedef struct _player {
    int health; // Blood Points
    LocationID location;
    LocationID realLocation;
    LocationID trail[TRAIL_SIZE]; // Newest location at index 0
    LocationID realTrail[TRAIL_SIZE]; // Replaces all HIs, D1-5s and TPs with actual locations (or C?/S? for hunters looking at Dracula), same goes for realLocation
} Player;

struct gameView {
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    //WORKING: 
    // GAME
    // Game score
    // PLAYERS - Player struct with player type, health, position, Player history (move and move type)
    // Current Player
    // Position of all players
    // Health of all players
    // OVERALL 
    // Map - whether certain 

    Map map;
    int gameScore;
    Round round;

    PlayerID currentPlayer;
    PlayerMessage messages[MAX_MESSAGES];
    Player players[NUM_PLAYERS]; // Index in array = PlayerID
    
    LocationID dracLastSeen;
    int movesAgoLastSeen;
    int dracSinceAtSea;
};

// Static Functions
//static void getEncounters(GameView gameView, EncounterID id, PlayerID player, LocationID location);
static EncounterID abbrevToEncounter (char encounter);
static void processEncounter (GameView g, EncounterID id, PlayerID player);
static PlayerID abbrevToPlayer(char * player);
static void addToTrail (GameView g, PlayerID player, LocationID location);
static void addToRealTrail (GameView g, PlayerID player, LocationID location);
static void updateHealth (GameView g, PlayerID player, int change, int gainLoss);
//static int dracMoveToSea (GameView g, LocationID location, int trailIndex);

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    // Initialise All Values 
    GameView gameView = malloc(sizeof(struct gameView));
    assert (gameView != NULL); // Check malloc was successful
    
    gameView->gameScore = GAME_START_SCORE;
    gameView->round = 0;
    gameView->currentPlayer = PLAYER_LORD_GODALMING;
    gameView->map = newMap();
    gameView->dracLastSeen = NOWHERE;
    gameView->movesAgoLastSeen = TRAIL_SIZE; // Shouldn't be accessed until Drac has been seen at least once
    gameView->dracSinceAtSea = 0;
    
    // Player Points Initialisation 
    int p = 0;
    for (p = 0; p < NUM_PLAYERS; p++) {
        
        // Set Health 
        if (p == PLAYER_DRACULA) {
            gameView->players[p].health = GAME_START_BLOOD_POINTS; 
        } else { // Hunters
            gameView->players[p].health = GAME_START_HUNTER_LIFE_POINTS; 
        }
        
        // Set locations to NOWHERE    
        gameView->players[p].location = NOWHERE;
        gameView->players[p].realLocation = NOWHERE;
        
        // Initialise all trail locations to NOWHERE
        int t = 0;
        for (t = 0; t < TRAIL_SIZE; t++) {
            gameView->players[p].trail[t] = NOWHERE;
            gameView->players[p].realTrail[t] = NOWHERE;
        }
    }
    
    int i;
    
    // Parse string and update game values
    int turnCount = 0; // Used to determine how many rounds
    int length = strlen(pastPlays);
    LocationID turnStartedIn;
    
    for (i = 0; i < length; i += PLAY_STRING_SIZE) {
        // Update Player
        // Get current player
        PlayerID curr = abbrevToPlayer(&pastPlays[i]);
        
        // Assign location turn was started in (for resting purposes, could be different to trail[1])
        turnStartedIn = gameView->players[curr].location;
        
        // Assign location
        gameView->players[curr].location = abbrevToID(&pastPlays[i + 1]);
        
        // Assign realLocation
        gameView->players[curr].realLocation = gameView->players[curr].location;
        
        if (gameView->players[curr].realLocation == HIDE) {
            gameView->players[curr].realLocation = gameView->players[curr].realTrail[0];
            
        } else if (gameView->players[curr].realLocation >= DOUBLE_BACK_1 && gameView->players[curr].realLocation <= DOUBLE_BACK_5) {
            gameView->players[curr].realLocation = gameView->players[curr].realTrail[gameView->players[curr].realLocation - DOUBLE_BACK_1];
            
        } else if (gameView->players[curr].realLocation == TELEPORT) {
            gameView->players[curr].realLocation = CASTLE_DRACULA;
        }
        
        // Update dracLastSeen
        if (curr == PLAYER_DRACULA) {
            if (validPlace(gameView->players[curr].realLocation)) {
                gameView->dracLastSeen = gameView->players[curr].realLocation;
                gameView->movesAgoLastSeen = 0;
                gameView->dracSinceAtSea = 0;
            } else {
                gameView->movesAgoLastSeen++;
            }
        }
        
        // Encounters
        if (curr == PLAYER_DRACULA) {
            // Process Dracula Actions 
            
            if (pastPlays[i + 5] == 'V') {
                // V at trail[5] matures
                processEncounter(gameView, ENCOUNTER_VAMPIRE, curr);
            }
            
            // Update GameScore
            gameView->gameScore -= SCORE_LOSS_DRACULA_TURN;
           	
            if (gameView->players[curr].realLocation == SEA_UNKNOWN || (validPlace(gameView->players[curr].realLocation) && isSea(gameView->players[curr].realLocation))) {
                updateHealth(gameView, curr, LIFE_LOSS_SEA, LOSS);
                
            } else if (gameView->players[curr].realLocation == CASTLE_DRACULA) {
                updateHealth(gameView, curr, LIFE_GAIN_CASTLE_DRACULA, GAIN);
            }
            
        } else {
            // Hunter starts turn in hospital (hospital doesn't actually appear in trail)
            if (gameView->players[curr].health == 0) {
                gameView->players[curr].health = GAME_START_HUNTER_LIFE_POINTS;
            } 
            
            // Process Encounters
            int start;
            for (start = 3; start < 7; start++) {
                EncounterID id = abbrevToEncounter(pastPlays[i + start]);
                if (id >= 0) {
                    // Hunter faces encounters
                    processEncounter(gameView, id, curr);
                    
                    // Update dracLastSeen
                    if (id == ENCOUNTER_VAMPIRE) {
                        Round vampireSet = 0;
                        while (vampireSet < gameView->round) {
                            vampireSet += 13;
                        }
                        vampireSet -= 13;
                        
                        int movesAgo = gameView->round - vampireSet - 1;
                        //printf("movesAgo = %d\n", movesAgo);
                        if (movesAgo < gameView->movesAgoLastSeen || gameView->dracLastSeen == NOWHERE) {
                            gameView->dracLastSeen = gameView->players[curr].location;
                            gameView->movesAgoLastSeen = movesAgo;
                        }
                        
                    // Found Drac's trail but don't know when he was there
                    } else {
                        if (gameView->movesAgoLastSeen > 5 || gameView->dracLastSeen == NOWHERE) {
                            gameView->dracLastSeen = gameView->players[curr].location;
                            gameView->movesAgoLastSeen = 5;
                        
                        // Else if this location on his trail must be more recent than Drac's last certain known location (due to how far he can travel in a certain number of moves)
                        } else if (mapDistanceBetween(gameView->map, gameView->dracLastSeen, gameView->players[curr].location, 1, 0, gameView->dracSinceAtSea) >= (TRAIL_SIZE - gameView->movesAgoLastSeen)) {
                            
                            gameView->dracLastSeen = gameView->players[curr].location;
                            gameView->movesAgoLastSeen -= mapDistanceBetween(gameView->map, gameView->dracLastSeen, gameView->players[curr].location, 1, 0, gameView->dracSinceAtSea);
                        }
                    }
                }
            }
        }
       
        // Add location to trail (must be completed after dracula actions processed) 
        addToTrail(gameView, curr, gameView->players[curr].location);
        addToRealTrail(gameView, curr, gameView->players[curr].realLocation);
        
        if (curr != PLAYER_DRACULA) {
            // If hunter has been killed, location must be changed to hospital even though trail still shows location where the hunter travelled to and was killed (idk I asked JAS)
            // Health remains at 0 until start of next turn
            if (gameView->players[curr].health == 0) {
                gameView->players[curr].location = ST_JOSEPH_AND_ST_MARYS;
                gameView->players[curr].realLocation = ST_JOSEPH_AND_ST_MARYS;
                
            // Check if hunter rested
            } else if (turnStartedIn == gameView->players[curr].location) {
                updateHealth(gameView, curr, LIFE_GAIN_REST, GAIN);
            }
        }
    
        // Update if Drac had been at sea since he was last spotted
        int j;
        LocationID currLoc;
        for (j = 0; j <= gameView->movesAgoLastSeen && j < TRAIL_SIZE; j++) {
            currLoc = gameView->players[PLAYER_DRACULA].realTrail[j];
            if (currLoc == SEA_UNKNOWN || (validPlace(currLoc) && idToType(currLoc) == SEA)) {
                gameView->dracSinceAtSea = 1;
            }
        }
         
        // Check if round completed 
        turnCount++;
        if (!((turnCount) % 5)) {
            gameView->round++;
        }
    }
    
    // Copy messages array
    for (i = 0; i < turnCount; i++) {
        strcpy(gameView->messages[i], messages[i]);
    }
   
    gameView->currentPlayer = (turnCount) % 5;
    
    // If current hunter has been killed, health restored at start of turn
    if (gameView->players[gameView->currentPlayer].health == 0) {
        gameView->players[gameView->currentPlayer].health = GAME_START_HUNTER_LIFE_POINTS;
    }
    
    return gameView;
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

static void processEncounter (GameView g, EncounterID id, PlayerID player) {
    
    if (id == ENCOUNTER_TRAP) {
        if (player != PLAYER_DRACULA) {
            updateHealth(g, player, LIFE_LOSS_TRAP_ENCOUNTER, LOSS);
        }
        
    } else if (id == ENCOUNTER_VAMPIRE) {
        if (player == PLAYER_DRACULA) {
            g->gameScore -= SCORE_LOSS_VAMPIRE_MATURES;
        }
    } else if (id == ENCOUNTER_DRACULA) {
        updateHealth(g, player, LIFE_LOSS_DRACULA_ENCOUNTER, LOSS);
        updateHealth(g, PLAYER_DRACULA, LIFE_LOSS_HUNTER_ENCOUNTER, LOSS);
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

/* addToTrail
 * adds given Location to a player's trail
 */
static void addToTrail (GameView g, PlayerID player, LocationID location)
{
    // Shift trail along, remove 5th item
    int i;
    for (i = (TRAIL_SIZE - 1); i > 0; i--) {
        g->players[player].trail[i] = g->players[player].trail[i - 1];
    }
    // Add current location to front of trail
    g->players[player].trail[0] = location;
}

/* addToRealTrail
 * adds given Location to a player's realTrail
 */
static void addToRealTrail (GameView g, PlayerID player, LocationID location)
{
    // Shift trail along, remove 5th item
    int i;
    for (i = (TRAIL_SIZE - 1); i > 0; i--) {
        g->players[player].realTrail[i] = g->players[player].realTrail[i - 1];
    }
    // Add current location to front of trail
    g->players[player].realTrail[0] = location;
}
 
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    //COMPLETE THIS IMPLEMENTATION
    disposeMap(toBeDeleted->map);
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    return currentView->round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    return (currentView->currentPlayer);
}

// Get the current score
int getScore(GameView currentView)
{
    return currentView->gameScore;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    return currentView->players[player].health;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    return currentView->players[player].location;
}

// Get the current realLocation id of a given player
LocationID getRealLocation(GameView currentView, PlayerID player)
{
    return currentView->players[player].realLocation;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = currentView->players[player].trail[i];
    }
}

// Fills the trail array with the realLocation ids of the last 6 turns
void getRealHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = currentView->players[player].realTrail[i];
    }
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    return mapConnectedLocations(currentView->map, numLocations, from, player, round, road, rail, sea);
}

static void updateHealth (GameView g, PlayerID player, int change, int gainLoss) {
    
    g->players[player].health += change * gainLoss;
    
    if (g->players[player].health <= 0) {
        g->players[player].health = 0;
        if (player != PLAYER_DRACULA) {
            g->gameScore -= SCORE_LOSS_HUNTER_HOSPITAL;
        }
    }
    
    if (player != PLAYER_DRACULA && g->players[player].health > GAME_START_HUNTER_LIFE_POINTS) {
        g->players[player].health = GAME_START_HUNTER_LIFE_POINTS;
    }
    
    return;
}

int distanceBetween (GameView currentView, LocationID src, LocationID dest, int road, int rail, int sea) {
    return mapDistanceBetween (currentView->map, src, dest, road, rail, sea);
}

LocationID getDracLastSeen(GameView currentView, int *movesAgoLastSeen) {
    *movesAgoLastSeen = currentView->movesAgoLastSeen;
    return currentView->dracLastSeen;
}

int getDracSinceAtSea (GameView currentView) {
    return currentView->dracSinceAtSea;
}
