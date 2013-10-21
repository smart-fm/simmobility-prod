/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;

import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.TimeMessage;
import com.google.gson.*;
import org.apache.commons.codec.binary.Base64;
        
/**
 *
 * @author gandola, vahid
 */
public class TimeHandler extends Handler<TimeMessage>{
    private TimeMessage message;
    private int temp_clientID;
    public TimeHandler(TimeMessage message_, Connector connector, int temp_clientID_) {
        super (message_, connector);
        message = message_;
        temp_clientID = temp_clientID_;
    }

    @Override
    public void handle() {
        /*
         * if you need to use the simMobility current time, here is your chance.
         * you have a "TimeMessage message" filled with the data you need
         */  
        System.out.println("Handling time message for " + temp_clientID);
        
        
        
        
        //for testing purpose, send an announce message on eache tick
        
//        JsonObject packet = new JsonObject();
//        JsonObject packetHeader = new JsonObject();
//        packetHeader.addProperty("SENDER_TYPE", "ANDROID_EMULATOR");
//        packetHeader.addProperty("NOF_MESSAGES", String.valueOf(1));
//        packet.add("PACKET_HEADER", packetHeader);
//        JsonArray packetMessageArray = new JsonArray();
        JsonObject msg = new JsonObject();
        msg.addProperty("SENDER", String.valueOf(temp_clientID));
        msg.addProperty("SENDER_TYPE", "ANDROID_EMULATOR");
        msg.addProperty("MESSAGE_TYPE", "MULTICAST");
        msg.addProperty("MESSAGE_CAT", "APP");
        //put data in base64 format
        String orig = "MULTICAST String from client " + temp_clientID;
        byte[] encoded_byte = Base64.encodeBase64(orig.getBytes()); 
        String encoded_string = new String(encoded_byte);
//        System.out.println("Sample Base64 string=> '" +  encoded_string +"'");
        msg.addProperty("MULTICAST_DATA", encoded_string );
        
//        packetMessageArray.add(msg);
//        packet.add("DATA", packetMessageArray);
        //dont'send directly, send it to a buffer to be sent later
        System.out.println("client " + temp_clientID + "  inserting into buffer");
        getConnector().insertSendBuffer(/*packet*/msg);
    }
}
