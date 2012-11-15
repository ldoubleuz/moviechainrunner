#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
  #define DBG(x) printf x
#else
  #define DBG(x)
#endif

#define FNAME "titlecycle"

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
void print_path(ll_node* p);

void free_graph(graph* g);
void free_ll(ll_node* ll,void f(void *o));

vertex* top_sort(graph* g,graph* flipped);
graph* flip(graph* g);
ll_node* longest_path_dag(graph* g, graph* flipped);
ll_node* cycles(graph* g);

int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"r");
   graph* g = read_graph(input);
   graph* f = flip(g);
   fclose(input);

   print_graph(g);

   ll_node* cyc = cycles(g);

   ll_node* cur = cyc;
   while(cur != NULL) {
      print_path(cur->data);
      cur = cur->next;
   }

   free_ll(cyc,NULL);
   free_graph(g);
   free_graph(f);
}

/*
 * Returns a list of all of the cycles in the given graph
 */
ll_node* cycles(graph* g) {

   ll_node* cycles = NULL;

   dfs_node* stack = malloc(sizeof(struct dfs_node));
   stack->vs = g->verts[g->numV-1].edges;
   stack->len = g->verts[g->numV-1].numE;
   stack->curI = 0;
   stack->next = NULL;
   ll_node* path = NULL;

   char visited[g->numV];
   for(int i= g->numV - 1;i>=0;i--)
      visited[i] = 0;

   while(1) {

      while(stack->curI == stack->len) {

         if(path == NULL) {
            free(stack);
            return cycles;
         }

         DBG(("exit %i\n",(int)path->data));

         ll_node* new_path = path->next;
         free(path);
         path = new_path;

         dfs_node* stack_new = stack->next;
         free(stack);
         stack = stack_new;
      }

      vertex v = g->verts[stack->vs[stack->curI++].dest];

      if(visited[v.label]) {
         DBG(("touch %i\n",v.label));

         ll_node* cyc_path = malloc(sizeof(struct ll_node));
         cyc_path->data = (void*)(long)v.label;
         cyc_path->next = NULL;
         
         ll_node* cur = path;
         while(cur != NULL && (long)cur->data != v.label) {
            ll_node* new_cyc_path = malloc(sizeof(struct ll_node));
            new_cyc_path->data = cur->data;
            new_cyc_path->next = cyc_path;
            cyc_path = new_cyc_path;

            cur = cur->next;
         }
         
         if(cur != NULL) {
            ll_node* new_cycles = malloc(sizeof(struct ll_node));
            new_cycles->data = cyc_path;
            new_cycles->next = cycles;
            cycles = new_cycles;
         }

      } else {
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
      }

   }
}

/*
 * Returns the longest path in the given graph. The given graph must be a DAG.
 * The reverse of the graph must also be passed to this function
 */
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

/*
 * Returns an arrry containing all of the vertices in g in topological order
 */
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


   while(1) {

      while(stack->curI == stack->len) {

         if(path == NULL) {
            free(stack);

            for(int i=0;i<g->numV;i++)
               DBG(("%i  ",order[i].label));
            DBG(("\n"));

            free(sources);
            free_graph(flipped);
            return order;
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

   }


}

/*
 * frees the given graph
 */
void free_graph(graph* g) {
   for(int i=g->numV-1;i>=0;i--)
      free(g->verts[i].edges);
   free(g->verts);
   free(g);
}

/*
 * frees the given linked list. If f is not NULL, calls f on each node's data
 */
void free_ll(ll_node* ll,void f(void *o)) {
   while(ll != NULL) {
      if(f != NULL)
         free(ll->data);

      ll_node* ll_old = ll;
      ll = ll->next;
      free(ll_old);
   }
}

/*
 * returns a graph containing the vertices of v with flipped edges
 */
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

/*
 * Returns a graph read in from the given file. The spec for the file is given
 * in Graph_format.txt
 */
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

/*
 * Prints a path
 */
void print_path(ll_node* p) {
   while(p != NULL) {
      printf("%i",(int)(long)p->data);
      if(p->next != NULL)
         printf(" -> ");
      else
         printf("\n");
      p = p->next;
   }
}

/*
 * Prints a graph
 */
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
