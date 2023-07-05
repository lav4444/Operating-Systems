#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#define _GNU_SOURCE
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
#include <time.h>
#include <semaphore.h>

    sem_t *bSemOTVORENO;
    int *otvoreno;
    sem_t *oSemBR_MJESTA;
    int *br_klijenataTrenutno; //broj klijenata u salonu
    sem_t *SemRadi;
    int *primljenoKlijenta;  //NEPOTREBNO                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
    sem_t *bSemSpavanje;
    int *redniBrKlijent;
    int *krajRadnogVremena;


int radnoVrijeme = 20; //koliko sekundi ce frizerka raditi
int mjestaCekaonica; //br mjesta u cekaonici

void frizerka() {
    //otvaranje salona
    printf("Frizerka: Otvaram salon\n");

    sem_wait(bSemOTVORENO);
    *otvoreno = 1;
    printf("Frizerka: Postavljam znak OTVORENO\n");

    while(1) {
        if((*krajRadnogVremena == 1)&& (*otvoreno == 1)) {

            //budi frizerku ako spava
            // sem_wait(bSemSpavanje);
            // if (*SPAVA == 1) {
            // }
            sem_post(bSemSpavanje); 

            //sem_wait(bSemOTVORENO);  bbb
            printf("Frizerka: Postavljam znak ZATVORENO\n");
            *otvoreno = 0; //zatvaranje frizerskog salona
            //sem_post(bSemOTVORENO);  bbbb
        }
        sem_post(bSemOTVORENO);
        int trKlijen;
        //sem_wait(bSemBroj);   bbb
        trKlijen = *br_klijenataTrenutno;
        //sem_post(bSemBroj);   bbbb
        sem_wait(bSemOTVORENO);
        if (*br_klijenataTrenutno > 0) {
            //uzmi prvog klijenta iz čekaonice //semafor
            //printf("Kontola gore\n");
            sem_post(bSemOTVORENO);
            sem_wait(bSemSpavanje);
            sem_post(oSemBR_MJESTA);
            
            
        }
        else if (*otvoreno == 1) {
            sem_post(bSemOTVORENO);
            printf("Frizerka: Spavam dok klijenti ne dođu\n");
            sem_wait(bSemSpavanje);
            sem_post(bSemSpavanje); 
            //printf("BUDNA SAM IDEM VAS ŠIŠAT!\n");
            //sem_post(bSemSpavanje);

        }
        else{
            // kraj radnog vremena i salon je prazan
            // zatvori salon                                                                                                                                                                                                                                                           
            // završi s radom
            sem_post(bSemOTVORENO);
            //printf("Frizerka: Zatvaram salon\n");
            return;  //prije je bio return

        }
    }
    

    return;
}

void klijent(int idKlijent){
    //ispiši da si se pojavio(x)
    printf("\tKlijent(%d): Želim na frizuru\n", idKlijent);

    sem_wait(bSemOTVORENO);
    if ((*otvoreno == 1) && (*br_klijenataTrenutno < mjestaCekaonica)) {
        (*primljenoKlijenta)++;
        sem_post(bSemOTVORENO);
        (*br_klijenataTrenutno)++;
        printf("\tKlijent(%d): Ulazim u čekaonicu (%d)\n", idKlijent, *br_klijenataTrenutno); 

        sem_post(bSemSpavanje); 
        sem_wait(oSemBR_MJESTA);  //čekaj na svoj red //semafor
        
        printf("\tKlijent(%d): frizerka mi radi frizuru\n", idKlijent);

        for (int s=0; s<3; s++) {
                //printf("Kontola dole\n");
                sleep(1);//radi na frizuri 
        }

        printf("Frizerka: Klijent %d gotov\n", idKlijent);
        (*br_klijenataTrenutno)--;
    }
    else {
        sem_post(bSemOTVORENO);
        printf("\tKlijent(%d): Nema mjesta u čekaoni, vratit ću se sutra\n", idKlijent);
    }
    
 
    return;
}
 

int main(int argc, char *argv[]) {

    mjestaCekaonica = atoi(argv[1]);

    int IdSegment; /* identifikacijski broj segmenta */

    /* zauzimanje zajedničke memorije */
   IdSegment = shmget(IPC_PRIVATE, 4*sizeof(sem_t)+5*sizeof(int), 0600);
 
   if (IdSegment == -1) {
      exit(1);  /* greška - nema zajedničke memorije */
   }
 
   bSemOTVORENO = shmat(IdSegment, NULL, 1);
   otvoreno = (int *)(bSemOTVORENO+1);
   oSemBR_MJESTA = (sem_t *)(otvoreno+1);
   br_klijenataTrenutno = (int *)(oSemBR_MJESTA+1);
   SemRadi = (sem_t *)(br_klijenataTrenutno+1);
   primljenoKlijenta = (int *)(SemRadi+1);
   bSemSpavanje = (sem_t *)(primljenoKlijenta+1);
   redniBrKlijent = (int *)(bSemSpavanje+1);
   krajRadnogVremena= (int *)(redniBrKlijent+1);


   *br_klijenataTrenutno = 0;
   *otvoreno = 0;
   sem_init(bSemOTVORENO, 1, 1);
   sem_init(oSemBR_MJESTA, 1, 0);
   sem_init(SemRadi, 1, 1);
   sem_init(bSemSpavanje, 1, 0);
   *primljenoKlijenta = 0;
   *redniBrKlijent = 0;
   *krajRadnogVremena = 0;

   int sanse[] = {0, 0, 1};
   srand(time(NULL));

   if (fork() == 0) {   //forkanje - frizerka
    frizerka();
    exit(0);
   }
   
   if (fork() == 0) {
    int pid;
    while(1) {    //forkanje - klijenti
        sleep(1);
        if (*otvoreno == 0) {
            //printf("Frizerka: Postavljam znak ZATVORENO\n");
            break;
        }
        int nRandonNumber = rand()%((2+1)-0) + 0;
        if (sanse[nRandonNumber] == 1) {
            pid = fork();
            if( pid == 0 ) {
                (*redniBrKlijent)++;
                klijent(*redniBrKlijent);
                exit(0);
            }
        }

    }
    exit(0);
   }

    for(int i = 0; i< 15; i++){ 
        sleep(1);
    }
    *krajRadnogVremena = 1;

    *otvoreno = 0;


    sem_post(bSemSpavanje);
   
    wait(NULL);
    sleep(4);

    printf("Frizerka: Zatvaram salon\n");
    // for (int k=0; k<(*redniBrKlijent)+1; k++) {
    //     wait(NULL);
    // }
   
    sem_destroy(bSemOTVORENO);
    sem_destroy(oSemBR_MJESTA);
    sem_destroy(SemRadi);
    sem_destroy(bSemSpavanje);


   (void) shmdt(bSemOTVORENO);
   (void) shmctl(IdSegment, IPC_RMID, NULL);

    return 0;
}