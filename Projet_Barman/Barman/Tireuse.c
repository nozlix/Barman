// Programme qui décrit le fonctionnement d'une tireuse
#include <sys/shm.h> // pour shm
#include <unistd.h> // pour sleep
#include <errno.h> // pour errno
#include <stdio.h> // pour printf
#include <string.h> // pour string
#include <stdlib.h> // pour exit
#include <sys/sem.h>
#include "Tireuse.h"


//#include "Client.c";


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int create_semaphore(key_t key) {
    int semid;
    semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    return semid;
}

void lock_semaphore(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void unlock_semaphore(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void erase_semaphore(key_t key) {
    int semid;
    semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    union semun arg;
    arg.val = 1;
    semctl(semid, 0, IPC_RMID, arg);
}


int main(){
    //creer_shm();

    key_t key = ftok("Tireuse", 'A');
    int shmid = shmget(key, 2*sizeof(struct tireuse), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct tireuse *tireuse;
    tireuse = (struct tireuse *)shmat(shmid, (void *)0, 0);
    if (tireuse == (struct tireuse *)(-1)) {
        perror("shmat");
        exit(1);
    }

    // Exemple d'utilisation
    strcpy(tireuse[0].nom_biere, "Kwak");
    strcpy(tireuse[0].type_biere, "Ambree");
    //sprintf(tireuse->nom_biere,"%s",nom);
    tireuse[0].quantite_biere = 2.0;
    strcpy(tireuse[1].nom_biere, "Goudale");
    strcpy(tireuse[1].type_biere, "Blonde");
    tireuse[1].quantite_biere = 0.5;
    

    printf("Mémoire partagée de la tireuse créée.\n");
    //shmctl(shmid, IPC_RMID, NULL);



}

// Socket TCP
