// Compilation : gcc -o Controle.o Controle.c
/* Execution :
   1. Exécuter Commande.java
   2. Dans un autre terminal, lancer : ./Controle.o 127.0.0.1 7777 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <signal.h>
#include "communication.h"

extern int semid;


#define TAILLEBUF 1000

pthread_mutex_t controle_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t controle_cond = PTHREAD_COND_INITIALIZER;
int sock;
static struct sockaddr_in addr_serveur; // Adresse de la socket coté serveur
socklen_t lg;                           // Taille de l'addresse socket
extern volatile sig_atomic_t running;
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


void traitement(){
    int choix;
    char *msg;                              // Chaine à envoyer
    char buffer[TAILLEBUF];                 // Buffer de réception
    char *reponse;                          // Chaine reçue en réponse
    int nb_octets;  

    // ----- Choix du barman sur sa demande au fournisseur -----
            printf("\n\nQue voulez-vous demander au fournisseur ?\n");
            printf("  [1] : La liste des bières blondes\n");
            printf("  [2] : La liste des bières ambrées\n");

            scanf("%d", &choix);

            while (choix != 1 || choix != 2) {
                printf("Ce choix n'est pas accepté.\n");
                printf("\n\nQue voulez-vous demander au fournisseur ?\n");
                printf("  [1] : La liste des bières blondes\n");
                printf("  [2] : La liste des bières ambrées\n");

                scanf("%d", &choix);
            }

            switch (choix)
            {
            case 1:
                msg = "LISTEBLONDE";
                break;
            case 2:
                msg = "LISTEAMBREE";
                break;
            }

            if (sock == -1)
            {
                perror("Erreur création socket");
                exit(1);
            }

            // On envoie le message au serveur (LISTEBLONDE, LISTEAMBREE ou ACHETER)
            nb_octets = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr_serveur, lg);
            if (nb_octets == -1)
            {
                fprintf(stderr, "Erreur envoi message \"%s\"", msg);
                exit(1);
            }

            // On reçoie la réponse du serveur (listeBlondes(), listeAmbrees() ou demande de choix de biere)
            nb_octets = recvfrom(sock, buffer, TAILLEBUF, 0, (struct sockaddr *)&addr_serveur, &lg);
            if (nb_octets == -1)
            {
                perror("Erreur réponse serveur");
                exit(1);
            }
            reponse = (char *)malloc(nb_octets * sizeof(char));
            memcpy(reponse, buffer, nb_octets);
            printf("Bières : %s\n", reponse);
}

void *controle_thread(void *arg){
    struct connexion *connexion = (struct connexion *)arg;
    pthread_t *main_tid = (pthread_t *)connexion->thread;
    char *port = connexion->port;
    char *addr = connexion->addr;
    pthread_mutex_lock(&controle_mtx);
    
    struct hostent *serveur_host;           // Identifiant de la machine serveur
    
    char *msg;                              // Chaine à envoyer
    char buffer[TAILLEBUF];                 // Buffer de réception
    char *reponse;                          // Chaine reçue en réponse
    int nb_octets;                          // Nombre d'octets lus ou envoyés
    sock = socket(AF_INET, SOCK_DGRAM, 0);  // Création d'une socket UDP
    int choix;                              // Choix du barman
    char *biere;                            // Biere a acheter
    biere = (char *)malloc(sizeof(char) * 100);
    char *nom_biere;
    lg = sizeof(struct sockaddr_in);

    // ----- CONNEXION AU SOCKET UDP -----
    serveur_host = gethostbyname(addr);
    if (serveur_host == NULL)
    {
        perror("Erreur adresse serveur");
        exit(1);
    }

    // Création adresse socket destinatrice
    bzero(&addr_serveur, sizeof(struct sockaddr_in));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(atoi(port));
    memcpy(&addr_serveur.sin_addr.s_addr, serveur_host->h_addr, serveur_host->h_length);

    // ----- CONNEXION A LA SHM -----
    key_t key = ftok("Tireuse", 'A');
    int shmid = shmget(key, 2 * sizeof(struct tireuse), 0644);
    if (shmid == -1)
    {
        perror("shmget_ctrl");
        exit(1);
    }

    struct tireuse *tireuse;
    tireuse = (struct tireuse *)shmat(shmid, (void *)0, 0);
    if (tireuse == (struct tireuse *)(-1))
    {
        perror("shmat");
        exit(1);
    }

    //int semid = create_semaphore(key);
   

    while (running)
    {
        pthread_cond_wait(&controle_cond, &controle_mtx);
        // Verification de la quantité des fûts
        //printf("Etat de la tireuses 0 (ambrées, %s) : %f\n", tireuse[0].nom_biere, tireuse[0].quantite_biere);
        //printf("Etat de la tireuses 1 (blondes, %s) : %f\n", tireuse[1].nom_biere, tireuse[1].quantite_biere);
        if (tireuse[0].quantite_biere == 0.0 || tireuse[1].quantite_biere == 0.0) {
            
            if (tireuse[0].quantite_biere == 0.0)
            {
                msg = "ACHETERAMBREE";
            }

            if (tireuse[1].quantite_biere == 0.0)
            {
                msg = "ACHETERBLONDE";
            }

            //printf("msg : %s", msg);
            

            // On envoie le message au serveur (ACHETERBLONDE)
            nb_octets = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr_serveur, lg);
            if (nb_octets == -1)
            {
                fprintf(stderr, "Erreur envoi message \"%s\"", msg);
                exit(1);
            }
            
            // On reçoie la réponse du serveur (demande de biere)
            nb_octets = recvfrom(sock, buffer, TAILLEBUF, 0, (struct sockaddr *)&addr_serveur, &lg);
            if (nb_octets == -1)
            {
                perror("Erreur réponse serveur");
                exit(1);
            }
            
            reponse = (char *)malloc(nb_octets * sizeof(char));
            memcpy(reponse, buffer, nb_octets);
            printf("%s\n", reponse);
            scanf("%s", biere);

            while (strcmp(biere, "1")!=0 && strcmp(biere, "2")!=0 && strcmp(biere, "3")!=0){
                printf("Ce choix n'est pas accepté.\n");
                scanf("%s", biere);
            }

            // On envoie le nom de la bière
            nb_octets = sendto(sock, biere, strlen(biere), 0, (struct sockaddr *)&addr_serveur, lg);
            if (nb_octets == -1)
            {
                perror("Erreur envoi biere");
                exit(1);
            }

            // On reçoie la réponse du serveur (le fut de biere)
            nb_octets = recvfrom(sock, buffer, TAILLEBUF, 0, (struct sockaddr *)&addr_serveur, &lg);
            if (nb_octets == -1)
            {
                perror("Erreur réponse serveur");
                exit(1);
            }
            reponse = (char *)malloc(nb_octets * sizeof(char));
            memcpy(reponse, buffer, nb_octets);
            printf("Réponse validée :  %s\n\n", reponse);
            nom_biere = strtok(reponse, ",");

            // On met à jour la SHM
            
            lock_semaphore(semid);
            if (msg == "ACHETERAMBREE") {
                strcpy(tireuse[0].nom_biere, nom_biere);
                tireuse[0].quantite_biere = 5.0;
            } else {
                strcpy(tireuse[1].nom_biere, nom_biere);
                tireuse[1].quantite_biere = 5.0;
            }
            unlock_semaphore(semid);
        }

        // Attendre deux secondes avant de vérifier à nouveau l'état des tireuses
        sleep(2);
    }
    //erase_semaphore(key);
    shmdt(tireuse);
    
    

    // On ferme la socket et on libère la mémoire
    free(biere);
    close(sock);
}

