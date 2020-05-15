// Graph.c ... implementation of Graph ADT
// Written by John Shepherd, May 2013

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Graph.h"
#include "Queue.h"

// graph representation (adjacency matrix)
typedef struct GraphRep {
	int    nV;    // #vertices
	int    nE;    // #edges
	int  **edges; // matrix of weights (0 == no edge)
} GraphRep;

// check validity of Vertex
int validV(Graph g, Vertex v)
{
	return (g != NULL && v >= 0 && v < g->nV);
}

// make an edge
Edge mkEdge(Graph g, Vertex v, Vertex w)
{
	assert(g != NULL && validV(g,v) && validV(g,w));
	Edge new = {v,w}; // struct assignment
	return new;
}

// insert an Edge
// - sets (v,w) and (w,v)
void insertEdge(Graph g, Vertex v, Vertex w, int wt)
{
	assert(g != NULL && validV(g,v) && validV(g,w));
	if (g->edges[v][w] == 0) {
		g->edges[v][w] = wt;
		g->edges[w][v] = wt;
		g->nE++;
	}
}

// remove an Edge
// - unsets (v,w) and (w,v)
void removeEdge(Graph g, Vertex v, Vertex w)
{
	assert(g != NULL && validV(g,v) && validV(g,w));
	if (g->edges[v][w] != 0) {
		g->edges[v][w] = 0;
		g->edges[w][v] = 0;
		g->nE--;
	}
}

// create an empty graph
Graph newGraph(int nV)
{
	assert(nV > 0);
	int v, w;
	Graph new = malloc(sizeof(GraphRep));
	assert(new != 0);
	new->nV = nV; new->nE = 0;
	new->edges = malloc(nV*sizeof(int *));
	assert(new->edges != 0);
	for (v = 0; v < nV; v++) {
		new->edges[v] = malloc(nV*sizeof(int));
		assert(new->edges[v] != 0);
		for (w = 0; w < nV; w++)
			new->edges[v][w] = 0;
	}
	return new;
}

// free memory associated with graph
void dropGraph(Graph g)
{
	assert(g != NULL);
	// not needed for this lab
}

// display graph, using names for vertices
void showGraph(Graph g, char **names)
{
	assert(g != NULL);
	printf("#vertices=%d, #edges=%d\n\n",g->nV,g->nE);
	int v, w;
	for (v = 0; v < g->nV; v++) {
		printf("%d %s\n",v,names[v]);
		for (w = 0; w < g->nV; w++) {
			if (g->edges[v][w]) {
				printf("\t%s (%d)\n",names[w],g->edges[v][w]);
			}
		}
		printf("\n");
	}
}

// find a path between two vertices using breadth-first traversal
// only allow edges whose weight is less than "max"
int findPath(Graph g, Vertex src, Vertex dest, int max, int *path)
{
	assert(g != NULL);

   int pathLength;
   int *visited = calloc(g->nV, sizeof(int));

   if (src == dest) {
      path[0] = src;
      pathLength = 1;
      return pathLength;
   }

   Queue q = newQueue();
   QueueJoin(q, src);
   Vertex curr, i;
   int order = 1;
   int finished = 0;

   while (!QueueIsEmpty(q) && !finished) {
      curr = QueueLeave(q);

      if (visited[curr]) {
         continue;
      }

      visited[curr] = order++;

      for (i = 0; i < g->nV; i++) {
         if (g->edges[curr][i] > 0 && g->edges[curr][i] <= max && !visited[i]) {

            if (i == dest) {
               finished = 1;
            } else {
               QueueJoin(q, i);
            }
         }
      }
   }

   if (!finished) {
      return 0;
   }

   path[0] = dest;
   pathLength = 1;

   curr = dest;
   Vertex prev;
   while (curr != src) {
      prev = -1;
      for (i = 0; i < g->nV; i++) {

         if (g->edges[curr][i] > 0 && g->edges[curr][i] <= max && visited[i] && (prev == -1 || visited[i] < visited[prev])) {
            prev = i;
         }
      }

      for (i = pathLength; i > 0; i--) {
         path[i] = path[i - 1];
      }
      path[0] = prev;
      pathLength++;
      curr = prev;
   }

	return pathLength;
}












