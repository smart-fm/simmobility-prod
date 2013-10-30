/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;
import com.google.gson.JsonObject;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import com.google.gson.stream.JsonReader;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import java.util.ArrayList;
import java.util.List;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.WhoAreYouMessage;

/**
 *
 * @author gandola, vahid
 */
public class WhoAreYouHandler extends Handler<WhoAreYouMessage> {
    private int clientID;
    public WhoAreYouHandler(WhoAreYouMessage message, Connector connector, int clientID_) {
        super(message, connector);
        clientID = clientID_;
        System.out.println("creating WhoAreYouHandler");
    }

    @Override
    public void handle() {
        System.out.println("handling WhoAreYouHandler");
        JsonObject packet = new JsonObject();
        JsonObject packetHeader = new JsonObject();
        packetHeader.addProperty("NOF_MESSAGES", String.valueOf(1));
        packet.add("PACKET_HEADER", packetHeader);
        JsonArray packetMessageArray = new JsonArray();
        
        
        JsonObject msg = new JsonObject();
        msg.addProperty("SENDER", String.valueOf(clientID));
        msg.addProperty("SENDER_TYPE", "ANDROID_EMULATOR");
        msg.addProperty("MESSAGE_TYPE", "WHOAMI");
//        JsonObject WHOAMI = new JsonObject();
        msg.addProperty("ID", String.valueOf(clientID));
        msg.addProperty("TYPE", "ANDROID_EMULATOR");
        
        List<String> serviceStrings = new ArrayList<String>();
        serviceStrings.add("SIMMOB_SRV_TIME");
        serviceStrings.add("SIMMOB_SRV_LOCATION");

        Gson gson = new Gson();
        JsonElement element = gson.toJsonTree(serviceStrings, new TypeToken<List<String>>() {}.getType());

//        if (! element.isJsonArray()) {
//            // fail appropriately
//            System.out.println("WhoAreYouHandler is handling5-error");
//        }

        JsonArray serviceArray = element.getAsJsonArray();
        msg.add("REQUIRED_SERVICES", serviceArray);

//        msg.add("WHOAMI", WHOAMI);
        
        packetMessageArray.add(msg);
        packet.add("DATA", packetMessageArray);
        
        /*
         * seth please add your emulator's ID here. 
         * Note that it will be received as unsidned int 
         * on the other side
         */
        System.out.println("WhoAreYouHandler made:" + packet.toString());
        //do not send it to send buffer. 
        //This ispart of the initial authentication protocol 
        //and is different from...from... just different ok!?
        getConnector().send(packet.toString());
    }
    /*
{
    "PACKET_HEADER": {
        "SENDER"  : "client-id-X",
        "SENDER_TYPE" : "client-type-XX",
        "NOF_MESSAGES" : 1,
        "PACKET_SIZE" : 123

    },
    "DATA":[
        {
            "SENDER" : "client-id-X",
            "SENDER_TYPE" : "ANDROID_EMULATOR",
            "MESSAGE_TYPE" : "WHOAMI",
            "WHOAMI" :{
                "ID" : "client-id-X",
                "TYPE" : "ANDROID_EMULATOR",
        	"REQUIRED_SERVICES" : [
                                        "TIME",
                                        "LOCATION"
                                       ]
            }

        }
        ]
}
     */
}
