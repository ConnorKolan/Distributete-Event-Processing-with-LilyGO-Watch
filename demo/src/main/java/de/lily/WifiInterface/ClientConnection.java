package de.lily.WifiInterface;

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.InputStreamReader;
import java.net.Socket;

public class ClientConnection extends Thread{
    private Socket socket;
    private String name;

    public ClientConnection(String name, Socket socket){
        this.socket = socket;
        this.name = name;
    }

    @Override
    public void run() {
        try {
            System.out.println(this.name + ": Able to recieve Msgs");
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            String input = "";
            while ((input = reader.readLine()) != null && !input.equals("close")) {
                System.out.println(input);
            }

            System.out.println(this.name + ": Connection closed");

        } catch (EOFException eofException){
            System.out.println("Chat wurde geschlossen");
        } catch (Exception e) {
            System.out.println("Server: " + e);
        }

        /*TODO Verbindung mit mehreren Peers zulassen*/
    }
}