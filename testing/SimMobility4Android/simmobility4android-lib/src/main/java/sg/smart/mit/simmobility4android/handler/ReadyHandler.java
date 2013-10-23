/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;
import java.util.ArrayList;
import java.util.List;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.ReadyMessage;

/**
 *
 * @author vahid
 */
public class ReadyHandler extends Handler<ReadyMessage> {
    private int temp_clientID;
    public ReadyHandler(ReadyMessage message, Connector connector, int temp_clientID_)
    {
        super(message, connector);
        temp_clientID = temp_clientID_;
    }
    @Override
    public void handle()
    {
        /*set, if you want your emulator set any flag to make sure 
         * it is successfully registered with the simMobility server(Broker)
         * here is your chance
         */
        System.out.println("Server Knows " + temp_clientID + " now.");
        
//        JsonObject packet = new JsonObject();
//        JsonObject packetHeader = new JsonObject();
//        packetHeader.addProperty("NOF_MESSAGES", String.valueOf(1));
//        packet.add("PACKET_HEADER", packetHeader);
//        System.out.println(packet.toString());
//        JsonArray packetMessageArray = new JsonArray();
//        
//        
//        JsonObject msg = new JsonObject();
//        msg.addProperty("SENDER", String.valueOf(temp_clientID));
//        msg.addProperty("SENDER_TYPE", "ANDROID_EMULATOR");
//        msg.addProperty("MESSAGE_TYPE", "READY_ACK");
//        
//        packetMessageArray.add(msg);
//        packet.add("DATA", packetMessageArray);
//        
//        /*
//         * seth please add your emulator's ID here. 
//         * Note that it will be received as unsidned int 
//         * on the other side
//         */
//        System.out.println("ReadyHandler made:" + packet.toString());
//        getConnector().send(packet.toString());
    }
    
}
