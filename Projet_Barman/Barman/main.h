int main_main();
void recupDemande(struct donneeClient *donnee);
void traiterCommande(struct client *client, int semid);
struct client *depile();
void empiler(struct donneeClient donnee);