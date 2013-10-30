/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.message;

/**
 *
 * @author gandola
 */
public class Message {

    public enum MessageType {
        WHOAREYOU,
        TIME_DATA,
        READY,
        LOCATION_DATA,
        MULTICAST,
        UNICAST,
        READY_TO_RECEIVE
    }

    public MessageType getMessageType() {
        return MESSAGE_TYPE;
    }

    public void setMessageType(MessageType MessageType) {
        this.MESSAGE_TYPE = MessageType;
    }
    
    private MessageType MESSAGE_TYPE;
    public String SENDER;
    public String SENDER_TYPE;
    
}
