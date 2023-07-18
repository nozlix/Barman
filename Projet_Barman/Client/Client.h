int creerSocketTCP();

enum requete_t {
    INFORMATIONS = 0,
    COMMANDE,
    ANNULE
};
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