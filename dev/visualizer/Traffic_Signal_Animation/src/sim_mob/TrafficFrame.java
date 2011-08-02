package sim_mob;

import java.awt.*; 
import javax.swing.*; 

public class TrafficFrame extends JFrame{
	
	private static final long serialVersionUID = 1L;

	public static void main(String[] args) {
		String inputFile = args.length>0 ? args[0] : "input.txt";
		
		TrafficFrame frame = new TrafficFrame(inputFile);
		
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); 
		
		frame.setVisible(true); 
	}
	
	
	// Constructor
	public TrafficFrame(String inFileName) { 

		// Set title
		super("Traffic Signal Animation");  
		
		// Set size
		setSize(1200, 800); 
		TrafficPanel trafficSignal = new TrafficPanel(inFileName); 
		Container contentPane= getContentPane();   
		contentPane.add(trafficSignal);   
	} 
	
}
