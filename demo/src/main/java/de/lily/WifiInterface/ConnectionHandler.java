package de.lily.WifiInterface;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import de.lily.App;
import io.siddhi.core.stream.input.InputHandler;

public class ConnectionHandler extends Thread{
    private int port;
    private App peer;
    private InputHandler inputHandler;

    public ConnectionHandler(int port, App peer, InputHandler inputHandler){
        this.port = port;
        this.peer = peer;
        this.inputHandler = inputHandler;
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
                    ClientConnection cConnection = new ClientConnection("Thread-" + numberOfThreads++ , s, inputHandler);
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
