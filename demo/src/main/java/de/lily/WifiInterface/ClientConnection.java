package de.lily.WifiInterface;

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.InputStreamReader;
import java.net.Socket;

import io.siddhi.core.stream.input.InputHandler;

public class ClientConnection extends Thread{
    private Socket socket;
    private String name;
    private InputHandler inputHandler;

    public ClientConnection(String name, Socket socket, InputHandler inputHandler){
        this.socket = socket;
        this.name = name;
        this.inputHandler = inputHandler;
    }

    @Override
    public void run() {
        try {
            System.out.println(this.name + ": Able to recieve Msgs");
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            String input = "";
            while ((input = reader.readLine()) != null && !input.equals("close")) {
                inputHandler.send(parseMessage(input));
            }

            System.out.println(this.name + ": Connection closed");

        } catch (EOFException eofException){
            System.out.println("Chat wurde geschlossen");
        } catch (Exception e) {
            System.out.println("Server: " + e);
        }
    }

    private Object[] parseMessage(String message){
        String[] csv = message.split(";");
        Object[] obj = new Object[]{csv[0], Integer.parseInt(csv[1]), Integer.parseInt(csv[2])};
        return obj;
    }
}