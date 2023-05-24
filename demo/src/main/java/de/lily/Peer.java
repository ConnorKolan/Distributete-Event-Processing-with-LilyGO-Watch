package de.lily;
import java.util.Scanner;

import de.lily.WifiInterface.ClientThread;
import de.lily.WifiInterface.ConnectionHandler;

public class Peer{
    public boolean runThread = true;
    public static void main(String[] args) {
        Peer thisPeer = new Peer();

        //Get Port for Server from User
        int port = 5555;

        Scanner in = new Scanner(System.in);
        System.out.println("Select Port to listen on(default: 5555; 'none' for no listening)");   
        String input = in.nextLine();
        if (!input.equals("none")){
            try {
                port = Integer.parseInt(input);
            } catch (NumberFormatException e) {}
        }
        ConnectionHandler serverThread = new ConnectionHandler(port, thisPeer);
        serverThread.start();

        //-----------------------------------------------------
        //Get Port to connect to from User
        
        System.out.println("Select IP to connect to ('none' if you dont want to connect to anyone)");
        String ip = in.nextLine();
        if (!ip.equals("none")){
            System.out.println("Select Port to connect to (default: 80)");
            try {
                port = Integer.parseInt(in.nextLine());
            } catch (NumberFormatException e) {
                port = 80;
            }
            in.close();
    
            System.out.println("Listening on port " + port);
            System.out.println("Connecting to " + ip + ":" + port);
    
            ClientThread clientThread = new ClientThread(port, thisPeer, ip);
            clientThread.start();
        }
        //------------------------------------------------------

        boolean ServerAlive = true;
        while (ServerAlive) {
            if(!serverThread.isAlive()){
                ServerAlive = false;
                thisPeer.runThread = false;
            }
        }
    }
}
