/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import java.util.List;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.connector.MinaConnector;
import sg.smart.mit.simmobility4android.message.ReadyToReceiveMessage;

/**
 *
 * @author vahid
 */
public class ReadyToReceiveHandler extends Handler<ReadyToReceiveMessage>  {
    private ReadyToReceiveMessage message;
    private int temp_clientID;
    public ReadyToReceiveHandler(ReadyToReceiveMessage message_, Connector connector_, int temp_clientID_) {
        super (message_, connector_);
        message = message_;
        temp_clientID = temp_clientID_;
    }

    @Override
    public void handle() {
//        throw new UnsupportedOperationException("Not supported yet.");
        System.out.println(temp_clientID + "handling the ReadyToReceive");
        JsonObject packet = new JsonObject();
        JsonObject packetHeader = new JsonObject();
        List<JsonObject> sendBuffer = getConnector().getSendBuffer();
        packet.add("PACKET_HEADER", packetHeader);
        JsonArray packetMessageArray = new JsonArray();
        for(JsonObject msg : sendBuffer)
        {
           packetMessageArray.add(msg); 
        }
        //add a last message saying Done
         
        JsonObject msg = new JsonObject();
        msg.addProperty("SENDER", String.valueOf(temp_clientID));
        msg.addProperty("SENDER_TYPE", "ANDROID_EMULATOR");
        msg.addProperty("MESSAGE_TYPE", "CLIENT_MESSAGES_DONE");
        packetMessageArray.add(msg); 
        /////////////        
        packetHeader.addProperty("NOF_MESSAGES", String.valueOf(packetMessageArray.size()));
        packet.add("DATA", packetMessageArray);
        System.out.println("sending to simmobility '" + packet.toString() + "'");
        getConnector().send(packet.toString());
        sendBuffer.clear(); 
        
    }
    
}
