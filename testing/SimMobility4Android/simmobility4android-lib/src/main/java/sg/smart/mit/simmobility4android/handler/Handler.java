package sg.smart.mit.simmobility4android.handler;

import sg.smart.mit.simmobility4android.connector.Connector;
import sg.smart.mit.simmobility4android.message.Message;

/**
 * Interface for all handler implementations
 *
 * @author gandola
 */
public abstract class Handler<T extends Message> {

    private T message;
    private Connector connector;

    public Handler(T message, Connector connector) {
        this.message = message;
        this.connector = connector;
    }

    public abstract void handle();

    protected Connector getConnector() {
        return connector;
    }

    public T getMessage() {
        return message;
    }
}
