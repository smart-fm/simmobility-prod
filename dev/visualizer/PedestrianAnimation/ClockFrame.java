import java.awt.*; 
import javax.swing.*; 
public class ClockFrame extends JFrame{ 
	
	public static void main(String[] args) { 
		ClockFrame frame = new ClockFrame(); 
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); 
		frame.setVisible(true); 
	} 
	
	public ClockFrame() { 
		super("Pedestrian Animation");  // Or setTitle(É) 
		setSize(800, 800); 
		ClockPanel clock = new ClockPanel(); 
		Container contentPane= getContentPane();   
		contentPane.add(clock);   
	} 
}