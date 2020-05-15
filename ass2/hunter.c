// hunter.c
// Implementation of your "Fury of Dracula" hunter AI

#include <stdlib.h>
#include <stdio.h>
#include "Game.h"
#include "HunterView.h"
#include <string.h>

static int evaluateMove(HunterView gameState, LocationID move);

void decideHunterMove(HunterView gameState)
{
	registerBestPlay("GE","1111");
	
	
	// TESTING ONLY - this function is heaps useful though
	int movesAgoLastSeen;
	LocationID dracLastSeen = hunterGetDracLastSeen(gameState, &movesAgoLastSeen);
	
	if (dracLastSeen != NOWHERE) {
		printf("Drac last seen %d moves ago in %s\n", movesAgoLastSeen, idToName(dracLastSeen));
	} else {
		printf("Drac not seen yet\n");
	}
	
	if (validPlace(whereIs(gameState, whoAmI(gameState)))) {
		printf("I am in %s\n", idToName(whereIs(gameState, whoAmI(gameState))));
	}
	
	printf("My health is: %d\n", howHealthyIs(gameState, whoAmI(gameState)));
	
	printf("dracSinceAtSea: %d\n", hunterGetDracSinceAtSea(gameState));
	
	int dracIsland[NUM_ISLANDS];
	giveMeDracIslands(gameState, dracIsland);
	/*printf("Drac could be in:\n");
	printf("Europe: %d\n", dracIsland[EUROPE]);
	printf("Britain: %d\n", dracIsland[BRITAIN]);
	printf("Ireland: %d\n", dracIsland[IRELAND]);
	printf("Sardinia: %d\n", dracIsland[SARDINIA]);
	*/
	char message[MESSAGE_SIZE];
	int i;
	for (i = 0; i < NUM_ISLANDS; i++) {
		message[i] = '0' + dracIsland[i];
	}
	//message[NUM_ISLANDS] = '\0';
	strcpy(&message[NUM_ISLANDS], "  <- dracIsland array (Europe, Britain, Ireland, Sardinia)");
	
	int moveValue, bestMove, numLocations;
	LocationID *possibleMoves = whereCanIgo(gameState, &numLocations, 1, 1, 1);
	
	if (numLocations == 0) { // Game just started
		LocationID allLocations[NUM_MAP_LOCATIONS];
		for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
			allLocations[i] = i;
		}
		possibleMoves = allLocations;
		numLocations = NUM_MAP_LOCATIONS;
	}
	
	
	bestMove = -1000000000;
	for (i = 0; i < numLocations; i++) {
		moveValue = evaluateMove(gameState, possibleMoves[i]);
		printf("PossibleMove[ %s ]: %d\n", idToName(possibleMoves[i]), moveValue);
		
		if (moveValue >= bestMove) {
			registerBestPlay(idToAbbrev(possibleMoves[i]), "");
			bestMove = moveValue;
		}
	}
}

static int evaluateMove(HunterView gameState, LocationID move) {
	int moveValue = 0;
	int me = whoAmI(gameState);
	int myLocation = whereIs(gameState, me);
	int player, dist, i;
	int round = giveMeTheRound(gameState);
	
	if (whereIs(gameState, PLAYER_DRACULA) == NOWHERE) { // FIRST MOVE
		if (me == PLAYER_LORD_GODALMING && move == BARI) {
		    moveValue += 1000;
		} else if (me == PLAYER_DR_SEWARD && move == EDINBURGH) {
		    moveValue += 1000;
		} else if (me == PLAYER_VAN_HELSING && move == CADIZ) {
		    moveValue += 1000;
		} else if (me == PLAYER_MINA_HARKER && move == SZEGED) {
		    moveValue += 1000;
		} 
		
	} else { // REGULAR PLAY
		int dracSinceAtSea = hunterGetDracSinceAtSea(gameState);
		LocationID dracTrail[TRAIL_SIZE];
		giveMeTheRealTrail(gameState, PLAYER_DRACULA, dracTrail);
		LocationID dracOrigTrail[TRAIL_SIZE];
		giveMeTheTrail(gameState, PLAYER_DRACULA, dracOrigTrail);
		LocationID playerTrail[TRAIL_SIZE];
		
		int movesAgoLastSeen;
		LocationID dracLastSeen = hunterGetDracLastSeen(gameState, &movesAgoLastSeen);
		
		// Move to Dracula's location if possible
		if (move == whereIs(gameState, PLAYER_DRACULA)) {
		    moveValue += 1000000;
		    return moveValue;
		}
		
		// Do research if Dracula has been lost for a long time
		if (((dracLastSeen == NOWHERE && round > TRAIL_SIZE) || (dracLastSeen != NOWHERE && movesAgoLastSeen > 7)) && move == myLocation) {
			moveValue += 500000;
			return moveValue;
		}
		
		// MINA_HARKER guards CASTLE_DRACULA -- Not for now, see how it goes
		if (0 && me == PLAYER_MINA_HARKER) { // Edited to remove guarding hunter
			// If able to stay / move into guarding position, do so
			if (move == KLAUSENBURG || move == GALATZ) {
				moveValue += 100000;
				
				// Better to alternate between KLAUSENBURG and GALATZ
				if (move == myLocation) { 
					moveValue -= 10;
				}
				
				// ...Although rest if low on health
				if (howHealthyIs(gameState, me) <= 2 && move == myLocation) {
					moveValue += 20;
				}
				
				// If Drac has just moved from CD and is not at your current location, then he MUST be at the other one
				if (dracTrail[1] == CASTLE_DRACULA && move == myLocation) {
					moveValue -= 50000;
				}
				
			// Probably respawned at hospital or moved to encounter dracula, get back to position
			} else {
				dist = huntDistanceBetween(gameState, move, CASTLE_DRACULA, 1, 1, 1);
				moveValue -= dist * 10000;
				
				// Avoid other hunters when getting back to position
				// Avoid city locations on any players' trails, the more recent the location is on the trail, the less likely it is that Dracula would have visited since
				for (player = 0; player < NUM_PLAYERS - 1; player++) {
					giveMeTheRealTrail(gameState, player, playerTrail);
					
					for (i = 0; i < TRAIL_SIZE; i++) {
						if (move == playerTrail[i] && idToType(move) == LAND) {
							moveValue -= (2000 / (((double)(i + (player >= me)) + 0.01)));
							// If a player literally just looked in a city and dracula hasn't moved since, he's probably not there
							// (player >= me) adds 1 if the player hasn't had a turn since Dracula's last move
						}
					}
				}
			}
			
		// Other Hunters patrol
		} else {
			// Dracula is unlikely to be in the hospital, hospital also doesn't provide a shortcut for any travel routes
			if (move == ST_JOSEPH_AND_ST_MARYS) {
			    moveValue -= 1000000;
			}
			
			// If Dracula is at CASTLE_DRACULA, it will show in pastPlays
			if (move == CASTLE_DRACULA) {
				moveValue -= 1000000;
			}
			
			// Prioritise locations Drac could be, otherwise move towards that area
			if (dracLastSeen != NOWHERE) {
				dist = huntDistanceBetween(gameState, dracLastSeen, move, 1, 0, dracSinceAtSea);
				
				// If Drac has hidden or doubled back, the range of locations he could've moved to is decreased
				int hideOrDbUsed = 0;
				for (i = 0; i < movesAgoLastSeen && i < TRAIL_SIZE; i++) {
					if (dracOrigTrail[i] >= DOUBLE_BACK_1 && dracOrigTrail[i] <= DOUBLE_BACK_5 && dracOrigTrail[i] <= (HIDE + movesAgoLastSeen - i)) {
						hideOrDbUsed += dracOrigTrail[i] - DOUBLE_BACK_1 + 1;
					} else if (dracOrigTrail[i] == HIDE) {
						hideOrDbUsed++;
					}
				}
				
				// Prioritise locations Drac could be
				if (dist != -1 && dist <= movesAgoLastSeen - hideOrDbUsed) {
					moveValue += 100000;
					
					// If already in Drac's possible area, prioritise cities over seas
					if (idToType(move) == LAND) {
						moveValue += 1000;
					}
				
				// ...otherwise move towards that area
				} else {
					dist = huntDistanceBetween(gameState, dracLastSeen, move, 1, 1, 1);
					moveValue -= dist * 10000;
				}
				
				// Avoid city locations on any players' trails, the more recent the location is on the trail, the less likely it is that Dracula would have visited since
				for (player = 0; player < NUM_PLAYERS - 1; player++) {
					giveMeTheRealTrail(gameState, player, playerTrail);
					
					for (i = 0; i < TRAIL_SIZE; i++) {
						if (move == playerTrail[i] && idToType(move) == LAND) {
							moveValue -= (4000 / (((double)(i + (player >= me)) + 0.01)));
							// If a player literally just looked in a city and dracula hasn't moved since, he's probably not there
							// (player >= me) adds 1 if the player hasn't had a turn since Dracula's last move
						}
					}
				}
				
				// Spread out - be as far away as possible from other hunters
				int closestDist = 100;
				for (player = 0; player < NUM_PLAYERS - 1; player++) {
					if (player != me) {
						dist = huntDistanceBetween(gameState, move, whereIs(gameState, player), 1, 1, 1) * (1 + 0*(player == PLAYER_MINA_HARKER));
						if (dist < closestDist) {
							closestDist = dist;
						}
					}
				}
				moveValue += closestDist * 500;
				
				// Maximise distance from own trail - increases movement of hunters
				giveMeTheRealTrail(gameState, me, playerTrail);
				for (i = 0; i < 4 && i < round; i++) {
					dist = huntDistanceBetween(gameState, move, playerTrail[i], 1, 1, 1);
					moveValue += dist * 1000;
				}
				
				// Prioritise coastal cities if Drac is at sea / has just come back from sea
				if (round > 1 && (idToType(dracTrail[1]) == SEA || idToType(dracTrail[0]) == SEA) && idToPort(move) == COASTAL) {
					moveValue += 5000;
				}
				
				// Don't stay where you are unless you need to REST
				if (move == myLocation && howHealthyIs(gameState, me) > 4) {
					moveValue -= 10000;
				}
				
				// Prioritise locations that are likely to contain a vampire
				if (((round - movesAgoLastSeen - 1) % 13 == 1 || (round - movesAgoLastSeen - 1) % 13 == 12) && movesAgoLastSeen <= 3 && huntDistanceBetween(gameState, move, dracLastSeen, 1, 0, dracSinceAtSea) == 1) {
					moveValue += 5000;
				}
			
			
			// Start of game, drac hasn't been spotted yet
			} else {
				// Avoid city locations on any players' trails, the more recent the location is on the trail, the less likely it is that Dracula would have visited since
				for (player = 0; player < NUM_PLAYERS - 1; player++) {
					giveMeTheRealTrail(gameState, player, playerTrail);
					
					for (i = 0; i < TRAIL_SIZE; i++) {
						if (move == playerTrail[i] && idToType(move) == LAND) {
							moveValue -= (2000 / (((double)(i + (player >= me)) + 0.01)));
							// If a player literally just looked in a city and dracula hasn't moved since, he's probably not there
							// (player >= me) adds 1 if the player hasn't had a turn since Dracula's last move
						}
					}
				}
				
				// Spread out - be as far away as possible from other hunters
				int closestDist = 100;
				for (player = 0; player < NUM_PLAYERS - 1; player++) {
					if (player != me) {
						dist = huntDistanceBetween(gameState, move, whereIs(gameState, player), 1, 1, 1) * (1 + 0*(player == PLAYER_MINA_HARKER));
						if (dist < closestDist) {
							closestDist = dist;
						}
					}
				}
				moveValue += closestDist * 500;
				
				// Maximise distance from own trail - increases movement of hunters
				giveMeTheRealTrail(gameState, me, playerTrail);
				for (i = 0; i < 4 && i < round; i++) {
					dist = huntDistanceBetween(gameState, move, playerTrail[i], 1, 1, 1);
					moveValue += dist * 1000;
				}
				
				// Prioritise cities over seas for finding Dracula
				if (idToType(move) == LAND) {
					moveValue += 3000;
				}
				
				// Prioritise coastal cities if Drac is at sea / has just come back from sea
				if (round > 1 && (idToType(dracTrail[1]) == SEA || idToType(dracTrail[0]) == SEA) && idToPort(move) == COASTAL) {
					moveValue += 5000;
				}
				
				// Don't stay where you are unless you need to REST
				if (move == myLocation && howHealthyIs(gameState, me) > 4) {
					moveValue -= 10000;
				}
			}
		}
	}
	
	return moveValue;
}
