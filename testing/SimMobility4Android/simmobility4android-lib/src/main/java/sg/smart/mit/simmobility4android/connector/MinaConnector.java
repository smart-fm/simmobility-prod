/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.connector;

import com.google.gson.JsonObject;
import java.net.InetSocketAddress;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.mina.core.future.ConnectFuture;
import org.apache.mina.core.future.WriteFuture;
import org.apache.mina.core.service.IoConnector;
import org.apache.mina.core.service.IoHandler;
import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolCodecFilter;
import org.apache.mina.filter.codec.textline.TextLineCodecFactory;
import org.apache.mina.filter.logging.LoggingFilter;
import org.apache.mina.transport.socket.nio.NioSocketConnector;
import sg.smart.mit.simmobility4android.handler.Handler;
import sg.smart.mit.simmobility4android.handler.HandlerFactory;
import sg.smart.mit.simmobility4android.handler.JsonHandlerFactory;
import sg.smart.mit.simmobility4android.listener.MessageListener;

/**
 *
 * @author gandola, vahid
 */
public class MinaConnector implements Connector {

    private IoConnector connector;
    private volatile boolean connected;
    private IoSession session;
    private MessageListener messageListener;
    private HandlerFactory handlerFactory;
    private int clientID;
    private ArrayList<JsonObject> sendBuffer;//contains Gson's JsonObjects
    private final int BUFFER_SIZE = 2048;
    private final Logger LOG = Logger.getLogger(getClass().getCanonicalName());
    public MinaConnector(int clientID_) {
        clientID = clientID_;
        this.handlerFactory = new JsonHandlerFactory();
        sendBuffer = new ArrayList<JsonObject>();
    }
    @Override
    public ArrayList<JsonObject>  getSendBuffer()
    {
        return sendBuffer;
    }
    
    @Override
    public void insertSendBuffer(JsonObject jsonObject)
    {
        sendBuffer.add(jsonObject);
    }
    @Override
    public void connect(String host, int port) {
        if (!connected) {
            if (session != null && session.isConnected()) {
                session.close(true);
            }
            if (connector != null) {
                connector.dispose();
            }
            connector = new NioSocketConnector();
            connector.getSessionConfig().setUseReadOperation(true);
            connector.getSessionConfig().setReadBufferSize(BUFFER_SIZE);
            
            //connector.getFilterChain().addLast("logger", new LoggingFilter());
            TextLineCodecFactory tt = new TextLineCodecFactory(Charset.forName("UTF-8"));
            tt.setDecoderMaxLineLength(6000);
            tt.setEncoderMaxLineLength(6000);
            connector.getFilterChain().addLast("codec", new ProtocolCodecFilter(tt));
            connector.setHandler(new IoHandler() {
                @Override
                public void sessionCreated(IoSession is) throws Exception {
                    session = is;
                    connected = true;
                    LOG.info("client "+ clientID + " Session Created");
                }

                @Override
                public void sessionOpened(IoSession is) throws Exception {
                    session = is;
                    LOG.info("client "+ clientID + " Session Opened");
                }

                @Override
                public void sessionClosed(IoSession is) throws Exception {
                     LOG.info("client "+ clientID + " Session Closed");
                   
                }

                @Override
                public void sessionIdle(IoSession is, IdleStatus is1) throws Exception {
                     LOG.info("client "+ clientID + " Session Idle");
                }

                @Override
                public void exceptionCaught(IoSession is, Throwable thrwbl) throws Exception {
                    LOG.info("client "+ clientID + " got an Exception" + thrwbl.toString());
                }

                @Override
                public void messageReceived(IoSession is, Object o) throws Exception {
                  LOG.info("client "+ clientID + " packet received'" + o.toString() + "' size = " + o.toString().length());
                    char[] chars = new char[o.toString().length() - 8];
                    o.toString().getChars(8, o.toString().length(), chars, 0);
                    String str = new String(chars, 0, chars.length);
                    MinaConnector.this.messageListener.onMessage(str);
                }

                @Override
                public void messageSent(IoSession is, Object o) throws Exception {
                    LOG.info(String.format("client %d sent Message: '%s' was sent.", clientID, o));
                }
            });
            ConnectFuture future ;
            do{
                future = connector.connect(new InetSocketAddress(host, port));
            if (!connected == true) {
               LOG.info("Failed attemp to connect") ;
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException ex) {
                        Logger.getLogger(MinaConnector.class.getName()).log(Level.SEVERE, null, ex);
                    }
            }
            else {
               LOG.info("Success attemp to connect") ;
//               connected = true; //todo: is this a good place to set the flag?
            }
            }while(!connected);
                
            
            future.awaitUninterruptibly();
//            if (future.isConnected()) {
//                session = future.getSession();
//                session.getConfig().setUseReadOperation(true);
//                session.getCloseFuture().awaitUninterruptibly();
//
//            }                      
        }
    }

    @Override
    public void disconnect() {
        if (connected) {
            if (session != null) {
                session.close(true);
                session.getCloseFuture().awaitUninterruptibly();
                session = null;
                connector.dispose();
            }
            connected = false;
        }
    }

    @Override
    public void send(Object data) {
//        System.out.println("outgoing data : " + data.toString());
        
        if(connected) 
        {
//            System.out.println("we are connected");
        }
        else
        {
            System.out.println("client "+ clientID + " we are NOT connected");
        }
        if(data != null) 
        {
//            System.out.println("data not null");
        }
        else
        {
          System.out.println("client "+ clientID + " data is null");  
        }
        if(session != null) 
        {
//            System.out.println("session not null");
        }  
        else
        {
            System.out.println("client "+ clientID + " session is null");
        }
        
        if(session.isConnected())
        {
//            System.out.println("session.isConnected");
        }
        else
        {
            System.out.println("client "+ clientID + " session is not Connected");
        }
        
        if (connected && data != null && session != null && session.isConnected()) {
//            String str = "{\"PACKET_HEADER\":{\"NOF_MESSAGES\":\"1\"},\"DATA\":[{\"SENDER\":\"110\",\"SENDER_TYPE\":\"ANDROID_EMULATOR\",\"MESSAGE_TYPE\":\"READY_ACK\"}]}";
            int i = data.toString().length()+1;
            String str = String.format("%8h%s",i , data.toString());
            System.out.println("Sending:" + str);
            System.out.println("Declared size for \n'" + data.toString() + "' \n is : '" + i + "'");
            
            session.write(str);
            
        }
    }

    @Override
    public void listen(MessageListener listener) {
        messageListener = listener;
    }
}
