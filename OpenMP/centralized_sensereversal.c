#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#define nthreads 4

bool globalSense;
bool *localSense;
int startcount=nthreads;
int P, N;

int FetchAndDecrementCount();


void SenseReversalBarrier_Init()
{
  localSense = (bool*) malloc(sizeof(bool)*(P));
  int i;
  for (i = 0; i < P; ++i) 
    localSense[i] = true;
  globalSense = true;

}


void SenseReversalBarrier(int thread_num) 
{
  localSense[thread_num] = !localSense[thread_num]; // Toggle private sense variable
  if (FetchAndDecrementCount() == 1)
  { 
    startcount = P;
    printf("%d",globalSense = localSense[thread_num]);
  }

  else
  {
    while (globalSense != localSense[thread_num]) { } // Spin
  }
}    
    
//Gets the current count and decrements it. Also returns the current count
int FetchAndDecrementCount()
{ 

  int myCount;
  #pragma omp critical
  {
    myCount = startcount;
    startcount--;
  }
  return myCount;

}


int main(int argc, char **argv)
{
  int thread_num = -1;
  if (argc==3){
        if (sscanf (argv[1], "%d", &P)!=1) printf ("P - not an integer\n");
        if (sscanf (argv[2], "%d", &N)!=1) printf ("N - not an integer\n");
  }
  else {P = 4; N = 1000;}
  struct timeval tv1, tv2;
  double total_time;
  SenseReversalBarrier_Init();

  #pragma omp parallel num_threads(P) firstprivate(thread_num) firstprivate(N)
  {
    int i;
    thread_num = omp_get_thread_num();
    gettimeofday(&tv1, NULL);
    for (i = 0; i < N; ++i)
    {
      printf("\nThread %d entered barrier",thread_num);
      SenseReversalBarrier(thread_num);
      printf("\nThread %d entered barrier",thread_num);
      SenseReversalBarrier(thread_num);
      printf("\nThread %d entered barrier",thread_num);
      SenseReversalBarrier(thread_num);
      printf("\nThread %d entered barrier",thread_num);
      SenseReversalBarrier(thread_num);
    }
    gettimeofday(&tv2, NULL);
  }
  total_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec)*1000000;
  printf("\nSUMMARY:\nTotal run-time for %d "
            "loops with 5 barriers per loop: %fs\n"
            "The average time per barrier: %fus\n",
            N, total_time/1000000, (double)(total_time/(N*5)));
  return 0;
}