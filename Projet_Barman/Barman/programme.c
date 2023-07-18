#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "communication.h"
pthread_t main_tid, communication_tid, controle_tid;
int current_thread = 0;
volatile sig_atomic_t running = 1;
int semid;
key_t key;

void sigint_handler(int sig) {
    running = 0;
    erase_semaphore(key);
    pthread_cond_signal(&main_cond);
    pthread_cond_signal(&communication_cond);
    pthread_cond_signal(&controle_cond);
    pthread_exit(NULL);
}
void *switch_threads(void *arg);

int main(int argc, char** argv) {
    if(argc != 4){
        printf("Erreur lors de la saisie de connexion\n");
    }
    system("./Tireuse");
    key = ftok("Tireuse", 'A');
    semid = create_semaphore(key);

    
    struct connexion connexion_main;
    connexion_main.thread = communication_tid;
    struct connexion connexion_communication;
    connexion_communication.thread = controle_tid;
    connexion_communication.port = malloc(sizeof(argv[3]));
    connexion_communication.port = argv[3];
    struct connexion connexion_controle;
    connexion_controle.thread = main_tid;
    connexion_controle.addr = malloc(sizeof(argv[1]));
    connexion_controle.port = malloc(sizeof(argv[2]));
    connexion_controle.addr = argv[1];
    connexion_controle.port = argv[2];
    //printf("& %s 2 %s  %s",  connexion_controle.addr, connexion_controle.port, connexion_communication.port);
    
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    pthread_create(&main_tid, NULL, main_thread, (void *)&connexion_main);
    
  
    pthread_create(&communication_tid, NULL, communication_thread, (void *)&connexion_communication);
   
    pthread_create(&controle_tid, NULL, controle_thread, (void *)&connexion_controle);
    
    
    pthread_t scheduler_tid;
    pthread_create(&scheduler_tid, NULL, switch_threads, NULL);

    pthread_join(main_tid, NULL);
    pthread_join(communication_tid, NULL);
    pthread_join(controle_tid, NULL);


    return 0;
}
void *switch_threads(void *arg) {
    while (running) {
        sleep(1);
        if (current_thread == 0) {
            pthread_cond_signal(&main_cond);
            current_thread = 1;
        } else if (current_thread == 1) {
            pthread_cond_signal(&communication_cond);
            current_thread = 2;
        } else{
            pthread_cond_signal(&controle_cond);
            current_thread = 0;
        }
        
    }
    return NULL;
}
