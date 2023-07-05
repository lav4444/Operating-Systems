#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

struct node
{
    int data;
    struct node *next;
};

struct node *top = NULL;

void push(int item)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->data = item;
    nptr->next = top;
    top = nptr;
}

void display()
{
    struct node *temp;
    temp = top;
    if (top == NULL)
    {
        printf(" stog: -\n");
    }
    else
    {
        printf(" stog:");
        int first = 1;
        while (temp != NULL) {
            if (first != 1) {
                printf(";");
            }
            first = 0;
            printf(" %d, reg[%d]", temp->data, temp->data);
            temp = temp->next;
        }
        printf("\n");
    }
}
int seekTop() {
    if (top == NULL)
    {
        //printf("\n\nStack is empty ");////////
        return -1;
    }
    else
    {
        return top->data;
    }
}


int pop()
{
    if (top == NULL)
    {
        printf("\n\nStack is empty ");////////
        return -1;
    }
    else
    {
        struct node *temp;
        temp = top;
        top = top->next;
        //printf("\n\n%d deleted", temp->data);
        return temp->data;
    }
}

//----------------------------------------------

struct timespec t0; /* vrijeme pocetka programa */

/* postavlja trenutno vrijeme u t0 */
void postavi_pocetno_vrijeme()
{
	clock_gettime(CLOCK_REALTIME, &t0);
}

/* dohvaca vrijeme proteklo od pokretanja programa */
void vrijeme(void)
{
	struct timespec t;

	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec -= t0.tv_sec;
	t.tv_nsec -= t0.tv_nsec;
	if (t.tv_nsec < 0) {
		t.tv_nsec += 1000000000;
		t.tv_sec--;
	}

	printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec/1000000);
}

void spavaj(time_t sekundi)
{
	struct timespec koliko;
	koliko.tv_sec = sekundi;
	koliko.tv_nsec = 0;

	while (nanosleep(&koliko, &koliko) == -1 && errno == EINTR) {
        
    }
		//PRINTF("Bio prekinut, nastavljam\n");
}
//---------------------------------------------------------------

/* funkcije za obradu signala, navedene ispod main-a */ 
void obradi_signal(int sig);
void obradi_SIGUSR1(int sig);
void obradi_sigterm(int sig);
void obradi_sigint(int sig);
int nije_kraj = 1;
int tpChanged = 0;

//-------------------------------------
int zastavica1 = 0;
int zastavica2 = 0;
int zastavica3 = 0;
int trenStanje = 0;
void ispisStanja() {
    vrijeme();
    printf("K_Z=%d%d%d, T_P=%d,", zastavica1, zastavica2, zastavica3, trenStanje);
    display();
    printf("\n");

}



int main() {
    struct sigaction act;

	/* 1. maskiranje signala SIGUSR1 */
	act.sa_handler = obradi_signal; // kojom se funkcijom signal obradjuje 
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER; //naprednije mogucnosti preskocene
	sigaction(SIGUSR1, &act, NULL); // maskiranje signala - povezivanje sucelja OS-a 

	/* 2. maskiranje signala SIGTERM */
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);

	/* 3. maskiranje signala SIGINT */
	sigaction(SIGINT, &act, NULL);
    

    postavi_pocetno_vrijeme();

    vrijeme();
    printf("Program s PID=%ld krenuo s radom\n", (long) getpid()); 
    ispisStanja();
    
    /* neki posao koji program radi; ovdje samo simulacija */
    int i = 1;
    while(nije_kraj) {
        
    }
    printf("Program s PID=%ld zavrsio s radom\n", (long) getpid()); 

    return 0;
}

void obradi_signal(int sig) {

    if (sig == 2 && trenStanje!=3) { //SIGINT
        zastavica3 = 1;
    }
    else if(sig == 10 && trenStanje!=2) { //SIGUSR1
        zastavica2 = 1;
    }
    else if(sig == 15 && trenStanje!=1) { //SIGTERM
        zastavica1 = 1;
    }
    if (zastavica1 || zastavica2 || zastavica3) {
            if (zastavica3) {
                obradi_sigint(2);
                tpChanged = 1;
            }
            if (zastavica2) {
                obradi_SIGUSR1(10);
                tpChanged = 1;
            }
            if (zastavica1) obradi_sigterm(15);
            tpChanged = 0;
    }
}

void obradi_SIGUSR1(int sig) {
    if (tpChanged) {
        if (trenStanje<2) {
            vrijeme();
            printf("SKLOP: promijenio se T_P, prosljeđuje prekid razine 2 procesoru\n");
            tpChanged = 0;
        } else return;
    } else {
        vrijeme();
        printf("SKLOP: Dogodio se prekid razine 2 "); 
        if (trenStanje<2) {
            printf("i prosljeđuje se procesoru\n");
            ispisStanja();
        } else {
            printf("ali se on pamti i ne prosljeđuje procesoru\n");
            ispisStanja();
            return;
        }
    }
    int i=0;
    vrijeme();
    printf("Počela obrada prekida razine 2\n");
    push(trenStanje);
    trenStanje = 2;
    zastavica2 = 0;
    ispisStanja();
    for (i = 1; i <= 12; i++) {
        sleep(1); 
    }  
    vrijeme();
    printf("Završila obrada prekida razine 2\n"); 
    trenStanje = seekTop();
    vrijeme();
    printf("Nastavlja se "); 
    if (seekTop()>0) {
        printf("obrada prekida razine %d\n", pop());
    } else {
        printf("izvođenje glavnog programa\n");
        pop();
    }
    ispisStanja();
}
void obradi_sigterm(int sig) {
    if (tpChanged) {
        if (trenStanje<1) {
            vrijeme();
            printf("SKLOP: promijenio se T_P, prosljeđuje prekid razine 1 procesoru\n");
            tpChanged = 0;
        } else return;
        
    } else {
        vrijeme();
        printf("SKLOP: Dogodio se prekid razine 1 "); 
        if (trenStanje<1) {
            printf("i prosljeđuje se procesoru\n");
            ispisStanja();
        } else {
            printf("ali se on pamti i ne prosljeđuje procesoru\n");
            ispisStanja();
            return;
        }
    }
    int i=0;
    vrijeme();
    printf("Počela obrada prekida razine 1\n");
    push(trenStanje);
    trenStanje = 1;
    zastavica1 = 0;
    ispisStanja();
    for (i = 1; i <= 12; i++) {
        sleep(1); 
    }  
    vrijeme();
    printf("Završila obrada prekida razine 1\n"); 
    trenStanje = seekTop();
    vrijeme();
    printf("Nastavlja se "); 
    if (seekTop()>0) {
        printf("obrada prekida razine %d\n", pop());
    } else {
        printf("izvođenje glavnog programa\n");
        pop();
    }
    ispisStanja();
}
void obradi_sigint(int sig) {
    vrijeme();
    printf("SKLOP: Dogodio se prekid razine 3 "); 
    if (trenStanje<3) {
        printf("i prosljeđuje se procesoru\n");
        ispisStanja();
    } 
    vrijeme();
    printf("Počela obrada prekida razine 3\n");
    push(trenStanje);
    trenStanje = 3;
    zastavica3 = 0;
    ispisStanja();
    for (int i = 1; i <= 12; i++) {
        sleep(1); 
    }  
    vrijeme();
    printf("Završila obrada prekida razine 3\n"); 
    trenStanje = seekTop();
    vrijeme();
    printf("Nastavlja se "); 
    if (seekTop()>0) {
        printf("obrada prekida razine %d\n", pop());
    } else {
        printf("izvođenje glavnog programa\n");
        pop();
    }
    ispisStanja();
}