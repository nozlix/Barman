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
#include <signal.h>
#include <stdbool.h>

#include "communication.h"
extern volatile sig_atomic_t running;

pthread_mutex_t communication_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t communication_cond = PTHREAD_COND_INITIALIZER;

void *communication_thread(void *arg) {
    struct connexion *connexion = (struct connexion *)arg;
    pthread_t *controle_tid = (pthread_t *)connexion->thread;
    char* port = connexion->port;
    /* struct donneeClient *donnee = (struct donneeClient *)arg;
    int socket = donnee->client_socket;
    int thread_id = donnee->thread_id;
    pthread_t *controle_tid = (pthread_t *)arg; */
    pthread_mutex_lock(&communication_mtx);
    struct tireuse *tireuse;
    tireuse = malloc(2 * sizeof(struct tireuse));
    recoitTireuse(tireuse);
    int socket_ecoute, socket_service;
    static struct sockaddr_in addr_client;
    int lg;
    socket_ecoute = creerSocketTCP(atoi(port));
    if (socket_ecoute == -1)
    {
        perror("Creation socket service\n");
        //return 1;
    }
    if (listen(socket_ecoute, 10) == -1)
    {
        perror("erreur listen\n");
        //return 1;
    }
    lg = sizeof(struct sockaddr_in);
    int thread_id_nombre = 0;
    while (running)
    {
        pthread_cond_wait(&communication_cond, &communication_mtx);
        socket_service = accept(socket_ecoute, (struct sockaddr *)&addr_client, &lg);
        struct donneeClient *donnee = malloc(sizeof(struct donneeClient));
        //donnee->tireuse = malloc(2*sizeof(struct tireuse));
        donnee->client_socket = socket_service;
        memcpy(donnee->tireuse, tireuse, 2 * sizeof(struct tireuse));
        donnee->thread_id = thread_id_nombre++;        
        pthread_t client_thread;
        int resultat = pthread_create(&client_thread, NULL, gererClientThread, donnee);
        if (resultat != 0)
        {
            perror("Erreur lors de la création du thread");
            exit(EXIT_FAILURE);
        }
        
        // Détache le thread pour qu'il libère les ressources lorsqu'il a terminé
        pthread_detach(client_thread);
    }
    close(socket_service);
    close(socket_ecoute);
    pthread_mutex_unlock(&communication_mtx);
    return NULL;
}

void communication_thread_handler(int signum) {
    // Ne rien faire, juste pour interrompre pause()
}


volatile sig_atomic_t running_com = 1;

void signal_handler_com(int sig) {
    running_com = 0;
}



/* Vérifie la disponibilité de la bière et renvoie envoie la réponse à gerer client*/
enum reponse_t verificationCommande(struct tireuse tireuse, struct tireuse tireuse_commande)
{
    struct reponse rep;
    if (strcmp(tireuse_commande.nom_biere, tireuse.nom_biere) == 0)
    {
        if (tireuse_commande.quantite_biere <= tireuse.quantite_biere)
        {
            return VALIDER;
        }
        else
        {
            return QUANTITE_INDISPONIBLE;
        }
    }
    else
    {
        return BIERE_INDISPONIBLE;
    }
}
/* Cherche le type de la bière */
struct tireuse verificationCommandeType(struct tireuse *tireuse, char *type_biere)
{
    if (strcmp(type_biere, "Ambree") == 0)
    {

        // verificationCommande(tireuse[0], tireuse_commande);
        return tireuse[0];
    }
    else
    {
        // verificationCommande(tireuse[1], tireuse_commande);
        return tireuse[1];
    }
}
int creerSocket(int port, int type)
{
    static struct sockaddr_in adresse;
    int sock;
    sock = socket(AF_INET, type, 0);
    if (sock == -1)
    {
        return -1;
    }
    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Erreur lors de la configuration de SO_REUSEADDR");
        exit(1);
    }
    bzero((char *)&adresse, sizeof(adresse));
    adresse.sin_family = AF_INET;
    printf("%d\n", port);
    adresse.sin_port = htons(port);
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&adresse, sizeof(adresse)) == -1)
    {
        return -1;
    }
    return sock;
}

void *gererClientThread(void *arg)
{
    // int client_socket = *((int *) arg);
    int fd;
    struct donneeClient *donnee = (struct donneeClient *)arg;
    int socket = donnee->client_socket;
    int thread_id = donnee->thread_id;
    struct tireuse *tireuse = donnee->tireuse;
    struct requete req;
    struct tireuse tireuse_commande;
    struct reponse rep;
    rep.taille_reponse = sizeof(struct reponse);
    if (read(socket, &req, sizeof(struct requete)) <= 0)
    {
        perror("Erreur\n");
        return NULL;
    }
    donnee->req = req.type_requete;
    if (req.type_requete == INFORMATIONS)
    {
        
        if ((fd = open("nouveau", O_WRONLY)) == -1)
        {
            perror("open");
            //return 1;
        }
        if ((write(fd, donnee, sizeof(struct donneeClient))) == -1)
        {
            perror("read");
            //return 1;
        }
        close(fd);

        if ((fd = open("envoie", O_RDONLY)) == -1)
        {
            perror("open");
            //return 1;
        }
        if ((read(fd, donnee, sizeof(struct donneeClient))) == -1)
        {
            perror("read");
            //return 1;
        }
        close(fd);
        printf("tireuse 0 %s\n", donnee->tireuse[0].nom_biere);
        if((write(socket, donnee->tireuse, 2 * sizeof(struct tireuse))) ==-1){
          perror("erreur envoie tireuse\n");  
        };
        
    }
    else if (req.type_requete == COMMANDE)
    {
        
        if (write(socket, &req, sizeof(struct requete)) <= 0)
        {
            exit(1);
        }
        if (read(socket, &tireuse_commande, sizeof(struct tireuse)) <= 0)
        {
            perror("Erreur\n");
            return NULL;
        }
        donnee->tireuse[0] = tireuse_commande;


        if ((fd = open("nouveau", O_WRONLY)) == -1)
        {
            perror("open");
            //return 1;
        }
        if ((write(fd, donnee, sizeof(struct donneeClient))) == -1)
        {
            perror("read");
            //return 1;
        }
        close(fd);

        if ((fd = open("envoie", O_RDONLY)) == -1)
        {
            perror("open");
            //return 1;
        }
        if ((read(fd, &rep, sizeof(struct reponse))) == -1)
        {
            perror("read");
            //return 1;
        }
        close(fd);

        if((write(socket, &rep, sizeof(struct reponse))) == -1){
            perror("erreur envoie reponse\n");
        };
        
        
        
    }
        // Fermeture de la connexion
        close(socket);
        //free(donnee);
        pthread_exit(NULL);
}
// Création socket
int creerSocketTCP(int port)
{
    return (creerSocket(port, SOCK_STREAM));
}

// Permet de recupérer la tireuse depuis le pipe
void recoitTireuse(struct tireuse* tireuse){
    int fd;
    if ((fd = open("envoie", O_RDWR)) == -1)
    {
        perror("open");
        //return NULL;
    }
    if ((read(fd, tireuse, 2 * sizeof(struct tireuse))) == -1)
    {
        perror("read");
    }
    close(fd);
}

/* int principales()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler_com;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    struct tireuse *tireuse;
    tireuse = malloc(2 * sizeof(struct tireuse));
    recoitTireuse(tireuse);
    int socket_ecoute, socket_service;
    static struct sockaddr_in addr_client;
    int lg;
    socket_ecoute = creerSocketTCP(4000);
    if (socket_ecoute == -1)
    {
        perror("Creation socket service\n");
        return 1;
    }
    if (listen(socket_ecoute, 10) == -1)
    {
        perror("erreur listen\n");
        return 1;
    }
    lg = sizeof(struct sockaddr_in);
    int thread_id_nombre = 0;
    while (running_com)
    {

        socket_service = accept(socket_ecoute, (struct sockaddr *)&addr_client, &lg);
        struct donneeClient *donnee = malloc(sizeof(struct donneeClient));
        //donnee->tireuse = malloc(2*sizeof(struct tireuse));
        donnee->client_socket = socket_service;
        memcpy(donnee->tireuse, tireuse, 2 * sizeof(struct tireuse));
        //donnee->tireuse = tireuse;
        donnee->thread_id = thread_id_nombre++;
        
        pthread_t client_thread;
        int resultat = pthread_create(&client_thread, NULL, gererClientThread, donnee);
        if (resultat != 0)
        {
            perror("Erreur lors de la création du thread");
            exit(EXIT_FAILURE);
        }
        
        // Détache le thread pour qu'il libère les ressources lorsqu'il a terminé
        pthread_detach(client_thread);
    }
    close(socket_service);
    close(socket_ecoute);
    return 0;
} */
