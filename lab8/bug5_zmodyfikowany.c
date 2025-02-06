/******************************************************************************
* FILE: bug5.c
* DESCRIPTION:
*   A simple pthreads program that dies before the threads can do their
*   work. Figure out why.
* AUTHOR: 07/06/05 Blaise Barney
* LAST REVISED: 07/11/12
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 5

pthread_barrier_t barrier;  

void *PrintHello(void *threadid)
{
   int i;
   double myresult = 0.0;

   printf("thread=%ld: waiting at the barrier...\n", (long)threadid);
   pthread_barrier_wait(&barrier); 
	printf("thread=%ld: starting calculations...\n", (long)threadid);
   for (i = 0; i < 10; i++) 
      myresult += sin(i) * tan(i);

   printf("thread=%ld result=%e. Done.\n", (long)threadid, myresult);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   int rc;
   long t;

   pthread_barrier_init(&barrier, NULL, NUM_THREADS);

   for (t = 0; t < NUM_THREADS; t++)
   {
      printf("Main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
      if (rc)
      {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
   }

   for (t = 0; t < NUM_THREADS; t++)
   {
      pthread_join(threads[t], NULL); 
   }

   pthread_barrier_destroy(&barrier);

   printf("Main: Done.\n");
   return 0;
}

