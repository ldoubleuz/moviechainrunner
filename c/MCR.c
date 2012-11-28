#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
  #define DBG(x) printf x
  #define DBGP(i) printf("%lx\n",(unsigned long)(i))
#else
  #define DBG(x)
  #define DBGP(i)
#endif

#define FNAME "cleaned_unweighted"

struct vertex {
   short label;
   short numE;
   short* edges;
};

struct graph {
   short numV;
   struct vertex* verts;
};

struct dfs_node {
   struct vertex v;
   short vs_cur_i;
};

struct path {
   short len;
   short* labels;
};

typedef struct arc arc;
typedef struct vertex vertex;
typedef struct graph graph;
typedef struct dfs_node dfs_node;
typedef struct path path;

graph* read_graph(FILE *fin);

void print_graph(graph* g);
void print_path(path* p);

void free_graph(graph* g);
void free_path(path* p);

short* build_path(dfs_node* stack, int len);
path* search(graph* g);


int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"rb");
   graph* g = read_graph(input);
   fclose(input);

   path* best = search(g);
   print_path(best);

   free_path(best);
   free_graph(g);
}

path* search(graph* g) {

   dfs_node* stack = calloc(500,sizeof(struct dfs_node));
   int cur_i = 0;

   stack[cur_i].v = g->verts[g->numV - 1];
   stack[cur_i].vs_cur_i = 0;

   char search_path[g->numV];
   for(int i= g->numV - 1;i>=0;i--)
      search_path[i] = 0;

   search_path[g->numV - 1] = 1;

   path best;
   best.len = 0;
   best.labels = NULL;


   while(1) {

      while(stack[cur_i].vs_cur_i == stack[cur_i].v.numE) {

         DBG(("exit %i\n",(int)path->data));

         if(cur_i > best.len) {
            free(best.labels);
            best.labels = build_path(stack,cur_i);
            best.len = cur_i;
            printf("%i\n",cur_i);
         }

         search_path[stack[cur_i].v.label] = 0;
         cur_i--;

         if(cur_i == 0) {
            path* p = malloc(sizeof(struct path));
            p->len = best.len;
            p->labels = best.labels;

            free(stack);
            return p;
         }
      }

      short label = *stack[cur_i].v.edges;
      stack[cur_i].vs_cur_i++;
      stack[cur_i].v.edges++;

      if(!search_path[label]) {
         DBG(("enter %i\n",v.label));

         search_path[label] = 1;

         vertex v = g->verts[label];

         cur_i++;

         stack[cur_i].v = v;
         stack[cur_i].vs_cur_i = 0;

      }else {
         DBG(("touch %i\n",v.label));
      }

   }
}

short* build_path(dfs_node* stack, int len) {
   short* path = malloc(sizeof(short) * len);

   for(int i=1;i<=len;i++)
      path[i-1] = stack[i].v.label;

   return path;
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
 * frees a path
 */
void free_path(path* p) {
   free(p->labels);
   free(p);
}

/*
 * Returns a graph read in from the given file. The spec for the file is given
 * in Graph_format.txt
 */
graph* read_graph(FILE *fin) {

   char *data = malloc(100);
   int ind=0, c, len = 100;

   while((c = getc(fin)) != EOF) {
      data[ind++] = c;
      if(ind == len) {
         len *= 2;
         data = realloc(data,len);
      }
   }

   data = realloc(data,ind);

   graph* g = malloc(sizeof(graph));
      
   g->numV = (0xff & data[0]) | (0xff00 & (data[1] << 8));
   g->verts = calloc(g->numV,sizeof(vertex));

   int offset = 2;

   data += 2;
   for(int i=0;i < g->numV; i++) {
      vertex* v = g->verts + i;

      memmove(v,data,4);
      data+=4;
      v->edges = malloc(v->numE * sizeof(short));
      memmove(v->edges,data,sizeof(short) * v->numE);
      data+=sizeof(short) * v->numE;

      offset += (sizeof(short) * v->numE + 4);
   }

   free(data);

   return g;
}

/*
 * Prints a path
 */
void print_path(path* p) {
   for(int i=0;i<p->len;i++) {
      printf("%i",p->labels[i]);
      if(i < p->len - 1)
         printf(",");
   }
}

/*
 * Prints a graph
 */
void print_graph(graph* g) {

   for(int i=0;i<g->numV;i++) {
      
      vertex* v = g->verts + i;

      printf("v: %i numE: %i  {",v->label,v->numE);
      for(int j=0;j<v->numE;j++) {
         if(j != 0) printf(", ");
         printf("%i",v->edges[j]);
      }
      printf("}\n");

   }
}
