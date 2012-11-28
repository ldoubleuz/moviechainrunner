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

#define FNAME "titlecycle"

struct arc {
   short src;
   short dest;
   short len;
   short cc_i;
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

struct cycle_counter_node {
   short src;
   short dest;
   short g_cc_i;
   short f_cc_i;
};

struct arc_info {
   short num_cut_cycles;
   short curI;
};

struct cycle_counter {
   struct ll_node* counter;
   struct arc_info* g_info;
   struct arc_info* f_info;
   short g_info_len;
   short f_info_len;
};


typedef struct arc arc;
typedef struct vertex vertex;
typedef struct graph graph;
typedef struct dfs_node dfs_node;
typedef struct ll_node ll_node;
typedef struct cycle_counter_node cc_node;
typedef struct arc_info arc_info;
typedef struct cycle_counter* cycle_counter;

graph* read_graph(FILE *fin);

void print_graph(graph* g);
void print_path(ll_node* p);
void print_cycle_counter(cycle_counter p);

void free_graph(graph* g);
void free_ll(ll_node* ll);
void free_cycle_counter(cycle_counter ll);

vertex* top_sort(graph* g,graph* flipped);
graph* flip(graph* g);
ll_node* longest_path_dag(graph* g, graph* flipped,int* len_dest);
ll_node* cycles(graph* g);
cycle_counter build_cycle_counter(ll_node* cycles, graph* g, graph* f);
void init_cut_cycles(cycle_counter cc, graph* g, graph* f);
void delete_edge(arc_info* cc_data, vertex* v, short cci);
void fix_edge(arc_info* cc_data, vertex* v, short cci);
ll_node* longest_path_da(graph* g,int* len_dest);
int inc_cycles(cycle_counter cc, graph* g, graph* f);
int compute_path_len(graph* g, ll_node* p);


int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"r");
   graph* g = read_graph(input);
   fclose(input);

   int path_len;
   ll_node* path = longest_path_da(g,&path_len);
   print_path(path);
   printf("%i\n",path_len);

   /*
   graph* f = flip(g);

   ll_node* cyc = cycles(g);
   print_graph(g);
   printf("\n\n");
   print_graph(f);
   printf("\n\n");

   cycle_counter counter = build_cycle_counter(cyc,g,f);

   print_cycle_counter(counter);
   init_cut_cycles(counter, g, f);
   print_cycle_counter(counter);
   print_graph(g);
   printf("\n\n");
   print_graph(f);
   printf("\n\n");

   inc_cycles(counter,g,f);
   print_graph(g);
   printf("\n\n");
   print_graph(f);
   */

   /*

   print_cycle_counter(cycle_counter);

   ll_node* cur = cyc;
   while(cur != NULL) {
      print_path(cur->data);
      cur = cur->next;
   }

   free_ll(cyc);
   free_graph(g);
   free_graph(f);
   */
}

/*
 * Computes the longest path on a directed graph
 */
ll_node* longest_path_da(graph* g,int* len_dest) {

   graph* f = flip(g);
   ll_node* cyc = cycles(g);
   cycle_counter counter = build_cycle_counter(cyc,g,f);
   init_cut_cycles(counter,g,f);



   int best_len = -1;
   ll_node* best_path = NULL;

   do {
      printf("g: \n");
      print_graph(g);
      printf("\nf: \n");
      print_graph(f);
      printf("\ncycle_counter: \n");
      print_cycle_counter(counter);
      printf("\n\n");

      int new_len;
      ll_node* new_path = longest_path_dag(g,f,&new_len);

      if(new_len > best_len) {
         free_ll(best_path);
         best_len = new_len;
         best_path = new_path;
      }
   } while(inc_cycles(counter,g,f));

   *len_dest = best_len;

   free_cycle_counter(counter);
   free_ll(cyc);
   free_graph(f);

   return best_path;
}

/*
 * Cuts the initial edges from the cycle counter. Assumes that the NULL node in
 * each of the cycle's loops is not the initial node
 */
void init_cut_cycles(cycle_counter counter, graph* g, graph* f) {
   ll_node* cc = counter->counter;
   while(cc != NULL) {
      cc_node n = *(cc_node*)((ll_node*)cc->data)->data;

      delete_edge(counter->g_info,g->verts + n.src,n.g_cc_i);
      delete_edge(counter->f_info,f->verts + n.dest,n.f_cc_i);

      cc = cc->next;
   }
}

/*
 * Adds another cut to the edge. Deletes it if that's the first cut
 */
inline void delete_edge(arc_info* cc_data, vertex* v, short cci) {
   arc_info* ai = cc_data+cci;

   if(ai->num_cut_cycles == 0) {
      arc temp = v->edges[ai->curI];
      v->numE--;
      v->edges[ai->curI] = v->edges[v->numE];
      cc_data[v->edges[ai->curI].cc_i].curI = ai->curI;
      ai->curI = v->numE;
      v->edges[v->numE] = temp;
   }

   ai->num_cut_cycles++;
}

/*
 * Fixes one cut of an edge. Note that it isn't necessarily added back to the 
 * graph if there are multiple cuts
 */
inline void fix_edge(arc_info* cc_data, vertex* v, short cci) {
   arc_info* ai = cc_data + cci;

   if(ai->num_cut_cycles == 1) {
      if(ai->curI > v->numE) {
         arc temp = v->edges[ai->curI];
         v->edges[ai->curI] = v->edges[v->numE];
         cc_data[v->edges[ai->curI].cc_i].curI = ai->curI;
         ai->curI = v->numE;
         v->edges[v->numE] = temp;

         v->numE++;
      } else { //Just expand the number of edges if numE == curI
         ai->num_cut_cycles = 0;
         v->numE++;
      }

   } else {
      ai->num_cut_cycles--;
   }
}

/*
 * Increments which cycles are cut. If all combinations have been tried, fixes
 * the graph and returns 0. Otherwise returns 1.
 */
int inc_cycles(cycle_counter counter, graph* g, graph* f) {

   ll_node* cc = counter->counter;
   int carry = 0;
   do {

      ll_node* cycle = (ll_node*)cc->data;
      cc_node n = *(cc_node*)cycle->data;

      fix_edge(counter->g_info,g->verts + n.src,n.g_cc_i);
      fix_edge(counter->f_info,f->verts + n.dest,n.f_cc_i);

      if(cycle->next->data == NULL) {
         carry = 1;
         cc->data = cycle->next->next;
         n = *(cc_node*)cycle->next->next->data;
      } else {
         cc->data = cycle->next;
         n = *(cc_node*)cycle->next->data;
      }

      DBGP(1);
      DBGP(2);
      DBGP(3);
      delete_edge(counter->g_info,g->verts + n.src,n.g_cc_i);
      delete_edge(counter->f_info,f->verts + n.dest,n.f_cc_i);

      cc = cc->next;

   } while(carry == 1 && cc != NULL);

   return carry != 1;

}

/*
 * Builds a cycle counter. A cycle counter has one counter for each cylce in
 * the given list. A counter consists of a linked-list loop where the data
 * is points to the edges of the cycles in the given graph. Each counter has
 * a special node with NULL data that serves as a marker for completing a cycle.
 */
cycle_counter build_cycle_counter(ll_node* cycles, graph* g, graph* f) {
   
   cycle_counter cc = malloc(sizeof(struct cycle_counter));

   ll_node* counter = NULL;
   
   int g_edge_i = 0;
   int f_edge_i = 0;
   while(cycles != NULL) {
      ll_node* cycle = cycles->data;
      short first_l = (short)(long)cycle->data;

      ll_node* loop_start = malloc(sizeof(struct ll_node));
      loop_start->data = NULL;
      loop_start->next = NULL;

      ll_node* loop = loop_start;
      while(cycle != NULL) {

         short u = (short)(long)cycle->data;
         short v = (cycle->next==NULL) ? first_l:(short)(long)cycle->next->data;

         arc* gedge = g->verts[u].edges;
         while(gedge->dest != v)
            gedge += 1;

         arc* fedge = f->verts[v].edges;
         while(fedge->dest != u)
            fedge += 1;

         if(gedge->cc_i == -1)
            gedge->cc_i = g_edge_i++;

         if(fedge->cc_i == -1)
            fedge->cc_i = f_edge_i++;

         cc_node* new_cc_node = malloc(sizeof(struct cycle_counter_node));
         new_cc_node->src = u;
         new_cc_node->dest = v;
         new_cc_node->g_cc_i = gedge->cc_i;
         new_cc_node->f_cc_i = fedge->cc_i;

         ll_node* new_loop = malloc(sizeof(struct ll_node));
         new_loop->data = new_cc_node;
         loop->next = new_loop;
         loop = new_loop;

         cycle = cycle->next;
      }

      loop->next = loop_start;

      ll_node* new_counter_node = malloc(sizeof(struct ll_node));
      new_counter_node->data = loop_start->next;
      new_counter_node->next = counter;
      counter = new_counter_node;

      cycles = cycles->next;
   }

   cc->counter = counter;
   cc->g_info = malloc(g_edge_i * sizeof(struct arc_info));
   cc->f_info = malloc(f_edge_i * sizeof(struct arc_info));
   cc->g_info_len = g_edge_i;
   cc->f_info_len = f_edge_i;


   for(int i = 0;i<g->numV;i++) {
      vertex v = g->verts[i];
      for(int j = 0;j<v.numE; j++) {
         arc u = v.edges[j];
         if(u.cc_i != -1) {
            arc_info* a = cc->g_info + u.cc_i;
            a->curI = j;
            a->num_cut_cycles = 0;
         }
      }
   }
   
   for(int i = 0;i<f->numV;i++) {
      vertex v = f->verts[i];
      for(int j = 0;j<v.numE; j++) {
         arc u = v.edges[j];
         if(u.cc_i != -1) {
            arc_info* a = cc->f_info + u.cc_i;
            a->curI = j;
            a->num_cut_cycles = 0;
         }
      }
   }

   return cc;
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
ll_node* longest_path_dag(graph* g,graph* flipped,int* len_dest) {
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

   *len_dest = max;
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
 * frees the given linked list. Does not touch the data
 */
void free_ll(ll_node* ll) {
   while(ll != NULL) {
      ll_node* ll_old = ll;
      ll = ll->next;
      free(ll_old);
   }
}

/*
 * frees a cycle counter structure. Doesn't touch any of the edges.
 */
void free_cycle_counter(cycle_counter c_counter) {

   ll_node* cc = c_counter->counter;
   while(cc != NULL) {
      
      ll_node* c = cc->data;
      while(c != NULL) {
         ll_node* new_c = (c->data == NULL) ? NULL : c->next;
         free(c);
         c = new_c;
      }
      ll_node* new_cc = cc->next;
      free(cc);
      cc = new_cc;
   }

   free(c_counter->g_info);
   free(c_counter->f_info);
   free(c_counter);
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
         arc* a = &flipped->verts[old_dest_num].edges[numIn[old_dest_num]];
         a->src = old_dest_num;
         a->dest = vl;
         a->cc_i = -1;
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
      v->edges = calloc(v->numE,sizeof(arc));
      arc* edge = v->edges;
      for(int j=0;j<v->numE;j++) {
         edge->src = v->label;
         edge->dest = *(short*)data;
         edge->cc_i = -1;
         edge->len = *(char*)(data+2);
         DBG(("(%i -> %i)\n",edge->src,edge->dest));
         DBG(("%lx\n",(unsigned long)edge));
         data+=4;
         edge+=1;
      }
   }

   free(data);

   return g;
}

void print_cycle_counter(cycle_counter c_counter) {

   ll_node* cc = c_counter->counter;
   while(cc != NULL) {
      ll_node* cyc = cc->data;
      ll_node* first = cc->data;
      int count = 0;

      while(cyc != first || count == 0) {
         if(cyc->data != NULL) {
            cc_node cc = *(cc_node*)cyc->data;
            printf("{(%i -> %i),%i %i} ",cc.src,cc.dest,cc.g_cc_i,cc.f_cc_i);
         } else {
            printf("mark ");
         }
         count++;
         cyc = cyc->next;
      }
      printf("\n");

      cc = cc->next;
   }

   arc_info* gai = c_counter->g_info;
   for(int i=0;i<c_counter->g_info_len;i++)
      printf("(%i, %i) ",gai[i].curI,gai[i].num_cut_cycles);
   printf("\n");

   arc_info* fai = c_counter->f_info;
   for(int i=0;i<c_counter->f_info_len;i++)
      printf("(%i, %i) ",fai[i].curI,fai[i].num_cut_cycles);
   printf("\n");
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

      printf("v: %i   numE: %i",v->label,v->numE);
      for(int j=0;j<v->numE;j++) {
         if(j != 0) printf("\t\t"); else printf("\t");
         printf("(from %i, to %i, cc_i %i, weight %i)\n",
                  (v->edges + j)->src,(v->edges +j)->dest,
                  (v->edges + j)->cc_i ,(v->edges + j)->len);
      }
      if(v->numE == 0)
         printf("\n");

   }
}
