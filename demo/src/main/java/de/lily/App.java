package de.lily;
import java.util.Scanner;

import de.lily.WifiInterface.ClientThread;
import de.lily.WifiInterface.ConnectionHandler;

import io.siddhi.core.SiddhiAppRuntime;
import io.siddhi.core.SiddhiManager;
import io.siddhi.core.event.Event;
import io.siddhi.core.stream.input.InputHandler;
import io.siddhi.core.stream.output.StreamCallback;


public class App{
    public boolean runThread = true;
    public static void main(String[] args) {
        App thisPeer = new App();
        Scanner in = new Scanner(System.in);

        //-----------------------------------------------------
        //init siddhi
        String siddhiApp = "" +
                        "define stream StockStream (symbol string, price int, volume int); " +
                        "" +
                        "@info(name = 'query1') " +
                        "from StockStream[volume < 150] " +
                        "select symbol, price " +
                        "insert into OutputStream;";

        System.out.println("Siddhi querry (event data needs to be ;-seperated):");
        System.out.println(siddhiApp);

        //Siddhi
        SiddhiManager siddhiManager = new SiddhiManager();
        
        //Generate runtime
        SiddhiAppRuntime siddhiAppRuntime = siddhiManager.createSiddhiAppRuntime(siddhiApp);

        //Adding callback to retrieve output events from stream
        siddhiAppRuntime.addCallback("OutputStream", new StreamCallback() {
            @Override
            public void receive(Event[] events) {
                //To convert and print event as a map
                for (Event event : events) {
                    for (Object string : event.getData()) {
                        System.out.print(string + " ");
                    }
                }
                System.out.println();
            }
        });

        //Get InputHandler to push events into Siddhi
        InputHandler inputHandler = siddhiAppRuntime.getInputHandler("StockStream");

        //Start processing
        siddhiAppRuntime.start();


        //-----------------------------------------------------
        //Get Port for Server from User
        int port = 5555;

        System.out.println("Select Port to listen on(default: 5555; 'none' for no listening)");   
        String input = in.nextLine();
        if (!input.equals("none")){
            try {
                port = Integer.parseInt(input);
            } catch (NumberFormatException e) {}
        }
        ConnectionHandler serverThread = new ConnectionHandler(port, thisPeer, inputHandler);
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