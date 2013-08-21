package sg.smart.mit.simmobility4android.handler;

import java.util.ArrayList;
import sg.smart.mit.simmobility4android.connector.Connector;

public interface HandlerFactory {

    ArrayList<Handler> create(Connector connector, Object message, int ID);
}
