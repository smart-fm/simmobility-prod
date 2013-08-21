/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;

class ServerThread extends Thread {

    private Socket s = null;

    // private int index = 0;
    public ServerThread(Socket s) {
        this.s = s;
    }

    public void run() {
        System.out.println("Accept Client Connection......");
        try {
            if (s != null) {
                while (!s.isClosed()) {
                    OutputStream out = s.getOutputStream();
                    String str = "{\"MessageType\":\"WhoAreYou\"}\n";
                    out.write(str.getBytes());
                    System.out.println("Message was sent");
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException ex) {
                        Logger.getLogger(ServerThread.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            }
        } catch (IOException ioe) {
            System.out.println(ioe.getMessage());
        }
    }
}

public class Server {

    public static void main(String[] args) throws IOException {

        int connectCount = 10;
        int index = 0;

        Socket[] sArray = new Socket[connectCount];
        ServerSocket server = new ServerSocket();
        InetSocketAddress address = new InetSocketAddress("localhost", 7999);
        server.bind(address);

        if (server.isBound()) {
            System.out.println("Waitting Client Connection.......");
            while (index < 10) {
                sArray[index] = server.accept();
                ServerThread smartHost = new ServerThread(sArray[index]);
                index++;
                smartHost.start();
            }
            System.out.println("The Max Connection Count reached......");
        }
        System.out.println("Complete to Send the Msg......");
    }
}