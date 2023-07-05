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

int N, M, A;
atomic_int *ulaz, *broj;



int indexOfMax(int myArray[], int size) {
    int i;
    int maxValue = myArray[0];
    int maxIndex =0;

    for (i = 1; i < size; ++i) {
        if ( myArray[i] > maxValue ) {
            maxValue = myArray[i];
            maxIndex = i;
        }
    }
    return maxIndex;
}

void *funkcija(void *brDretve) {

    for (int s=0; s < M; s++) {
           
        ulaz[*(int*)(brDretve)] = 1;
        broj[*(int*)(brDretve)] = broj[indexOfMax(broj, N)] + 1; 
        broj[indexOfMax(broj, N)] = broj[*(int*)(brDretve)]; 
        ulaz[*(int*)(brDretve)] = 0;
        for (int j = 0; j < N; j++) {
            while (ulaz[j] == 1);
            while ( (broj[j] != 0) && (broj[j] < broj[*(int*)(brDretve)] || (broj[j] == broj[*(int*)(brDretve)] && j < *(int*)(brDretve)))); 
        }
        //K.O.
        printf("Dretva: %d povecava A na %d\n", *(int*)(brDretve), ++A);
        sleep(1);
        // 
        broj[*(int*)(brDretve)] = 0;
        //N.K.O.

    }
}

int main(int argc, char *argv[]) {

    N = atoi(argv[1]);  //broj dretvi
    M = atoi(argv[2]);  //povećanje u svakoj

    pthread_t thr_id[N];
    int polje[N];
    for (int k=0; k < N; k++) {
        polje[k] = k;
    }

    //dijeljena memorija
    int *memorija;
    memorija = malloc(sizeof(int)*(1+N+N) + N*sizeof(pthread_t));

    A = memorija;
    ulaz = (memorija + 1);
    broj = (memorija + 1 + N);

    //inicijalizacija
    A = 0;
    for (int k=0; k < N; k++) {
        ulaz[k] = broj[k] = 0;
    }


    for (int k=0; k < N; k++) {
        /* pokretanje dretvi */
        if (pthread_create(&thr_id[0], NULL, funkcija, &polje[k]) != 0) {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        } 
    }

    for (int k=0; k < N; k++) {
        pthread_join(thr_id[k], NULL);
    }
    printf("Vrijednost A: %d\n", A);

    //delete(memorija);



    return 0;
}