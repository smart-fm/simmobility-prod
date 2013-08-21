/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android;

import java.util.ArrayList;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.connector.MinaConnector;
import sg.smart.mit.simmobility4android.listener.MessageListener;

import sg.smart.mit.simmobility4android.handler.*;

/**
 *
 * @author gandola, vahid
 */
public class Client implements Runnable {

    private static HandlerFactory handlerFactory;
    public int clientID;

    public Client(int clientID) /*throws IOException, InterruptedException*/ {
        this.clientID = clientID;
    }

    @Override
    public void run() {
        final Connector conn = new MinaConnector(clientID);//is this "final" thing correct?
        handlerFactory = new JsonHandlerFactory();
        conn.listen(new MessageListener() {

            @Override
            public void onMessage(Object message) {
                ArrayList<Handler> handlers = handlerFactory.create(conn, message, clientID);
                for(Handler handler: handlers)
                {
                    handler.handle();
                }
            }
        });
        System.out.println("client " + clientID + " connecting...");
        conn.connect("localhost", 6745);
    }
}
