// Places.c ... implementation of Places

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Places.h"

typedef struct Place {
   char      *name;
   char      *abbrev;
   LocationID id;
   PlaceType  type;
   int        island;
   int        coastal;
} Place;

// Places should appear in alphabetic order
// Each entry should satisfy (places[i].id == i)
// First real place must be at index MIN_MAP_LOCATION
// Last real place must be at index MAX_MAP_LOCATION
static Place places[] =
{
   {"Adriatic Sea", "AS", ADRIATIC_SEA, SEA, SEA, SEA},
   {"Alicante", "AL", ALICANTE, LAND, EUROPE, COASTAL},
   {"Amsterdam", "AM", AMSTERDAM, LAND, EUROPE, COASTAL},
   {"Athens", "AT", ATHENS, LAND, EUROPE, COASTAL},
   {"Atlantic Ocean", "AO", ATLANTIC_OCEAN, SEA, SEA, SEA},
   {"Barcelona", "BA", BARCELONA, LAND, EUROPE, COASTAL},
   {"Bari", "BI", BARI, LAND, EUROPE, COASTAL},
   {"Bay of Biscay", "BB", BAY_OF_BISCAY, SEA, SEA, SEA},
   {"Belgrade", "BE", BELGRADE, LAND, EUROPE, LAND},
   {"Berlin", "BR", BERLIN, LAND, EUROPE, LAND},
   {"Black Sea", "BS", BLACK_SEA, SEA, SEA, SEA},
   {"Bordeaux", "BO", BORDEAUX, LAND, EUROPE, COASTAL},
   {"Brussels", "BU", BRUSSELS, LAND, EUROPE, LAND},
   {"Bucharest", "BC", BUCHAREST, LAND, EUROPE, LAND},
   {"Budapest", "BD", BUDAPEST, LAND, EUROPE, LAND},
   {"Cadiz", "CA", CADIZ, LAND, EUROPE, COASTAL},
   {"Cagliari", "CG", CAGLIARI, LAND, SARDINIA, COASTAL},
   {"Castle Dracula", "CD", CASTLE_DRACULA, LAND, EUROPE, LAND},
   {"Clermont-Ferrand", "CF", CLERMONT_FERRAND, LAND, EUROPE, LAND},
   {"Cologne", "CO", COLOGNE, LAND, EUROPE, LAND},
   {"Constanta", "CN", CONSTANTA, LAND, EUROPE, COASTAL},
   {"Dublin", "DU", DUBLIN, LAND, IRELAND, COASTAL},
   {"Edinburgh", "ED", EDINBURGH, LAND, BRITAIN, COASTAL},
   {"English Channel", "EC", ENGLISH_CHANNEL, SEA, SEA, SEA},
   {"Florence", "FL", FLORENCE, LAND, EUROPE, LAND},
   {"Frankfurt", "FR", FRANKFURT, LAND, EUROPE, LAND},
   {"Galatz", "GA", GALATZ, LAND, EUROPE, LAND},
   {"Galway", "GW", GALWAY, LAND, IRELAND, COASTAL},
   {"Geneva", "GE", GENEVA, LAND, EUROPE, LAND},
   {"Genoa", "GO", GENOA, LAND, EUROPE, COASTAL},
   {"Granada", "GR", GRANADA, LAND, EUROPE, LAND},
   {"Hamburg", "HA", HAMBURG, LAND, EUROPE, COASTAL},
   {"Ionian Sea", "IO", IONIAN_SEA, SEA, SEA, SEA},
   {"Irish Sea", "IR", IRISH_SEA, SEA, SEA, SEA},
   {"Klausenburg", "KL", KLAUSENBURG, LAND, EUROPE, LAND},
   {"Le Havre", "LE", LE_HAVRE, LAND, EUROPE, COASTAL},
   {"Leipzig", "LI", LEIPZIG, LAND, EUROPE, LAND},
   {"Lisbon", "LS", LISBON, LAND, EUROPE, COASTAL},
   {"Liverpool", "LV", LIVERPOOL, LAND, BRITAIN, COASTAL},
   {"London", "LO", LONDON, LAND, BRITAIN, COASTAL},
   {"Madrid", "MA", MADRID, LAND, EUROPE, LAND},
   {"Manchester", "MN", MANCHESTER, LAND, BRITAIN, LAND},
   {"Marseilles", "MR", MARSEILLES, LAND, EUROPE, COASTAL},
   {"Mediterranean Sea", "MS", MEDITERRANEAN_SEA, SEA, SEA, SEA},
   {"Milan", "MI", MILAN, LAND, EUROPE, LAND},
   {"Munich", "MU", MUNICH, LAND, EUROPE, LAND},
   {"Nantes", "NA", NANTES, LAND, EUROPE, COASTAL},
   {"Naples", "NP", NAPLES, LAND, EUROPE, COASTAL},
   {"North Sea", "NS", NORTH_SEA, SEA, SEA, SEA},
   {"Nuremburg", "NU", NUREMBURG, LAND, EUROPE, LAND},
   {"Paris", "PA", PARIS, LAND, EUROPE, LAND},
   {"Plymouth", "PL", PLYMOUTH, LAND, BRITAIN, COASTAL},
   {"Prague", "PR", PRAGUE, LAND, EUROPE, LAND},
   {"Rome", "RO", ROME, LAND, EUROPE, COASTAL},
   {"Salonica", "SA", SALONICA, LAND, EUROPE, COASTAL},
   {"Santander", "SN", SANTANDER, LAND, EUROPE, COASTAL},
   {"Saragossa", "SR", SARAGOSSA, LAND, EUROPE, LAND},
   {"Sarajevo", "SJ", SARAJEVO, LAND, EUROPE, LAND},
   {"Sofia", "SO", SOFIA, LAND, EUROPE, LAND},
   {"St Joseph and St Marys", "JM", ST_JOSEPH_AND_ST_MARYS, LAND, EUROPE, LAND},
   {"Strasbourg", "ST", STRASBOURG, LAND, EUROPE, LAND},
   {"Swansea", "SW", SWANSEA, LAND, BRITAIN, COASTAL},
   {"Szeged", "SZ", SZEGED, LAND, EUROPE, LAND},
   {"Toulouse", "TO", TOULOUSE, LAND, EUROPE, LAND},
   {"Tyrrhenian Sea", "TS", TYRRHENIAN_SEA, SEA, SEA, SEA},
   {"Valona", "VA", VALONA, LAND, EUROPE, COASTAL},
   {"Varna", "VR", VARNA, LAND, EUROPE, COASTAL},
   {"Venice", "VE", VENICE, LAND, EUROPE, COASTAL},
   {"Vienna", "VI", VIENNA, LAND, EUROPE, LAND},
   {"Zagreb", "ZA", ZAGREB, LAND, EUROPE, LAND},
   {"Zurich", "ZU", ZURICH, LAND, EUROPE, LAND},
};

// given a Place number, return its name
char *idToName(LocationID p)
{
   assert(validPlace(p));
   return places[p].name;
}

// given a Place number, return its type
int idToType(LocationID p)
{
   if (p == CITY_UNKNOWN) {
      return LAND;
   } else if (p == SEA_UNKNOWN) {
      return SEA;
   }
   
   assert (validPlace(p));
   return places[p].type;
}

// given a Place name, return its ID number
// binary search
int nameToID(char *name)
{
   int lo = MIN_MAP_LOCATION, hi = MAX_MAP_LOCATION;
   while (lo <= hi) {
      int mid = (hi+lo)/2;
      int ord = strcmp(name,places[mid].name);
      if (ord < 0)
         hi = mid-1;
      else if (ord > 0)
         lo = mid+1;
      else
         return places[mid].id;
   }
   return NOWHERE;
}

// given a Place abbreviation (2 char), return its ID number
int abbrevToID(char *abbrev)
{
   // an attempt to optimise a linear search
   Place *p;
   Place *first = &places[MIN_MAP_LOCATION];
   Place *last = &places[MAX_MAP_LOCATION];
   for (p = first; p <= last; p++) {
      char *c = p->abbrev;
      if (c[0] == abbrev[0] && c[1] == abbrev[1] && c[2] == '\0') return p->id;
   }
   
   // Check other locations
   switch (abbrev[0]) {
      case 'C':
         return CITY_UNKNOWN;
      break;
      
      case 'S':
         return SEA_UNKNOWN;
      break;
      
      case 'H':
         return HIDE;
      break;
      
      case 'D':
         if (abbrev[1] == '1') {
            return DOUBLE_BACK_1;
         } else if (abbrev[1] == '2') {
            return DOUBLE_BACK_2;
         } else if (abbrev[1] == '3') {
            return DOUBLE_BACK_3;
         } else if (abbrev[1] == '4') {
            return DOUBLE_BACK_4;
         } else if (abbrev[1] == '5') {
            return DOUBLE_BACK_5;
         }
      break;
      
      case 'T':
         return TELEPORT;
      break;
   }
   
   return NOWHERE;
}

// given a Place number, return its abbreviation
char *idToAbbrev(LocationID p)
{
   if (p == TELEPORT) {
      return "TP";
   } else if (p == HIDE) {
      return "HI";
   } else if (p == DOUBLE_BACK_1) {
      return "D1";
   } else if (p == DOUBLE_BACK_2) {
      return "D2";
   } else if (p == DOUBLE_BACK_3) {
      return "D3";
   } else if (p == DOUBLE_BACK_4) {
      return "D4";
   } else if (p == DOUBLE_BACK_5) {
      return "D5";
   }
   
   assert(validPlace(p));
   return places[p].abbrev;
}

// give a Place number, return its island group
int idToIsland(LocationID p)
{
   if (p == CITY_UNKNOWN) {
      return ANYWHERE;
   } else if (p == SEA_UNKNOWN) {
      return SEA;
   }
   
   assert (validPlace(p));
   return places[p].island;
}

// give a Place number, return its coastal status
int idToPort(LocationID p)
{
   if (!validPlace(p)) {
      return -1;
   }
   
   return places[p].coastal;
}
