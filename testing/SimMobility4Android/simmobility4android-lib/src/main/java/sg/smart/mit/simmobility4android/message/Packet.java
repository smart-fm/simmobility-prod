/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.message;
import java.util.List;
import sg.smart.mit.simmobility4android.message.PacketHeader;
import sg.smart.mit.simmobility4android.message.Message;

/**
 *
 * @author vahid
 */
public class Packet {
    public PacketHeader PACKET_HEADER;
    public Message[] DATA;
}
