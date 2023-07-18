#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include "Tireuse.h"
#include "Client.h"
#define TAILLEBUF 50
int sock; // socket locale coté client
struct tireuse *tireuse;
char* port;
char* addr;

int creerSocketTCP()
{
    int attente = 0;
    // identifiant de la machine serveur
    struct hostent *serveur_host;
    // adresse de la socket coté serveur
    static struct sockaddr_in addr_serveur;
    // taille de l'addresse socket
    socklen_t lg;
    // descripteur de la socket locale
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return -1;
    }
    serveur_host = gethostbyname(addr);
    if (serveur_host == NULL)
    {
        perror("erreur adresse serveur");
        return -1;
    }
    // création adresse socket destinatrice
    bzero(&addr_serveur, sizeof(struct sockaddr_in));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(atoi(port));
    memcpy(&addr_serveur.sin_addr.s_addr, serveur_host->h_addr, serveur_host->h_length);
    while(connect(sock, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1 && attente < 4){
        sleep(1);
        attente++;
        if(attente == 4){
            perror("tentative de connection");
        }
    }
    
    //if (connect(sock, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1)
    //{
        //perror("tentative de connection");
        //exit(1);
    //}
    return sock;
}

/* Demander des informations au bar */
void recupererInformation()
{
    sock = creerSocketTCP();
    struct requete req;
    char *message;
    int taille_msg;
    req.type_requete = INFORMATIONS;
    req.taille_requete = sizeof(int);
    taille_msg = sizeof(struct requete);
    message = (char *)malloc(taille_msg);
    tireuse = malloc(2*sizeof(struct tireuse));
    memcpy(message, &req, sizeof(struct requete));
    if (write(sock, message, taille_msg) <= 0)
    {
        //free(message);
        //exit(1);
        perror("Erreur envoie recup Info\n");
    }
    if ((read(sock, tireuse, 2 * sizeof(struct tireuse))) <= 0)
    {
        //free(message);
        perror("Recuperer tireure\n");
        //exit(1);
    }
    free(message);
}
void demanderInformation()
{
    recupererInformation();
    printf("Les bières disponibles sont : \n");
    for (int i = 0; i < 2; i++)
    {
        printf("La bière : %s\nCouleur : %s\nQuantité restante : %f L\n", tireuse[i].nom_biere, tireuse[i].type_biere, tireuse[i].quantite_biere);
    }
}

void commanderBiere(struct tireuse* tireuse, int numTireuse, float volumeBiere)
{
    struct requete req;
    req.taille_requete = sizeof(struct requete);
    req.type_requete = COMMANDE;
    /* création de la structure pour demander la bière concerné */
    struct tireuse tireuse_commande;
    //char* reponse[] = {"Valider", "Bière indisponible", "Quantité disponible insuffisante"};
    tireuse_commande = tireuse[numTireuse];
    printf("Demande de %s\n", tireuse_commande.nom_biere);
  
    tireuse_commande.quantite_biere = volumeBiere;
    sock = creerSocketTCP();
    struct reponse rep;
    //char* message;
    if(write(sock, &req, sizeof(struct requete))<= 0)
    {
        
        exit(1);
    }
    
    if(read(sock, &req, sizeof(struct requete)) <= 0){
        exit(0);
    }
    
    if(write(sock, &tireuse_commande, sizeof(struct tireuse))<= 0)
    {
        
        exit(1);
    }
     
    if(read(sock, &rep, sizeof(struct reponse)) <= 0){
        exit(0);
    }
    if(rep.type_reponse == VALIDER){
        printf("\nVoici votre %s\n", tireuse_commande.nom_biere);
    }
    else if(rep.type_reponse == BIERE_INDISPONIBLE){
        printf("\n La bière demandée n'est plus disponible\n");
    }
    else if(rep.type_reponse == QUANTITE_INDISPONIBLE){
        printf("\n Il ne nous reste pas assez de bière pour remplir votre verre\n");
    }


    
}
// Choix pour annuler ou partir
void choixAnnuler()
{
    printf("[r] Je souhaite revenir en arrière\n");
    printf("[q] Je souhaite quitter l'établissement\n");
}

// Choix type de bière
void choixTypeBiere()
{
    /* printf("[b] Je souhaite une bière blonde\n");
    printf("[a] Je souhaite une bière ambrée\n"); */
    printf("[a] Je souhaite une bière %s\n", tireuse[0].nom_biere);
    printf("[b] Je souhaite une bière %s\n", tireuse[1].nom_biere);
    choixAnnuler();
}

void choixBiereBlonde()
{
    /* printf("[p] Je souhaite une Paix Dieu\n");
    printf("[g] Je souhaite une Goudale\n");
    printf("[d] Je souhaite une Delirium Tremens\n"); */
    printf("[a] Je souhaite une %s\n", tireuse[1].nom_biere);
    choixAnnuler();
}
void choixBiereAmbree()
{
    printf("[k] Je souhaite une Kwak\n");
    printf("[m] Je souhaite une Mousse Ta Shuc\n");
    printf("[c] Je souhaite une Queue de Charrue\n");
    choixAnnuler();
}

void choixVolumeBiere()
{
    printf("[a] Je souhaite une pinte\n");
    printf("[b] Je souhaite une demi\n");
    choixAnnuler();
}

void choixIncorrect(){
    printf("\nVeuillez m'excusez je n'ai pas compris votre demande\n");
}

char partir()
{
    return 'q';
}

int main(int args, char** argv)
{
    //static struct sockaddr_in addr_serveur; // socket d'écoute du serveur
    struct hostent *host_serveur;           // machine où tourne le serveur
    char reponse_client;
    char nom_biere;
    char type_biere;
    float volume_biere;
    int nb_octets;
    int demande;
    char reponse[TAILLEBUF];
    port = malloc(sizeof(argv[2]));
    addr = malloc(sizeof(argv[1]));
    port = argv[2];
    addr = argv[1];
    recupererInformation();

    printf("Bienvenue au bar rantanplan\n");
    do
    {

        printf("Que puis-je pour vous ?\n");
        printf("[d] Demander les bières disponible\n");
        printf("[c] Commander une bière\n");
        printf("[q] Quitter\n");
        scanf(" %99s", &reponse_client);
        if (strcmp(&reponse_client, "d")==0)
        {
            demanderInformation();
        }
        else if (strcmp(&reponse_client, "c")==0)
        {
            choixTypeBiere();
            scanf(" %c", &reponse_client);
            if(strcmp(&reponse_client, "a")==0){
            
                choixVolumeBiere();
                scanf(" %c", &reponse_client);
                if (strcmp(&reponse_client, "a")==0)
                {
                    volume_biere = reponse_client;
                    commanderBiere(tireuse, 0, 0.5);
                }
                else if(strcmp(&reponse_client, "b")==0){
                    commanderBiere(tireuse, 0, 0.25);
                }
                else if(strcmp(&reponse_client, "r")!=0 && strcmp(&reponse_client, "q")!=0){
                    choixIncorrect();
                }
            }
        
            else if (strcmp(&reponse_client, "b")==0)
            {
                choixVolumeBiere();
                scanf(" %c", &reponse_client);
                if (strcmp(&reponse_client, "a")==0)
                {
                    volume_biere = reponse_client;
                    commanderBiere(tireuse, 1, 0.5);
                }
                else if(strcmp(&reponse_client, "b")==0){
                    commanderBiere(tireuse, 1, 0.25);
                }
                else if(strcmp(&reponse_client, "r")==0 && strcmp(&reponse_client, "q")!=0){
                    choixIncorrect();
                }
            }
            else if(strcmp(&reponse_client, "r")!=0 && strcmp(&reponse_client, "q")!=0){
                choixIncorrect();
            }
        }
        else if (strcmp(&reponse_client, "q")!=0)
        {
            printf("\n\nJe n'ai pas compris votre demande\n\n");
        }
        while (getchar() != '\n');
    } while (strcmp(&reponse_client, "q")!=0);

    close(sock);
}

/* do{

        printf("Que puis-je pour vous ?\n");
        printf("[d] Demander les bières disponible\n");
        printf("[c] Commander une bière\n");
        printf("[q] Quitter\n");
        scanf(" %c", &reponse_client);
        if(reponse_client == 'd'){
            demanderInformation();
        }
        else if(reponse_client == 'c'){
            choixTypeBiere();
            scanf(" %c", &reponse_client);
            if(reponse_client == 'b'){
                type_biere = reponse_client;
                choixBiereBlonde();
                scanf(" %c", &reponse_client);
                if(reponse_client == 'p' || reponse_client == 'g' || reponse_client == 'd'){
                    nom_biere = reponse_client;
                    choixVolumeBiere();
                    scanf(" %c", &reponse_client);
                    if(reponse_client == 'p' || reponse_client == 'd'){
                        volume_biere = reponse_client;
                        commanderBiere(type_biere, nom_biere, volume_biere);
                    }

                }
            }
            else if(reponse_client == 'a'){
                type_biere = reponse_client;
                choixBiereAmbree();
                scanf(" %c", &reponse_client);
                if(reponse_client == 'k' || reponse_client == 'm' || reponse_client == 'c'){
                    nom_biere = reponse_client;
                    choixVolumeBiere();
                    scanf(" %c", &reponse_client);
                    if(reponse_client == 'p' || reponse_client == 'd'){
                        volume_biere = reponse_client;
                        commanderBiere(type_biere, nom_biere, volume_biere);
                    }

                }
            }
        }
        else if(reponse_client != 'q'){
            printf("\n\nJe n'ai pas compris votre demande\n\n");
        }
        while (getchar() != '\n');

    }while(reponse_client != 'q'); */
