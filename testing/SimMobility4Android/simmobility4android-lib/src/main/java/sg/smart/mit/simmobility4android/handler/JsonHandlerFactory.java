/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import java.util.ArrayList;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.*;

/**
 *
 * @author gandola, vahid
 */
public class JsonHandlerFactory implements HandlerFactory {
//    public Gson gson;

    public boolean canSendMessage;

    public JsonHandlerFactory() {
        canSendMessage = false;
//        gson = new GsonBuilder().registerTypeAdapter(TimeMessage.class, new TimeMessageDeserializer()).create();
    }

    @Override
    public ArrayList<Handler> create(Connector connector, Object packet, int clientID) {
        ArrayList<Handler> handlers = new ArrayList<Handler>();
//        System.out.println("client " + clientID + "in JsonHandlerFactory.create");
        Gson gson = new Gson();
//        Packet pack = gson.fromJson(packet.toString(), Packet.class);
        //test
        JsonParser parser = new JsonParser();
        JsonObject jo = (JsonObject) parser.parse(packet.toString());
        JsonElement header_ele = jo.get("PACKET_HEADER");
        PacketHeader PH = gson.fromJson(header_ele, PacketHeader.class);
//        System.out.println("Number of messages : " + PH.NOF_MESSAGES);
        JsonArray ja = jo.getAsJsonArray("DATA");
//        System.out.println("client " + clientID + " DATA size is: '" + ja.size());
        int i = 0;
        for (JsonElement je : ja) {
            //first time parsing of the message
            Message msg = gson.fromJson(je, Message.class);
//            System.out.println("the type is: " + msg.getMessageType().toString() + 
//                    "Sender:  " + msg.SENDER + "  Sender_type: " + msg.SENDER_TYPE);
            //second time parsing of the message
//            System.out.println("client " + clientID + " msg.getMessageType(" + msg.getMessageType().toString() + ")");
            switch (msg.getMessageType()) {
                
                case WHOAREYOU: {
                    handlers.add(new WhoAreYouHandler(null, connector, clientID));
                    break;
                }

                case TIME_DATA: {
                    TimeMessage res = gson.fromJson(je, TimeMessage.class);
//                    System.out.println("client " + clientID + ": TimeMessage 's tick is : " + res.getTick());
//                    System.out.println("returning a TimeHandler");
                    handlers.add(new TimeHandler(res, connector, clientID));
                    break;
                }

                case READY:
                    canSendMessage = true;
                    handlers.add(new ReadyHandler(null, connector,clientID));
                    break;

                case LOCATION_DATA: {
                    LocationMessage res = gson.fromJson(je, LocationMessage.class);
//                    System.out.println("client " + clientID + ": LocationMessage is : (" + res.getX() + "," + res.getY() + ")");
                    handlers.add(new LocationHandler(res, connector));
                    break;
                }
                case MULTICAST:{
                    MulticastMessage res = gson.fromJson(je, MulticastMessage.class);
//                    System.out.println("client " + clientID + "MulticastMessage 's data is : " + res.MULTICAST_DATA);
                    handlers.add(new MulticastHandler(res, connector, clientID));
                    break;                    
                }
                case READY_TO_RECEIVE:{
                    ReadyToReceiveMessage res = gson.fromJson(je, ReadyToReceiveMessage.class);
//                    System.out.println("client " + clientID + "got ReadyToReceiveMessage");
                    handlers.add(new ReadyToReceiveHandler(res, connector, clientID));
                    break;
                }
                default:
                    System.out.println("Switch " + msg.getMessageType() + " is invalid");
                    return null;
            }
        }
        return handlers;

    }
}
