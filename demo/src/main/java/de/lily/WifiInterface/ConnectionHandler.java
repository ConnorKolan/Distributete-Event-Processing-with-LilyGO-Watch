package de.lily.WifiInterface;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import de.lily.Peer;

public class ConnectionHandler extends Thread{
    private int port;
    private Peer peer;

    public ConnectionHandler(int port, Peer peer){
        this.port = port;
        this.peer = peer;
    }

    @Override
    public void run() {
        ServerSocket ss = null;
        Socket s = null;
        int numberOfThreads = 0;
        try {
            ss = new ServerSocket(port);
            while (peer.runThread) {
                if (peer.runThread) {
                    s = ss.accept();
                    ClientConnection cConnection = new ClientConnection("Thread-" + numberOfThreads++ , s);
                    cConnection.start();
                }
            }
            s.close();
            ss.close();
        
        } catch (IOException e) {
            System.out.println("Server: " + e);
        }

        /*TODO Verbindung mit mehreren Peers zulassen*/
    }
}
