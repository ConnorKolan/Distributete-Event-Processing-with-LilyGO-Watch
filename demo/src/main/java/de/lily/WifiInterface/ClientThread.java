package de.lily.WifiInterface;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;

import de.lily.Peer;

public class ClientThread extends Thread{
    private int port;
    private Peer superPeer;
    private String ip;

    public ClientThread(int port, Peer superPeer, String ip){
        this.port = port;
        this.superPeer = superPeer;
        this.ip = ip;
    }

    @Override
    public void run() {
        try {
            Socket s = new Socket(ip, port);
            DataOutputStream dos = new DataOutputStream(s.getOutputStream());

            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));

            String input = null;
            System.out.println("Client: Ready to send Msgs");
            while (superPeer.runThread) {
                input = br.readLine();
                dos.writeUTF(input);
            }
            s.close();
            dos.flush();
            dos.close();

        } catch (Exception e) {
        }

        System.out.println("Partner hat den Chat geschlossen");
    }
}