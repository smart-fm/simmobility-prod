/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.MulticastMessage;

/**
 *
 * @author vahid
 */
public class MulticastHandler extends Handler<MulticastMessage> {
    private MulticastMessage message;
    private int temp_clientID;
    public MulticastHandler(MulticastMessage message_, Connector connector, int temp_clientID_) {
        super (message_, connector);
        message = message_;
        temp_clientID = temp_clientID_;
    }

    @Override
    public void handle() {
//        System.out.println("handler for client " + temp_clientID + " Msg: '" + message.MULTICAST_DATA);
//        throw new UnsupportedOperationException("Not supported yet.");
    }
    
}
