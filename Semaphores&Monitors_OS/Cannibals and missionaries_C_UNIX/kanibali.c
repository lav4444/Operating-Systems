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


pthread_mutex_t m;
pthread_cond_t red[3];  //0 je red DESNO, 1 je red LIJEVO, 2 je red CAMAC

char *obala[] = {"DESNO", "LIJEVO"};
char *tipOsobe[] = {"K", "M"};

int obalaCamac;     //0-desna, 1-lijeva, trenutna obala camca
int kan_u_cam;
int mis_u_cam;
char *uCamcu[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int camac_u_voznji;
int prvii=1;
int kraj;
// char *redL[1000] = {NULL};
// int iRedLpoc;
// int iRedLzavr;
// char *redD[1000] = {NULL};
// int iRedDpoc;
// int iRedDzavr;

int brDretvi;

void *novaOsoba(void *id){
    int idOsoba = *(int*)(id);
    int obalaOsoba = rand()%2;   //osoba bira na koju obalu dolazi

    pthread_mutex_lock(&m);
        brDretvi++; 
        printf("%s: Pojavio se na %sJ obali\n", tipOsobe[idOsoba], obala[obalaOsoba]);
        ispis();
        // if(obalaOsoba==0) {
        //     redD[iRedDzavr++] = tipOsobe[idOsoba];
        // }else{
        //     redL[iRedLzavr++] = tipOsobe[idOsoba];
        // }

        //pthread_cond_wait(&red[obalaOsoba], &m);
        while( (obalaOsoba!=obalaCamac)||(kan_u_cam+mis_u_cam==7)||(kan_u_cam+(idOsoba==0)>mis_u_cam+(idOsoba==1) && mis_u_cam+(idOsoba==1)!=0)||camac_u_voznji==1 ){   //UVIJETI KAD NE ULAZI ODMAH NA ČAMAC
            // if(obalaOsoba==0) {
            //     redD[iRedDzavr++] = tipOsobe[idOsoba];
            // }else{
            //     redL[iRedLzavr++] = tipOsobe[idOsoba];
            // }
            pthread_cond_wait(&red[obalaOsoba], &m);
        } 

        //UKRCAVA SE U CAMAC    (mozda ovo trba stavit u zaseban mutex)prije je bilo u prvom mutexu
        if (idOsoba==0){
            kan_u_cam++;
        }else mis_u_cam++;

        uCamcu[kan_u_cam+mis_u_cam-1] = tipOsobe[idOsoba];
        printf("%s:  Ukrcavam se u čamac\n", tipOsobe[idOsoba]);
        ispis();
        pthread_cond_wait(&red[2], &m);

        //ISKRCAVANJE IZ CAMCA
        brDretvi--;
    pthread_mutex_unlock(&m);





    return NULL;
}

void *camac(){
    while(1){
        printf("C: prazan na %sJ obali\n", obala[obalaCamac]);
        ispis();
        if(brDretvi<=0 && prvii==0) {
            break;
        }
        prvii=0;

        while(1) {
            pthread_mutex_lock(&m);
            pthread_cond_signal(&red[obalaCamac]);
            // if(obalaCamac==0) {
            //     iRedDpoc++;
            // }else{
            //     iRedLpoc++;
            // }
                if (kan_u_cam+mis_u_cam>=3) {
                    //printf("a\n");
                    
                    printf("C: tri putnika ukrcana, polazak za 1s!\n");
                    ispis();
                    sleep(1);
                    camac_u_voznji = 1;   //camac je u pokretu, nema ukrcavanja
                    printf("C: polazim od %s prema %s, prevozim:", obala[obalaCamac], obala[1-obalaCamac]);
                    for (int s=0; s<kan_u_cam+mis_u_cam; s++) {
                        printf(" %s", uCamcu[s]);
                    }
                    printf("\n\n");

                    //PRIJEVOZ TRAJE 2s prije je bilo u zasebnom mutexu
                    sleep(1);
                    sleep(1);

                    printf("C: GOTOV prijevoz od %s prema %s, prevezeni:", obala[obalaCamac], obala[1-obalaCamac]);
                    for (int s=0; s<kan_u_cam+mis_u_cam; s++) {
                        printf(" %s", uCamcu[s]);
                    }
                    printf("\n");
                    pthread_cond_broadcast(&red[2]);
                    kan_u_cam=0;
                    mis_u_cam=0;
                    for (int s=0; s<7; s++) {
                        uCamcu[s] = NULL;
                    }
                    camac_u_voznji=0;

                    pthread_mutex_unlock(&m);
                    break;
                }

            pthread_mutex_unlock(&m);
        }

        obalaCamac = 1 - obalaCamac;
    }


    return NULL;
}
void ispis() {
    printf("C[%s]={", (obalaCamac==0)?"D":"L");
    int pprvi=1;
    for(int k=0; k<7; k++) {
        if (uCamcu[k] == NULL) break;
        if(pprvi==0){
            printf(" ");
        }
        pprvi=0;
        printf("%s", uCamcu[k]);
    }
    // printf("} LO={");
    // for(int k=iRedLpoc; k<iRedLzavr; k++) {
    //     //if (redL[k] == NULL) break;
    //     if(pprvi==0){
    //         printf(" ");
    //     }
    //     pprvi=0;
    //     printf("%s", redL[k]);
    // }
    // printf("} DO={");
    // for(int k=iRedDpoc; k<iRedDzavr+1; k++) {
    //     if (redD[k] == NULL) break;
    //     if(pprvi==0){
    //         printf(" ");
    //     }
    //     pprvi=0;
    //     printf("%s", redD[k]);
    // }
    printf("}");

    printf("\n\n");

    return;
}


int main(){



    pthread_t thrId;
    pthread_attr_t attr;

    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&red[0], NULL);   //RED DESNO
    pthread_cond_init(&red[1], NULL);   //RED LIJEVO
    pthread_cond_init(&red[2], NULL);   //RED CAMAC

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    brDretvi=0;
    obalaCamac=0;   //pocetno je CAMAC na DESNOJ obali
    kan_u_cam=0;
    mis_u_cam=0;
    camac_u_voznji=0;   //u pocetku camac miruje
    kraj=0;
    // iRedDpoc = 0;
    // iRedDzavr = 0;
    // iRedLpoc = 0;
    // iRedLzavr = 0;
    srand ((unsigned int) time(NULL));

    int kan = 0, mis = 1;

    pthread_create(&thrId, &attr, camac, NULL);

    for (int i=0; i<1312; i++) {
        sleep(1);
        if (i%2==0) {
            //dolazi misionar
            pthread_create(&thrId, &attr, novaOsoba, &mis);
        }
        //dolazi kanibal
        pthread_create(&thrId, &attr, novaOsoba, &kan);

    }
    sleep(2);
    kraj=1;
    printf("NE PRIMAMO NOVE PUTNIKE VISE!\n");
    while(1){   //cekamo da se svi putnici prevezu
        sleep(1);
        if (brDretvi<=0) break;
    } 

    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&red[0]); 
    pthread_cond_destroy(&red[1]);   
    pthread_cond_destroy(&red[2]);   




    return 0;
}