#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/prctl.h> //tuer fils


#include "communication.h"
#include "main.h"
extern volatile sig_atomic_t running;
extern int semid;
pthread_mutex_t main_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;



void main_thread_handler(int signum) {
    // Ne rien faire, juste pour interrompre pause()
}



struct client *file = NULL;



void *main_thread(void *arg) {
   
    struct connexion *connexion = (struct connexion *)arg;
    pthread_t *communication_tid = (pthread_t *)connexion->thread;
    
    pthread_mutex_lock(&main_mtx);

    //system("./Tireuse");
    key_t key = ftok("Tireuse", 'A');
    
    //key_t key = ftok("tireuse", 'R');
    int shmid = shmget(key, sizeof(struct tireuse), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget_main");
        exit(1);
    }

    struct tireuse *data;
    data = (struct tireuse *)shmat(shmid, (void *)0, 0);
    if (data == (struct tireuse *)(-1)) {
        perror("shmat");
        exit(1);
    }

    printf("biere : %s\n", data[0].nom_biere);
    
    

    int fd, ab, fd2;
    struct tireuse *tireuse;
    struct donneeClient *donnee = malloc(sizeof(struct donneeClient));
    int status, cle = 5;
    if (mkfifo("envoie", 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
    
    }
    if (mkfifo("recoit", 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");

    }
    if (mkfifo("nouveau", 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
       
    }
    if ((fd = open("envoie", O_WRONLY)) == -1) {
        perror("open");
    }
    if ((ab = write(fd, data, 2*sizeof(struct tireuse))) == -1) {
        perror("write");
    }
    close(fd);
    int new = open("nouveau", O_RDONLY);
    // fd = open("recoit", O_RDWR);
    //     if (fd < 0) {
    //         perror("Erreur lors de l'ouverture du pipe nommé pour lire");
    //         exit(EXIT_FAILURE);
    //     }
    while(running){
        pthread_cond_wait(&main_cond, &main_mtx);
        ssize_t read_size;
        while ((read_size = read(new, donnee, sizeof(struct donneeClient))) > 0 &&running) {
            recupDemande(donnee);
        }
        sleep(1);
        struct client *client_suivant = depile();
        if (client_suivant !=NULL){
            traiterCommande(client_suivant, semid);
            
        }
        free(client_suivant);


        
    }
    close(fd);
    if (shmdt(data) == -1) {
        perror("shmdt");
        exit(1);
    }
    erase_semaphore(key);
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        printf("shm2.shmctl: %s\n", strerror(errno));
        exit(1);
    }

    pthread_mutex_unlock(&main_mtx);
    return NULL;
}

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
        perror("semop_lock");
        exit(1);
    }
}

void unlock_semaphore(int semid) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop_unlock");
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


// Empiler les clients dans l'ordre FIFO
void empiler(struct donneeClient donnee) {
    struct client *new_client = (struct client *) malloc(sizeof(struct client));
    new_client->data = donnee;
    new_client->prochain = NULL;

    if (file == NULL) {
        file = new_client;
    } else {
        struct client *tmp = file;
        while (tmp->prochain != NULL) {
            tmp = tmp->prochain;
        }
        tmp->prochain = new_client;
    }
}

// Recupérer les demandes clients dans l'ordre FIFO
struct client *depile() {
    if (file == NULL) {
        return NULL;
    }

    struct client *next_client = file;
    file = file->prochain;
    next_client->prochain = NULL;

    return next_client;
}


void traiterCommande(struct client *client, int semid){
        struct tireuse *tireuse = malloc(2*sizeof(struct tireuse));
        struct reponse rep;
        rep.taille_reponse = sizeof(struct reponse);
        key_t key = ftok("Tireuse", 'A');
        int shmid = shmget(key, sizeof(struct tireuse), 0644 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget_traiter");
            exit(1);
        }
        
        tireuse = (struct tireuse *)shmat(shmid, (void *)0, 0);
        if (tireuse == (struct tireuse *)(-1)) {
            perror("shmat");
            exit(1);
        }

        lock_semaphore(semid);
        
        int fd = open("envoie", O_WRONLY);
        if (fd < 0) {
            perror("Erreur lors de l'ouverture du pipe nommé pour écrire\n");
            exit(EXIT_FAILURE);
        }
        if((strcmp(tireuse[0].nom_biere, client->data.tireuse[0].nom_biere)) == 0){
            if(tireuse[0].quantite_biere >= client->data.tireuse[0].quantite_biere){
                rep.type_reponse = VALIDER;
                tireuse[0].quantite_biere -= client->data.tireuse[0].quantite_biere;
                write(fd, &rep, rep.taille_reponse);
            }
            else{
                rep.type_reponse = QUANTITE_INDISPONIBLE;
                write(fd, &rep, rep.taille_reponse);
            }
        }
        else if((strcmp(tireuse[1].nom_biere, client->data.tireuse[0].nom_biere)) == 0){
            if(tireuse[1].quantite_biere >= client->data.tireuse[0].quantite_biere){
                rep.type_reponse = VALIDER;
                tireuse[1].quantite_biere -= client->data.tireuse[0].quantite_biere;
                write(fd, &rep, rep.taille_reponse);
            }
            else{
                rep.type_reponse = QUANTITE_INDISPONIBLE;
                write(fd, &rep, rep.taille_reponse);
            }   
        }
        else{
            rep.type_reponse = BIERE_INDISPONIBLE;
            write(fd, &rep, rep.taille_reponse);
        }
        unlock_semaphore(semid);
        shmdt(tireuse);
        
        close(fd);
         // détacher segment partagé
}


// ----- Recupère la demande du client depuis le pipe -----
void recupDemande(struct donneeClient *donnee) {
    struct tireuse *tireuse = malloc(2*sizeof(struct tireuse));
  
    if (donnee->req == INFORMATIONS){
        key_t key = ftok("Tireuse", 'A');
        int shmid = shmget(key, sizeof(struct tireuse), 0644 | IPC_CREAT);
        if (shmid == -1) {
            perror("shmget_recup");
            exit(1);
        }
        tireuse = (struct tireuse *)shmat(shmid, (void *)0, 0);
        if (tireuse == (struct tireuse *)(-1)) {
            perror("shmat");
            exit(1);
        }
        //lock_semaphore(semid);
        memcpy(donnee->tireuse, tireuse, 2 * sizeof(struct tireuse));
        //unlock_semaphore(semid);
        
        int fd = open("envoie", O_WRONLY);
        if (fd < 0) {
            perror("Erreur lors de l'ouverture du pipe nommé pour écrire");
            exit(EXIT_FAILURE);
        }
        write(fd, donnee, sizeof(struct donneeClient));
        shmdt(tireuse); // détacher segment partagé
        close(fd);
    }
    else if(donnee->req == COMMANDE){
        struct donneeClient data;
        memcpy(&data, donnee, sizeof(struct donneeClient));
        empiler(data);
    }
}


