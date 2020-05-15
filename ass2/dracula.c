// dracula.c
// Implementation of your "Fury of Dracula" Dracula AI

#include <stdlib.h>
#include <stdio.h>
#include "Game.h"
#include "DracView.h"

static int evaluateMove(DracView gameState, LocationID move);

void decideDraculaMove(DracView gameState)
{
	registerBestPlay("CD", "default");
	
	printf("lastEncountered = %d\n", lastFoundMe(gameState));
	
	int moveValue, bestMove, numLocations, i, j;
	LocationID *possibleMoves = whereCanIgo(gameState, &numLocations, 1, 1);
	
	if (numLocations == 0) {
		if (whereIs(gameState, PLAYER_DRACULA) != NOWHERE) {
			// Only option is to TELEPORT
			registerBestPlay("TP", "");
			
		} else { // Dracula yet to make a move
			LocationID allLocations[NUM_MAP_LOCATIONS];
			for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
				allLocations[i] = i;
				if (i == ST_JOSEPH_AND_ST_MARYS) {
					allLocations[i] = CASTLE_DRACULA; // Erase hospital from possible moves
				}
			}
			possibleMoves = allLocations;
			numLocations = NUM_MAP_LOCATIONS;
		}
	}
	
	bestMove = -1000000000;
	for (i = 0; i < numLocations; i++) {
		moveValue = evaluateMove(gameState, possibleMoves[i]);
		printf("PossibleMove[ %s ]: %d\n", idToName(possibleMoves[i]), moveValue);
		
		// Convert move to D1-5 or HI if necessary
		LocationID dracTrail[TRAIL_SIZE];
		giveMeTheTrail(gameState, PLAYER_DRACULA, dracTrail);
		int hideUsed = 0;
		LocationID move = possibleMoves[i];
		for (j = 0; j < TRAIL_SIZE - 1; j++) {
			if (dracTrail[j] == HIDE) {
				hideUsed = 1;
			}
			if (move == dracTrail[j]) {
				move = DOUBLE_BACK_1 + j /*- (j > 0 && dracTrail[j - 1] == HIDE)*/;
			}
		}
		if (possibleMoves[i] == whereIs(gameState, PLAYER_DRACULA) && !hideUsed && idToType(possibleMoves[i]) == LAND) {
			move = HIDE;
		}
		
		if (moveValue >= bestMove) {
			registerBestPlay(idToAbbrev(move), "");
			bestMove = moveValue;
		}
	}
}

static int evaluateMove(DracView gameState, LocationID move) {
	int moveValue = 0;
	int player, dist, i;
	
	// FIRST MOVE
	if (whereIs(gameState, PLAYER_DRACULA) == NOWHERE) {
		if (move == CASTLE_DRACULA) {
			int castleGuarded = 0;
			for (player = 0; player < NUM_PLAYERS - 1; player++) {
				//printf("%d\n", dracDistanceBetween(gameState, CASTLE_DRACULA, whereIs(gameState, player), 1, 1, 1));
				if (dracDistanceBetween(gameState, CASTLE_DRACULA, whereIs(gameState, player), 1, 1, 1) <= 4) {
					castleGuarded = 1;
				}
			}
			
			if (!castleGuarded) {
				moveValue += 50; // Only most isolated locations will beat CD - may as well spawn there to build up more health
			}
		}
		
		// Dracula prefers an isolated location
		int closestDist = 100;
		int secondClosestDist = 100;
		for (player = 0; player < NUM_PLAYERS - 1; player++) {
			// Only use sea moves if that's the only connection between the two locations
			dist = dracDistanceBetween(gameState, move, whereIs(gameState, player), 1, 1, (idToIsland(move) != idToIsland(whereIs(gameState, player)))) + (idToIsland(move) != idToIsland(whereIs(gameState, player))); // Add 1 if sea move necessary (hunters unlikely to immediately cross sea)
			//printf("dist = %d\n", dist);
			if (dist < closestDist) {
				secondClosestDist = closestDist;
				closestDist = dist;
			} else if (dist < secondClosestDist) {
				secondClosestDist = dist;
			}
		}
		moveValue += closestDist * 10;
		moveValue += secondClosestDist * 4;
		
		
		// Avoid Ireland as that would require multiple subsequent sea moves; Britain also isn't Great
		if (idToIsland(move) == IRELAND) {
			moveValue -= 10;
		} else if (idToIsland(move) == BRITAIN) {
			moveValue -= 5;
		}
		
		
	// REGULAR PLAY
	} else {
		LocationID dracTrail[TRAIL_SIZE];
		giveMeTheRealTrail(gameState, PLAYER_DRACULA, dracTrail);
		LocationID dracOrigTrail[TRAIL_SIZE];
		giveMeTheTrail(gameState, PLAYER_DRACULA, dracOrigTrail);

		// Avoid locations that a hunter could move to in the next round pretty much at all costs
		// Effect compounded if multiple hunters can move there
		// No encounters can occur at sea
		LocationID *hunterMoves;
		int numLocations;
		for (player = 0; player < NUM_PLAYERS - 1; player++) {
			hunterMoves = whereCanTheyGo(gameState, &numLocations, player, 1, 1, 1);
			for (i = 0; i < numLocations; i++) {
				if (hunterMoves[i] == move && idToType(move) != SEA) {
					moveValue -= 1000;
				}
			}
		}
		/*
		// Dracula likes his castle if there's no one immediately adjacent
		if (moveValue == 0 && move == CASTLE_DRACULA) {
			moveValue += 1000;
		}
		*/
		
		// Dracula prefers an isolated location
		int closestDist = 100;
		int secondClosestDist = 100;
		for (player = 0; player < NUM_PLAYERS - 1; player++) {
			dist = dracDistanceBetween(gameState, move, whereIs(gameState, player), 1, 1, 1);
			//printf("dist = %d\n", dist);
			if (dist < closestDist) {
				secondClosestDist = closestDist;
				closestDist = dist;
			} else if (dist < secondClosestDist) {
				secondClosestDist = dist;
			}
		}
		moveValue += closestDist * 10;
		moveValue += secondClosestDist * 4;
		
		// Dracula doesn't like staying at sea
		if (idToType(move) == SEA) {
			moveValue -= 100;
			
			// Definitely don't travel by sea if on very low health
			int myHealth = howHealthyIs(gameState, PLAYER_DRACULA);
			if (myHealth <= 6) {
				moveValue -= 12000 / myHealth;
			}
		}
		
		// Prefer move instead of HIDE
		if (move == whereIs(gameState, PLAYER_DRACULA)) {
			moveValue -= 1;
		}
		
		// Run if they know your location (maximise dist from last seen)
		int lastSawMeIndex = lastFoundMe(gameState);
		if (lastSawMeIndex >= 0 && lastSawMeIndex < 3) {
			dist = dracDistanceBetween(gameState, move, dracTrail[lastSawMeIndex], 1, 1, 1);
			moveValue += 2000 * dist;
		}
		
		// DO NOT move to a hunter's current location (they can see you there)
		int hunterThere = 0;
		for (player = 0; player < NUM_PLAYERS - 1; player++) {
			if (move == whereIs(gameState, player)) {
				hunterThere = 1;
			}
		}
		if (hunterThere && idToType(move) == LAND) {
			moveValue -= 5000;
		}
		
		// If at CD, run when they get to GA or KL
		if (move == CASTLE_DRACULA) {
			int hunterGA = 0;
			int hunterKL = 0;
			for (player = 0; player < NUM_PLAYERS - 1; player++) {
				if (whereIs(gameState, player) == GALATZ) {
					hunterGA = 1;
				} else if (whereIs(gameState, player) == KLAUSENBURG) {
					hunterKL = 1;
				}
			}
			// If only one of the two cities has a hunter there, run to the free one instead of staying at CD (XOR)
			if ((hunterGA || hunterKL) && !(hunterGA && hunterKL)) {
				moveValue -= 5000;
			}
		}
		
		// Move towards CD if it's unguarded and Drac is already close-ish
		int castleDist = dracDistanceBetween(gameState, move, CASTLE_DRACULA, 1, 0, 1);
		if (castleDist <= 3) {
			int castleGuarded = 0;
			for (player = 0; player < NUM_PLAYERS - 1; player++) {
				if (dracDistanceBetween(gameState, CASTLE_DRACULA, whereIs(gameState, player), 1, 1, 1) <= 4) {
					castleGuarded = 1;
				}
			}
			
			if (!castleGuarded && move == CASTLE_DRACULA) {
				moveValue += 1000;
				
			} else {
				
				int hideOrDbUsed = 0;
				int CDmoveUsed = 0;
				for (i = 0; i < TRAIL_SIZE - castleDist - 1; i++) {
					if (dracOrigTrail[i] >= HIDE && dracOrigTrail[i] <= DOUBLE_BACK_5) {
						hideOrDbUsed++;
					}
				}
				for (i = 0; i < TRAIL_SIZE - castleDist; i++) {
					if (dracOrigTrail[i] == CASTLE_DRACULA) {
						CDmoveUsed++;
					}
				}
				
				if (!castleGuarded && hideOrDbUsed != 2 && !CDmoveUsed) {
					moveValue += (4 - castleDist) * 15;
				}
			}
		}
		
		// Avoid unnecessarily using up DOUBLE_BACKs and HIDEs
		for (i = 0; i < TRAIL_SIZE; i++) {
			if (move == dracOrigTrail[i]) {
				moveValue -= 1;
			}
		}
	}
	
	return moveValue;
}
