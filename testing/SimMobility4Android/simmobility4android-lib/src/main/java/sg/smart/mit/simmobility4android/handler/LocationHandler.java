/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.handler;
import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.LocationMessage;
/**
 *
 * @author vahid
 */
public class LocationHandler extends Handler<LocationMessage> {
    private LocationMessage message;
    public LocationHandler(LocationMessage message_, Connector connector)
    {
        super(message_, connector);
        message = message_;
    }

    @Override
    public void handle() {
        
        /*
         * if you need to use the simMobility's information about
         * your current location, here is your chance.
         * you have a "LocationMessage message" or "LocationMessage getMessage()"
         * filled with the data you need
         */  
        LocationMessage message = getMessage();
        System.out.println("Current location is "+ message.getX() + ":" + message.getY());
        
    }
    
}
