package sim_mob;

import java.awt.*; 
import javax.swing.*; 

public class TrafficFrame extends JFrame{
	
	private static final long serialVersionUID = 1L;

	public static void main(String[] args) { 
		
		TrafficFrame frame = new TrafficFrame();
		
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); 
		
		frame.setVisible(true); 
	}
	
	
	// Constructor
	public TrafficFrame() { 

		// Set title
		super("Traffic Signal Animation");  
		
		// Set size
		setSize(1200, 800); 
		TrafficPanel trafficSignal = new TrafficPanel(); 
		Container contentPane= getContentPane();   
		contentPane.add(trafficSignal);   
	} 
	
}
