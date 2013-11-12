/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package sg.smart.mit.simmobility4android.message;

/**
 *
 * @author vahid
 */
public class LocationMessage extends Message{
    
    private int x;
    private int y;
    
    public int getX(){
        return x;
    }
    
    public int getY(){
        return y;
    }
}
