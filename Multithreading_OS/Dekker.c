#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

typedef struct zajPodaci {

    int A;
    atomic_int Z[2]; //zastavice
    atomic_int PRAVO;

}zajPodaci;

int IdSegment; /* identifikacijski broj segmenta */
int M;
struct zajPodaci *podatak;

 
void funkcija(int glDretva) {
    int i;

    int sporednaDretva = 1 - glDretva;


    (*podatak).Z[glDretva] = 1;
    while ((*podatak).Z[sporednaDretva] != 0){
        if ((*podatak).PRAVO != glDretva) { 
            (*podatak).Z[glDretva] = 0;
            while ((*podatak).PRAVO != glDretva); 
            (*podatak).Z[glDretva] = 1;
        } 
    }
    //K.O.
    i = (*podatak).A;
    printf("Dretva: %d povecava A na %d\n", glDretva, ++i);
    (*podatak).A = i; 
    sleep(1);
    //
    
    (*podatak).PRAVO = sporednaDretva;
    (*podatak).Z[glDretva] = 0;
    //N.K.O.;
    



    //printf("Procitano %d\n", i);
    

    return;
}


int main(int argc, char *argv[])
{
    M = atoi(argv[1]);

   /* zauzimanje zajedničke memorije */
   IdSegment = shmget(IPC_PRIVATE, sizeof(zajPodaci), 0600);
 
   if (IdSegment == -1) {
      exit(1);  /* greška - nema zajedničke memorije */
   }
 
   podatak = (zajPodaci *) shmat(IdSegment, NULL, 0);

   (*podatak).A = 0;
    podatak->Z[0] = 0;
    podatak->Z[1] = 0;
   (*podatak).PRAVO = 0; 


 
   /* pokretanje paralelnih procesa */
   if (fork() == 0) {
    for (int k=0; k < M; k++) {
        funkcija(0);
    }
    exit(0);
   } else {
    for (int k=0; k < M; k++) {
        funkcija(1);
    }
   }

   wait(NULL);
   printf("Vrijednost A: %d\n", (*podatak).A);
   
   (void) shmdt((char *) podatak);
   (void) shmctl(IdSegment, IPC_RMID, NULL);
 
   return 0;
}