import java.rmi.RemoteException;
import java.rmi.Remote;
import java.util.Vector;

public interface IBiere extends Remote {

    // Retourne la liste des bieres blondes
    public Vector<Biere> listeBlondes() throws RemoteException;

    // Retourne la liste des bieres ambrees
    public Vector<Biere> listeAmbrees() throws RemoteException;

    // Retourne un fut de biere de la biere dont on passe le nom en parametre.
    // Si cette biere n'existe pas, retourne <code>null</code>
    public Biere acheterBiere(String nom) throws RemoteException;

}
