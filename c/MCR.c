#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>

#ifdef DEBUG
  #define DBG(x) printf x
  #define DBGP(i) printf("%lx\n",(unsigned long)(i))
#else
  #define DBG(x)
  #define DBGP(i)
#endif

#define FNAME "test_unweighted"
#define F_OUT_NAME "test_sol"
#define NUM_T 8

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

dfs_node* init_stack(graph* g, int start_v, int num_v);
short* build_path(dfs_node* stack, int len);
void* search(void* args);

void Signal(int signum, void (*handler)(int));
void usr1_handler(int sig);
void usr2_handler(int sig);

path* best_paths;
pthread_mutex_t lock;

int main(int argc, char** argv) {
   FILE *input = fopen(FNAME,"rb");
   graph* g = read_graph(input);
   fclose(input);

   Signal(SIGUSR1, usr1_handler);
   Signal(SIGUSR2, usr2_handler);
   if(pthread_mutex_init(&lock,NULL) != 0) {
      printf("reallllyyy...");
      return -1;
   }

   pthread_t tid[NUM_T];

   best_paths = malloc(sizeof(path) * NUM_T);

   int num_each = (g->numV - 1) / NUM_T;
   for(int i=0;i<NUM_T;i++) {

      void** args = malloc(sizeof(void *) * 3);
      *args = (void*)(long)i;
      *(args + 1) = g;
      *(args + 2) = init_stack(g,i*num_each,
                        (i == NUM_T - 1) ? g->numV - i*num_each - 1 : num_each);
      
      pthread_create(tid+i,NULL,search,(void*)args);
   }

   void* ans;
   for(int i=0;i<NUM_T;i++)
      pthread_join(tid[i],&ans);

   pthread_mutex_destroy(&lock);

   usr2_handler(0);

   for(int i=0;i<NUM_T;i++)
      free(best_paths[i].labels);
   free(best_paths);
   free_graph(g);
}

dfs_node* init_stack(graph* g, int start_v, int num_v) { 
   dfs_node* stack = calloc(500,sizeof(struct dfs_node));

   stack[0].v = g->verts[g->numV - 1];

   stack[0].v.numE = num_v;
   stack[0].v.edges += start_v;
   stack[0].vs_cur_i = 0;

   return stack;
}

void *search(void* args) {
   int thread_num = (int)(long)(*((void**)args));
   graph* g = *((graph**)args + 1);
   dfs_node* stack = *((dfs_node**)args + 2);
   free(args);

   int cur_i = 0;
   char search_path[g->numV];
   for(int i= g->numV - 1;i>=0;i--)
      search_path[i] = 0;

   search_path[g->numV - 1] = 1;

   path best;
   best.len = 0;
   best.labels = NULL;

   while(1) {

      while(stack[cur_i].vs_cur_i == stack[cur_i].v.numE) {

         DBG(("exit %i\n",stack[cur_i].v.label));

         if(cur_i > best.len) {
            pthread_mutex_lock(&lock);
            free(best.labels);
            best.labels = build_path(stack,cur_i);
            best.len = cur_i;

            *(best_paths + thread_num) = best;
            pthread_mutex_unlock(&lock);
         }

         search_path[stack[cur_i].v.label] = 0;
         cur_i--;

         if(cur_i == -1) {
            printf("Done %i\n",thread_num);
            free(stack);
            return NULL;
         }
      }

      short label = *stack[cur_i].v.edges;
      stack[cur_i].vs_cur_i++;
      stack[cur_i].v.edges++;

      if(!search_path[label]) {
         DBG(("enter %i\n",label));

         search_path[label] = 1;

         vertex v = g->verts[label];

         cur_i++;

         stack[cur_i].v = v;
         stack[cur_i].vs_cur_i = 0;

      }else {
         DBG(("touch %i\n",label));
      }

   }
}

void usr1_handler(int sig) {
   printf("usr1  ");
   int max = -1;
   for(int i=0;i<NUM_T;i++) {
      if(best_paths[i].len > max)
         max = best_paths[i].len;
   }
   printf("%i\n",max);

}

void usr2_handler(int sig) {
   printf("usr2\n");
   int max = -1, max_i = 0;
   for(int i=0;i<NUM_T;i++) {
      if(best_paths[i].len > max) {
         max = best_paths[i].len;
         max_i = i;
      }
   }

   FILE *output = fopen(F_OUT_NAME,"w");
   pthread_mutex_lock(&lock);
   path p = best_paths[max_i];

   for(int i=0;i<p.len;i++) {
      fprintf(output,"%i",p.labels[i]);
      if(i < p.len - 1)
         fprintf(output,",");
   }
   pthread_mutex_unlock(&lock);

   fclose(output);

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
   char* orig_data = data;

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

   free(orig_data);

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

void Signal(int signum, void (*handler)(int)) {
   struct sigaction action, old_action;

   action.sa_handler = handler;
   sigemptyset(&action.sa_mask);
   action.sa_flags = SA_RESTART;

   if(sigaction(signum,&action,&old_action) < 0)
      printf("realllllyyyy?????");
}
