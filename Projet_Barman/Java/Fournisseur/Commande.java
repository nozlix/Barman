// Compilation : javac Commande.java
/* Execution : 
   1. Lancer Fournisseur.java
   2. Dans un autre terminal, lancer : java Commande 127.0.0.1 7777 */

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.Vector;

public class Commande {

    public static void main(String argv[]) throws SocketException, IOException, ClassNotFoundException {
        System.out.println("Le processus de commande est lancé...");

            // ----- SOCKET UDP -----
            DatagramSocket socket = new DatagramSocket(Integer.parseInt(argv[1]));
            byte[] data = new byte[20];
            DatagramPacket packet = new DatagramPacket(data, data.length);
            String response = "Message reçu\n";
            String bieres_ambrees = "";
            String bieres_blondes = "";

        
        while (true) {
        
        
            // Récupération et affichage des données (une chaîne de caractères)
            socket.receive(packet);
            String chaine = new String(packet.getData(), 0, packet.getLength());
            System.out.println("Message reçu : " + chaine);
            
            // ----- JAVA RMI -----
            try {
                // On récupère une référence sur l'objet distant nommé "DedeLaChope" via
                // le registry de la machine sur laquelle il s'exécute
                IBiere opBiere = (IBiere) Naming.lookup("rmi://" + argv[0] + "/DedeLaChope");

                // On récupère la liste des bières blondes
                Vector<Biere> blondes = opBiere.listeBlondes();
                StringBuilder bb = new StringBuilder();
                for (Biere b : blondes) {
                    bb.append(b.nom).append(", ");
                }
                if (bb.length() > 0) {
                    bb.deleteCharAt(bb.length() - 2); // Supprime la dernière virgule et l'espace après
                }
                bieres_blondes = bb.toString();

                // On récupère la liste des bières ambrées
                Vector<Biere> ambrees = opBiere.listeAmbrees();
                StringBuilder ba = new StringBuilder();
                for (Biere b : ambrees) {
                    ba.append(b.nom).append(", ");
                }
                if (ba.length() > 0) {
                    ba.deleteCharAt(ba.length() - 2); // Supprime la dernière virgule et l'espace après
                }
                bieres_ambrees = ba.toString();

                // L'utilisateur veux la liste des bières blondes
                if (chaine.equals("LISTEBLONDE")) {
                    response = bieres_blondes;
                    System.out.println("Reponse : " + response + "\n");
                }

                // L'utilisateur veux la liste des bières ambrées
                if (chaine.equals("LISTEAMBREE")) {
                    response = bieres_ambrees;
                    System.out.println("Reponse : " + response + "\n");
                }

                // L'utilisateur veux acheter un fut de bière ambrée
                if (chaine.equals("ACHETERAMBREE")) {
                    response = "\n\nBières ambrées :\n   [1] Kwak\n   [2] Mousse Ta Shuc\n   [3] Queue de Charrue\n\nQuelle bière voulez-vous acheter ? ";

                    // Envoie du nouveau paquet
                    packet.setData(response.getBytes());
                    packet.setLength(response.getBytes().length);
                    socket.send(packet);

                    // Reception du nouveau packet
                    socket.receive(packet);
                    String biere = new String(packet.getData(), 0, packet.getLength());

                    switch (biere) {
                        case "1":
                            biere = "Kwak";
                            break;
                        case "2":
                            biere = "Mousse Ta Shuc";
                            break;
                        case "3":
                            biere = "Queue de Charrue";
                            break;
                    }

                    Biere achat = opBiere.acheterBiere(biere);

                    response = achat.toString();
                    System.out.println("Reponse : " + response + "\n");
                }

                // L'utilisateur veux acheter une bière
                if (chaine.equals("ACHETERBLONDE")) {
                    response = "\n\nBières blondes :\n   [1] Paix Dieu\n   [2] Goudale\n   [3] Delirium Tremens\n\nQuelle bière voulez-vous acheter ?  \n";

                    // Envoie du nouveau paquet
                    packet.setData(response.getBytes());
                    packet.setLength(response.getBytes().length);
                    socket.send(packet);

                    // Reception du nouveau packet
                    socket.receive(packet);
                    String biere = new String(packet.getData(), 0, packet.getLength());

                    switch (biere) {
                        case "1":
                            biere = "Paix Dieu";
                            break;
                        case "2":
                            biere = "Goudale";
                            break;
                        case "3":
                            biere = "Delirium Tremens";
                            break;
                    }

                    Biere achat = opBiere.acheterBiere(biere);

                    response = achat.toString();
                    System.out.println("Reponse : " + response + "\n");
                }

                // Envoie du nouveau paquet
                packet.setData(response.getBytes());
                packet.setLength(response.length());
                socket.send(packet);

                // L'utilisateur quitte le fournisseur
                if (chaine.equals("QUITTER")) {
                    System.exit(0);
                }

            } catch (MalformedURLException | RemoteException | NotBoundException e) {
                System.err.println("Erreur : " + e.getMessage());
            }
        }
    }

}
