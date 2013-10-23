package sg.smart.mit.simmobility4android.connector;

import com.google.gson.JsonObject;
import java.util.ArrayList;
import sg.smart.mit.simmobility4android.listener.MessageListener;

/**
 * Interface for connector implementation.
 */
public interface Connector {
    /**
     * Connects to server. 
     * @param host of the server.
     * @param port of the server.
     */
    void connect(String host, int port);
    
    /**
     * Disconnects from the server.
     * @param host
     * @param port
     */
    void disconnect();
    
    void insertSendBuffer(JsonObject data);
    ArrayList<JsonObject>  getSendBuffer();
    
    /**
     * Sends the given object.
     * @param data
     */
    void send(Object data);
    
    void listen(MessageListener listener);
}
