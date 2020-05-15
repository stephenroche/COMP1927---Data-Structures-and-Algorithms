// Map.h ... interface to Map data type

#ifndef MAP_H
#define MAP_H

#include "Places.h"
#include "Globals.h" // Stephen added this for mapConnectedLocations function

#define ENCOUNTER_TRAP              0
#define ENCOUNTER_VAMPIRE           1
#define ENCOUNTER_DRACULA           2

#define NUM_ENCOUNTERS 3

typedef struct edge{
    LocationID  start;
    LocationID  end;
    TransportID type;
} Edge;

// graph representation is hidden 
typedef struct MapRep *Map; 

typedef int EncounterID;

// operations on graphs 
Map  newMap();  
void disposeMap(Map g); 
void showMap(Map g); 
int  numV(Map g);
int  numE(Map g, TransportID t);

#define MAX_CONNECTIONS 20

LocationID *mapConnectedLocations(Map map, int *numLocations, LocationID from, int player, int round, int road, int rail, int sea);
int mapDistanceBetween (Map map, LocationID src, LocationID dest, int road, int rail, int sea);

#endif

