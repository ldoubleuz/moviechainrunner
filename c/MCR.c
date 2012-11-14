#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNAME "smalltitles"

struct arc {
   short dest;
   char len;
   char flags;
};

struct vertex {
   short label;
   short numE;
   struct arc* edges;
};

struct graph {
   short numV;
   struct vertex* verts;
};

typedef struct arc arc;
typedef struct vertex vertex;
typedef struct graph graph;

graph* readGraph(FILE *fin);
void printGraph(graph* g);
vertex* topSort(graph* g);
graph* flip(graph* g);
void freeGraph(graph* g);

int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"r");
   graph* g = readGraph(input);
   graph* f = flip(g);

   printGraph(g);
   printGraph(f);

   fclose(input);
   free(g);
   free(f);
   

}

vertex* topSort(graph* g) {
   //TODO
}

void freeGraph(graph* g) {
   for(int i=g->numV-1;i>=0;i--)
      free(g->verts[i].edges);
   free(g->verts);
   free(g);
}

graph* flip(graph* g) {
   int numIn[g->numV];
   for(int i=g->numV - 1; i >= 0; i--)
      numIn[i] = 0;

   for(int i=0;i<g->numV;i++) {
      vertex v = g->verts[i];
      for(int j=0;j<v.numE;j++)
         numIn[v.edges[j].dest]++;
   }

   graph* flipped = malloc(sizeof(struct graph));

   flipped->numV = g->numV;
   flipped->verts = malloc(flipped->numV * sizeof(struct vertex));

   for(int i=0;i<g->numV;i++) {
      flipped->verts[i].label = i;
      flipped->verts[i].numE = numIn[i];
      flipped->verts[i].edges = calloc(numIn[i],sizeof(struct arc));
      numIn[i] = 0;
   }

   for(int i=0;i<g->numV;i++) {
      short vl = g->verts[i].label;
      for(int j=0;j<g->verts[i].numE;j++) {
         short old_dest_num = g->verts[i].edges[j].dest;
         flipped->verts[old_dest_num].edges[numIn[old_dest_num]].dest = vl;
         numIn[old_dest_num]++;
      }
   }

   return flipped;
}

graph* readGraph(FILE *fin) {
   fseek(fin,0L,SEEK_END);
   int sz = ftell(fin);
   fseek(fin,0L,SEEK_SET);

   char *data = malloc(sz);
   fgets(data,sz,fin);


   graph* g = malloc(sizeof(graph));
      
   g->numV = data[0] | (data[1] << 8);
   g->verts = calloc(g->numV,sizeof(vertex));

   data += 2;
   for(int i=0;i < g->numV; i++) {
      vertex* v = g->verts + i;

      memmove(v,data,4);
      data+=4;
      int edgesBytes = sizeof(arc) * v->numE;
      v->edges = malloc(edgesBytes);
      memmove(v->edges,data,edgesBytes);
      data+=edgesBytes;
   }

   free(data);

   return g;
}

void printGraph(graph* g) {

   for(int i=0;i<g->numV;i++) {
      
      vertex* v = g->verts + i;

      printf("v: %i   numE: %i\t",v->label,v->numE);
      for(int j=0;j<v->numE;j++)
         printf("(to %i,weight %i,flag %i)  ",
                  (v->edges + j)->dest,(v->edges +j)->len,(v->edges + j)->flags);
      printf("\n");

   }
}
