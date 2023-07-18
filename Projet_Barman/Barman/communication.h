#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <string.h>
#include <pthread.h>
#include "Tireuse.h"



extern pthread_cond_t main_cond;
extern pthread_cond_t communication_cond;
extern pthread_cond_t controle_cond;

void *main_thread(void *arg);
void *communication_thread(void *arg);
void *controle_thread(void *arg);




enum requete_t {
    INFORMATIONS = 0,
    COMMANDE,
    ANNULE
};

struct connexion
{
    pthread_t thread;
    char *addr;
    char *port;
};

struct donneeClient
{
    int client_socket;
    struct tireuse tireuse[2];
    int thread_id;
    enum requete_t req;

};

struct client
{
    struct donneeClient data;
    struct client *prochain;
};

void *gererClientThread(void *arg);

int creerSocketTCP();


struct requete{
    enum requete_t type_requete;
    int taille_requete;
};

enum reponse_t {
    VALIDER = 0,
    BIERE_INDISPONIBLE,
    QUANTITE_INDISPONIBLE
};
struct reponse{
    enum reponse_t type_reponse;
    int taille_reponse;
};

void gererClient(int socket, struct tireuse* tireuse);
void recoitTireuse(struct tireuse* tireuse);


#endif