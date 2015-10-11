#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#define nthreads 4

bool globalSense;
bool *localSense;
int startcount=nthreads;

int FetchAndDecrementCount();


void SenseReversalBarrier_Init()
{
  localSense = (bool*) malloc(sizeof(bool)*(nthreads));
  int i;
  for (i = 0; i < nthreads; ++i) 
    localSense[i] = true;
  globalSense = true;

}


void SenseReversalBarrier(int thread_num) 
{
  localSense[thread_num] = !localSense[thread_num]; // Toggle private sense variable
  if (FetchAndDecrementCount() == 1)
  { 
    startcount = nthreads;
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
  int thread_num = -1, x = 3;
  SenseReversalBarrier_Init();

  #pragma omp parallel num_threads(nthreads) firstprivate(thread_num) firstprivate(x)
  {

    int i;
    thread_num = omp_get_thread_num();
    for (i = 0; i < x; ++i)
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
  }

  return 0;
}