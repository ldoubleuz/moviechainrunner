#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
  #define DBG(x) printf x
#else
  #define DBG(x)
#endif

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

struct dfs_node {
   struct arc* vs;
   int len;
   int curI;
   struct dfs_node* next;
};

struct ll_node {
   void* data;
   struct ll_node* next;
};

typedef struct arc arc;
typedef struct vertex vertex;
typedef struct graph graph;
typedef struct dfs_node dfs_node;
typedef struct ll_node ll_node;

graph* read_graph(FILE *fin);
void print_graph(graph* g);
vertex* top_sort(graph* g,graph* flipped);
graph* flip(graph* g);
void free_graph(graph* g);
ll_node* longest_path_dag(graph* g, graph* flipped);

int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"r");
   graph* g = read_graph(input);
   graph* f = flip(g);
   fclose(input);


   free_graph(g);
   free_graph(f);
}

ll_node* longest_path_dag(graph* g,graph* flipped) {
   vertex* ord = top_sort(g,flipped);

   int weights[g->numV];
   int parents[g->numV];

   for(int i=g->numV-1;i>=0;i--) {
      weights[i]=0;
      parents[i]=-1;
   }

   for(int i=0;i<g->numV;i++)
      DBG(("%i %i\t",parents[i],weights[i]));
   DBG(("\n"));
   for(int i=0;i<g->numV;i++) {
      vertex v = ord[i];
      for(int j = v.numE-1;j>=0;j--) {
         arc d = v.edges[j];
         if(weights[d.dest] <= weights[v.label] + d.len) {
            weights[d.dest] = weights[v.label] + d.len;
            parents[d.dest] = v.label;
         }
      }
      for(int i=0;i<g->numV;i++)
         DBG(("%i %i\t",parents[i],weights[i]));
      DBG(("\n"));
   }

   int max = 0, maxI = -1;
   for(int i=g->numV-1;i>=0;i--)
      if(weights[i] > max) {
         max = weights[i];
         maxI = i;
      }

   ll_node* path = NULL;
   int cur = maxI;
   while(cur != -1) {
      DBG(("%i\n",cur));
      ll_node* new_path = malloc(sizeof(struct ll_node));
      new_path->data = (void*)(long)cur;
      new_path->next = path;
      path = new_path;

      cur = parents[cur];
   }

   free(ord);
   return path;
}

vertex* top_sort(graph* g, graph* flipped) {
   vertex* order = malloc(sizeof(struct vertex) * g->numV);
   int ordI = 0;

   
   arc* sources = malloc(sizeof(struct arc) * g->numV);
   int numSources = 0;

   for(int i = g->numV-1;i>=0;i--)
      if(g->verts[i].numE == 0)
         sources[numSources++].dest = i;

   realloc(sources,sizeof(struct arc) * numSources);


   dfs_node* stack = malloc(sizeof(struct dfs_node));
   stack->vs = sources;
   stack->len = numSources;
   stack->curI = 0;
   stack->next = NULL;
   ll_node* path = NULL;

   char visited[g->numV];
   for(int i= g->numV - 1;i>=0;i--)
      visited[i] = 0;


   do {

      while(stack->curI == stack->len) {

         if(path == NULL) {
            free(stack);
            break;
         }

         DBG(("exit %i\n",(int)path->data));

         order[ordI++] = g->verts[(int)path->data];

         ll_node* new_path = path->next;
         free(path);
         path = new_path;

         dfs_node* stack_new = stack->next;
         free(stack);
         stack = stack_new;
      }

      vertex v = flipped->verts[stack->vs[stack->curI++].dest];

      if(!visited[v.label]) {
         DBG(("enter %i\n",v.label));

         visited[v.label] = 1;
         ll_node* path_head = malloc(sizeof(struct ll_node));
         path_head->data = (void*)(long)v.label;
         path_head->next = path;
         path = path_head;

         dfs_node* stack_head = malloc(sizeof(struct dfs_node));
         stack_head->vs = v.edges;
         stack_head->len = v.numE;
         stack_head->curI = 0;
         stack_head->next = stack;
         stack = stack_head;

      }else
         DBG(("touch %i\n",v.label));

   } while(path != NULL);

   for(int i=0;i<g->numV;i++)
      DBG(("%i  ",order[i].label));
   DBG(("\n"));



   free(sources);
   free_graph(flipped);
   return order;

}

void free_graph(graph* g) {
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

graph* read_graph(FILE *fin) {
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

void print_graph(graph* g) {

   for(int i=0;i<g->numV;i++) {
      
      vertex* v = g->verts + i;

      printf("v: %i   numE: %i\t",v->label,v->numE);
      for(int j=0;j<v->numE;j++)
         printf("(to %i,weight %i,flag %i)  ",
                  (v->edges + j)->dest,(v->edges +j)->len,(v->edges + j)->flags);
      printf("\n");

   }
}
