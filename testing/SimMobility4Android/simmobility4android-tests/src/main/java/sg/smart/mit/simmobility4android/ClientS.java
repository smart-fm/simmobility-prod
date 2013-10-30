/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android;

import java.util.List;
import sg.smart.mit.simmobility4android.Client;
import java.io.IOException;
import java.lang.reflect.Array;
import java.util.HashMap;
import java.util.Set;
import java.util.TreeSet;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 *
 * @author vahid
 */
public class ClientS {

    public static HashMap<Integer, Client> clientList;

    public ClientS() {
    }

    public static void main(String[] args) throws IOException {
        clientList = new HashMap<Integer, Client>();
        ExecutorService service = Executors.newFixedThreadPool(10);
        for (int i = 110; i < 125; i++) {
            Client client = new Client(i);
            clientList.put(i, client);
            service.execute(client);
        }
    }
}
