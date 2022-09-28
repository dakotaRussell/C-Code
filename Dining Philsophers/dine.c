#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

#define NUM_PHILOSOPHERS 3
#define CHANGING 0
#define EATING 1
#define THINKING 2

typedef struct philosopher {
   pthread_t pthread;
   int state;
} philosopher;
   
static philosopher phil[NUM_PHILOSOPHERS]; /*array for each philosopher*/
static pthread_mutex_t forks[NUM_PHILOSOPHERS]; /*array for forks*/
static pthread_mutex_t print;
static int forksUsed[NUM_PHILOSOPHERS];

void *printHeader(int thread) {
   printf("|      %C      ", thread + 'A');
   return NULL;
}

void printState(int i) {
   int j, k;
   for (k=0; k<NUM_PHILOSOPHERS; k++) {
      printf("| ");
      for (j = 0; i<NUM_PHILOSOPHERS; i++) {
         if (i != NUM_PHILOSOPHERS-1) {
            if (forksUsed[j] == 1 && (j == i || j == 0)) {
               printf("%d", j);
            }
            else {
               printf("-");
            }
         }
         else {
            if (forksUsed[j] == 1 && (j == i || j == (i+1))) {
               printf("%d", j);
            }
            else {
               printf("-");
            }
         }
      }
      printf(" ");
      if (phil[i].state == CHANGING) {
         printf("      ");
      }
      else if (phil[i].state == EATING) {
         printf("Eat   ");
      }
      else if (phil[i].state == THINKING) {
         printf("Think ");
      }
   }
   printf("|\n");

}

void dawdle() {
   struct timespec tv;
   int msec = (int)(((double)random()/INT_MAX) * 1000);
   tv.tv_sec = 0;
   tv.tv_nsec = 1000000*msec;
   if (-1 == nanosleep(&tv,NULL)) {
      perror("nanosleep");
   }
}

void *philosopherFunc(int i, int numRuns) {
   int j = 0; 
   while (j < numRuns) {
      if (i % 2) { /* odd philosopher */
         pthread_mutex_lock(&forks[i]); /*pick up left fork*/
         forksUsed[i] = 1; 

         pthread_mutex_lock(&print);
         printState(i);        /*lock the printer and printing the state*/
         pthread_mutex_unlock(&print);

         if (i == NUM_PHILOSOPHERS - 1) {
            pthread_mutex_lock(&forks[0]);
            forksUsed[0] = 1;
         }
         else {
            pthread_mutex_lock(&forks[i+1]);
            forksUsed[i+1] = 1;
         }

         pthread_mutex_lock(&print);
         printState(i);
         pthread_mutex_unlock(&print);
      }   
      else { /* even philosopher */
         if (i == NUM_PHILOSOPHERS - 1) { 
            pthread_mutex_lock(&forks[0]);
            forksUsed[0] = 0;
         }
         else {
            pthread_mutex_lock(&forks[i+1]);  /*pick up right fork*/
            forksUsed[i+1] = 0; 
         } 

         pthread_mutex_lock(&print);
         printState(i);        /*lock the printer and printing the state*/
         pthread_mutex_unlock(&print);

         pthread_mutex_lock(&forks[i]); /*pick up left fork*/
         forksUsed[i] = 1; 

         pthread_mutex_lock(&print);
         printState(i);
         pthread_mutex_unlock(&print);
      }

      phil[i].state = EATING;

      pthread_mutex_lock(&print);
      printState(i);
      pthread_mutex_unlock(&print);

      dawdle(); /*spend time eating*/
   
      phil[i].state = CHANGING;

      pthread_mutex_unlock(&forks[i]);
      forksUsed[i] = 0; 

      pthread_mutex_lock(&print);
      printState(i); 
      pthread_mutex_unlock(&print);
   
      if (i == NUM_PHILOSOPHERS - 1) {
         pthread_mutex_unlock(&forks[0]);
         forksUsed[0] = 0;
      }
      else {
         pthread_mutex_unlock(&forks[i+1]);
         forksUsed[i+1] = 0;
      }

      pthread_mutex_lock(&print);
      printState(i);
      pthread_mutex_unlock(&print);

      phil[i].state = THINKING;

      j++;      
   }
   return NULL;
}

int main(int argc, char *argv[]) {
   int i;
   int numCycle = 1; /*the number of times a philosopher will eat-think*/
   pthread_mutex_init(&print, NULL); 

   if (argc > 1) {
      numCycle = *argv[1]; /*accepting option command line argument for 
                            number of cycles that each philosopher will eat_think*/
   }

   for (i=0; i<NUM_PHILOSOPHERS; i++) {
      printf("|=============");
   }
   printf("|\n");

   for (i=0; i<NUM_PHILOSOPHERS; i++) {
      printHeader(i);
   }
   printf("|\n");
   
   for (i=0; i<NUM_PHILOSOPHERS; i++) {
   printf("|=============");
   }

   printf("|\n");

   
   /*initializing the three arrays */
   for (i=0; i<NUM_PHILOSOPHERS; i++) {
      phil[i].state = 0;
      pthread_mutex_init(&forks[i], NULL);
      forksUsed[i] = 0; 
   }


   for (i=0; i<NUM_PHILOSOPHERS; i++) {
      int res;
      res = pthread_create(&phil[i].pthread, /* where to put the identifier      */
                           NULL,        /* don't set any special properties */
                           philosopherFunc(i, numCycle),       /* call the function philosopherFunc        */
                           (void *)(&forks[i])) ;

      if (-1 == res) {          /*  there was an error *//* report the error condition */
            fprintf(stderr,"Child %i: %s\n",i,strerror(errno));
            exit(-1);                 /* bail out. */
      }
   }



   /*initializing the three arrays */
  for (i=0; i<NUM_PHILOSOPHERS; i++) {
      phil[i].state = 0;
      pthread_mutex_init(&forks[i], NULL); 
   }

   i = 0;  

   while (i < NUM_PHILOSOPHERS) {
       if (phil[i].pthread)
         pthread_join(phil[i].pthread, NULL);
       i++;
   }  

   return 0;   
}
