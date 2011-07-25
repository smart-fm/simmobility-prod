package sim_mob;

import java.awt.*; 

import javax.swing.*; 

import java.awt.event.*; 
import java.awt.image.BufferedImage;
import java.io.*;
import java.util.*;
import java.util.regex.Pattern;

import javax.swing.Timer;
import javax.imageio.ImageIO;

public class TrafficPanel extends JPanel implements ActionListener{
	private static final long serialVersionUID = 1L;
	
	//Regex matching actual time tick lines.
	private static final String RG_INT = "-?\\d+";
	private static final String RG_DOUBLE = "-?\\d+(?:\\.\\d+)?";
	private static final Pattern TIME_TICK_LINE_REGEX = Pattern.compile(
		  "\\(" 
	    + RG_INT + "," + RG_INT + ","       //Agent ID, Time Tick 
		+ RG_DOUBLE + "," + RG_DOUBLE + "," //Agent X, Y
		+ RG_INT + "," + RG_DOUBLE + ","    //Phase signal, DS 
		+ RG_DOUBLE + "," + RG_INT + ","    //Cycle length, phase counter 
		+ RG_DOUBLE                         //Car direction
		+ "\\)"
	);
	
	//Saved data
	private ArrayList<TimeTick> ticks;
	
	// Add buttons and labels
	private JButton startButton, stopButton, stepForwardButton, stepBackButton, resetButton; 
	private JLabel frameNumLabel, cycleNumLabel, blanks, DSLabel, PhaseCounterLabel;
	
	
	private int numAgents = 60;		// Set number of agents
	private int curFrameNum = 1;	// Set the default current frame number
	private int signalPhase = 0;	// Set the default traffic signal phase
	private double cycleLength = 0; // Set the default cycle length
	private double DS = 0;  		// Set the default DS
	private int PhaseCounter = 0; 	// Set the default phase counter
	private int vehicleOvalSize = 7; // Set the vehicle size
	private int timerSpeed = 100; // Define frame speed:  default 10 frames/second
	
	
	// The (x, y) position for the road 
	private int scaledCrossingX1 = 460;
	private int scaledCrossingX2 = 730;	
	private int scaledCrossingY1 = 245;
	private int scaledCrossingY2 = 465;
	
	// Array list for vehicle image
	private ArrayList<BufferedImage> imageList = new ArrayList<BufferedImage>();
	
	//X, Y coord of agents
	private int[] agentXCoord = new int[numAgents];
	private int[] agentYCoord = new int[numAgents];
	
	//direction
	private double[] carDirection = new double[numAgents];
	
	BufferedImage image; 	// Declare the image variable
	
	
	//Helper method
	public static BufferedImage LoadImgResource(String path) throws IOException {
		InputStream input = TrafficFrame.class.getClassLoader().getResourceAsStream(path);
		return ImageIO.read(input);
	}
	
	
	// Set the frame speed
	Timer timer = new Timer(timerSpeed, this);
	
	// Constructor
	public TrafficPanel(String inFileName){ 	
		JPanel bottomPanel = new JPanel(); 
		JPanel topPanel = new JPanel();
		
		startButton = new JButton("Start");
		stopButton = new JButton("Stop");
		stepForwardButton = new JButton("Step Forward");
		stepBackButton = new JButton("Step Backward");
		resetButton = new JButton("Reset");    
		frameNumLabel = new JLabel("Frame #: " + curFrameNum);	//frame number
		blanks =  new JLabel("                                                                                        " +
				"                                                        ");
		
		cycleNumLabel = new JLabel("Cycle Length: " + cycleLength);
		DSLabel = new JLabel("  DS: " + DS);
		PhaseCounterLabel = new JLabel("  Phase Counter: " + PhaseCounter);
				
		bottomPanel.add(startButton);
		bottomPanel.add(stopButton);
		bottomPanel.add(stepForwardButton);
		bottomPanel.add(stepBackButton);
		bottomPanel.add(resetButton);
		
		topPanel.add(frameNumLabel);
		topPanel.add(blanks);
		topPanel.add(cycleNumLabel);
		topPanel.add(DSLabel);
		topPanel.add(PhaseCounterLabel);
		
		setLayout(new BorderLayout()); 
		add(bottomPanel, BorderLayout.SOUTH);
		add(topPanel, BorderLayout.NORTH);
		//add(bottomPanel, BorderLayout.EAST);
		
		// Who will listen to the button events? Your code here 
		startButton.addActionListener(this);
		stopButton.addActionListener(this);
		stepForwardButton.addActionListener(this);
		stepBackButton.addActionListener(this);
		resetButton.addActionListener(this);
		
		// Read the data from log file
		ArrayList<String> arl = new ArrayList<String>();
		readFile(inFileName, arl);
		readTicks(arl);
	}
	
	// Read the input from the log file
	public void readFile(String filename, ArrayList<String> arl) {
		//@SuppressWarnings("unused")
		//char[] boundaryBottomLeft, boundaryBottomRight, boundaryTopLeft, boundaryTopRight;
		
		
		try {
			FileReader fin = new FileReader(filename);
			BufferedReader b = new BufferedReader(fin);
			
			String currentLine = "";
			while((currentLine = b.readLine()) != null) {
				if (TIME_TICK_LINE_REGEX.matcher(currentLine).matches()) {
					arl.add(currentLine.substring(1, currentLine.length()-1));
				} else {
					System.out.println("Skipped line: " + currentLine);
				}
			}
			fin.close();
		}
		catch(FileNotFoundException ef) {
			System.out.println("File not found");
		}
		catch(IOException io) {
			System.out.println("IO Exception");
		}
		
		// Image name
		String imageNames[] = new String[] {
			"20", "45", "70", "90", "110", "135", "160", "180", 
			"200", "225", "250", "270", "290", "315", "340", "360"          
		};
		
		for(String name : imageNames) {
			try {			
				BufferedImage car = LoadImgResource("res/car" + name + ".png");
				imageList.add(car);
			} catch (IOException ie) {
				System.out.println("Error:"+ie.getMessage()); 
			}
		}
		
	}
	
	// Re-adjust vehicles' position to accommodate x-axis scaling
	public double[] scaleCoord(int x, int y) {
		
		double[] dblArray = new double[2];
		double newX, newY;

		newX = (500 + (x-500)*3.2) +90;
		newY = (300 + (y-300)*2.6) +50;
		
		dblArray[0] = newX;
		dblArray[1] = newY;

		return dblArray;
	}
	
	// Draw all the components on the panel
	public void paintComponent(Graphics g) {
		
		super.paintComponent(g); 
		
		// Labels on the panel
		cycleNumLabel.setText("Cycle Length: " + cycleLength);
		DSLabel.setText("  DS: " + DS + "%");	
		PhaseCounterLabel.setText("  Phase Counter: " + PhaseCounter);
		
		// Draw Road
		drawRoad(g);

		// Determine vehicle position
		setAgentPosition();
		
		// Display vehicles
		displayAgents(g);
		
		// Draw and display the traffic light phase
		displaySingalPhase(g);
		
	}
	
	
	// Draw road segment
	public void drawRoad(Graphics g){
		//Draw road
		
		int roadSideWidth = 5;
		g.setColor(Color.BLACK);
		g.drawRect(0, scaledCrossingY1-roadSideWidth, scaledCrossingX1, roadSideWidth);
		g.fillRect(0, scaledCrossingY1-roadSideWidth, scaledCrossingX1, roadSideWidth);
		g.drawRect(scaledCrossingX2, scaledCrossingY1-roadSideWidth, 1200, roadSideWidth);
		g.fillRect(scaledCrossingX2, scaledCrossingY1-roadSideWidth, 1200, roadSideWidth);
		
		
		g.drawRect(0, scaledCrossingY2, scaledCrossingX1, roadSideWidth);
		g.fillRect(0, scaledCrossingY2,	scaledCrossingX1, roadSideWidth);
		g.drawRect(scaledCrossingX2, scaledCrossingY2, 1200, roadSideWidth);
		g.fillRect(scaledCrossingX2, scaledCrossingY2, 1200, roadSideWidth);


		int startPointY2 = 0;
		int lengthRoadY = 245;

		
		g.drawRect(scaledCrossingX1, startPointY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX1, startPointY2, roadSideWidth, lengthRoadY);
		g.drawRect(scaledCrossingX2, startPointY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX2, startPointY2, roadSideWidth, lengthRoadY);
		
		g.drawRect(scaledCrossingX1, scaledCrossingY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX1, scaledCrossingY2, roadSideWidth, lengthRoadY);
		g.drawRect(scaledCrossingX2, scaledCrossingY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX2, scaledCrossingY2, roadSideWidth, lengthRoadY);
				
		//Draw road divider
		int dividerWidth = 40;
		for(int i=0; i<6; i++) {
	
				g.drawRect(dividerWidth*2*i, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);
				g.setColor(Color.BLACK);
				g.fillRect(dividerWidth*2*i, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);	
		
				g.drawRect(dividerWidth*2*i + 730, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);
				g.fillRect(dividerWidth*2*i + 730, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);	
		
		}

		int offset = 40;
		for(int i=0; i<3; i++) {
			g.drawRect(590, dividerWidth*2*i+offset, roadSideWidth, dividerWidth);
			g.fillRect(590, dividerWidth*2*i+offset, roadSideWidth, dividerWidth);			
		}
		
		for(int i=0; i<5; i++) {
			g.drawRect(590, dividerWidth*2*i + scaledCrossingY2, roadSideWidth, dividerWidth);
			g.fillRect(590, dividerWidth*2*i + scaledCrossingY2, roadSideWidth, dividerWidth);
		}
		
		
		
		
	}
	
	public void displaySingalPhase(Graphics g){
		
		// Draw traffic light
		int size = 15;  //light size
		
        // light zero
		int light_01_X = 460;
		int light_01_Y = 250;
     
		int light_02_X = light_01_X;
		int light_02_Y = light_01_Y + size;

		int light_03_X = light_01_X;
		int light_03_Y = light_02_Y + size;
     
		// light one
		int light_11_X = 680;
		int light_11_Y = 245;
     
		int light_12_X = light_11_X + size;
		int light_12_Y = light_11_Y;

		int light_13_X = light_12_X + size;	
		int light_13_Y = light_11_Y;

		// light two 
		int light_21_X = 720;	
		int light_21_Y = 415;
     
		int light_22_X = light_21_X;
		int light_22_Y = light_21_Y + size;

		int light_23_X = light_21_X;
		int light_23_Y = light_22_Y + size;

		// light three
		int light_31_X = 470;
		int light_31_Y = 455;
     
		int light_32_X = light_31_X + size;
		int light_32_Y = light_31_Y;

		int light_33_X = light_32_X + size;
		int light_33_Y = light_31_Y;

		
		g.drawOval(light_01_X,light_01_Y,size, size);
		g.drawOval(light_02_X,light_02_Y,size, size);
		g.drawOval(light_03_X,light_03_Y,size, size);

		g.drawOval(light_11_X,light_11_Y,size, size);
		g.drawOval(light_12_X,light_12_Y,size, size);
		g.drawOval(light_13_X,light_13_Y,size, size);		
		

		g.drawOval(light_21_X,light_21_Y,size, size);
		g.drawOval(light_22_X,light_22_Y,size, size);
		g.drawOval(light_23_X,light_23_Y,size, size);		
		
		g.drawOval(light_31_X,light_31_Y,size, size);
		g.drawOval(light_32_X,light_32_Y,size, size);
		g.drawOval(light_33_X,light_33_Y,size, size);		
		
		switch (signalPhase){
		case 0:
			g.setColor(Color.GREEN);
			
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_03_X,light_03_Y,size, size);
			g.fillOval(light_21_X,light_21_Y,size, size);
			
			g.setColor(Color.RED);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);

			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);		
			
			break;
		
		case 1:
			g.setColor(Color.RED);
			
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);

			g.setColor(Color.GREEN);
			g.fillOval(light_03_X,light_03_Y,size, size);
			g.fillOval(light_21_X,light_21_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);

			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);		
			
			
			break;

		case 2:

			g.setColor(Color.GREEN);
			
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);
			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);
						
			g.setColor(Color.RED);
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_03_X,light_03_Y,size, size);

			g.fillOval(light_21_X,light_21_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);		
			
			
			break;
		
		case 3:
			g.setColor(Color.RED);
			
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);
			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);

			g.setColor(Color.GREEN);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_03_X,light_03_Y,size, size);

			g.fillOval(light_21_X,light_21_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);		
			
			break;
			
		case 10:
			
			g.setColor(Color.YELLOW);
			
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_03_X,light_03_Y,size, size);
			g.fillOval(light_21_X,light_21_Y,size, size);
			
			g.setColor(Color.RED);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);

			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);	
			
			break;
		case 11:
			g.setColor(Color.RED);
			
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);

			g.setColor(Color.YELLOW);
			g.fillOval(light_03_X,light_03_Y,size, size);
			g.fillOval(light_21_X,light_21_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);

			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);		
			
			
			break;
			
		case 12:
			g.setColor(Color.RED);
			
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);
			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);

			g.setColor(Color.YELLOW);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_03_X,light_03_Y,size, size);

			g.fillOval(light_21_X,light_21_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);		
			
			break;
			
		case 13:
			
			g.setColor(Color.RED);
			
			g.fillOval(light_12_X,light_12_Y,size, size);
			g.fillOval(light_13_X,light_13_Y,size, size);
			g.fillOval(light_31_X,light_31_Y,size, size);
			g.fillOval(light_32_X,light_32_Y,size, size);

			g.setColor(Color.YELLOW);
			g.fillOval(light_11_X,light_11_Y,size, size);
			g.fillOval(light_33_X,light_33_Y,size, size);

			g.setColor(Color.RED);
			g.fillOval(light_01_X,light_01_Y,size, size);
			g.fillOval(light_02_X,light_02_Y,size, size);
			g.fillOval(light_03_X,light_03_Y,size, size);

			g.fillOval(light_21_X,light_21_Y,size, size);
			g.fillOval(light_22_X,light_22_Y,size, size);
			g.fillOval(light_23_X,light_23_Y,size, size);		
			
			break;
			
		default:
			
			System.out.println("None of signals are correct");
		}
		
		
	} 
	
	public void displayAgents(Graphics g){
		
		for(int i=0; i<numAgents; i++) {
			
			if(carDirection[i] > 0 && carDirection[i] <=30)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(0), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 30 && carDirection[i] <= 60 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(1), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 60 && carDirection[i] < 90 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(2), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
		    else if(carDirection[i] == 90 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(3), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);
			}
			
			
		    else if(carDirection[i] > 90 && carDirection[i] <=120)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(4), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 120 && carDirection[i] <= 150 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(5), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 150 && carDirection[i] < 180 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(6), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}			
			else if (carDirection[i] == 180)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(7), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			
			
			
			else if(carDirection[i] > 180 && carDirection[i] <=210)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(8), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 210 && carDirection[i] <= 240 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(9), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 240 && carDirection[i] < 270 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(10), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] == 270)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(11), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			
			
			
			
			else if(carDirection[i] > 270 && carDirection[i] <=300)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(12), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 300 && carDirection[i] <= 330 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(13), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else if (carDirection[i] > 330 && carDirection[i] < 360 )
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(14), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			
			else if (carDirection[i] == 360)
			{
//				System.out.println(carDirection[i]);
				g.drawImage( imageList.get(15), agentXCoord[i]-(vehicleOvalSize/2), agentYCoord[i]-(vehicleOvalSize/2), null);

			}
			else{}
		}
	}
	
	
	//Parse the agent data array. 
	private void readTicks(ArrayList<String> lines)  {
		ticks = new ArrayList<TimeTick>();
		for (String line : lines) {
			String[] items = line.split(",");
			
			//Create an agent representing this set of data.
			int frameNum = Integer.parseInt(items[1]);
			AgentTick agent = new AgentTick();
			agent.agentID = Integer.parseInt(items[0]);
			agent.agentX = Double.parseDouble(items[2]);
			agent.agentY = Double.parseDouble(items[3]);
			agent.phaseSignal = Integer.parseInt(items[4]);
			agent.ds = Double.parseDouble(items[5]);
			agent.cycleLen = Double.parseDouble(items[6]);
			agent.phaseCount = Integer.parseInt(items[7]);
			agent.carDir = Double.parseDouble(items[8]);
			
			//Ensure we have a spot to put the agent.
			while (ticks.size()<=frameNum) {
				ticks.add(new TimeTick());
			}
			TimeTick currTick = ticks.get(frameNum);
			if (currTick.agentTicks==null) {
				currTick.agentTicks = new Hashtable<Integer, AgentTick>();
			}
			
			//Put this agent in the right place
			currTick.agentTicks.put(agent.agentID, agent);
		}
	}
	
	
	public void setAgentPosition(){
		//Stop
		//if(curFrameNum >= ticks.size()) {
		//	timer.stop();
		//	return;
		//}
		
		//Iterate through all agents in this tick
		TimeTick tick = ticks.get(curFrameNum);
		for (AgentTick agent : tick.agentTicks.values()) {		
			//Scale the agent position
			double[] intArray = new double[2];
			intArray = scaleCoord((int)agent.agentX, (int)agent.agentY);
									
			agentXCoord[agent.agentID] = (int)intArray[0];
			agentYCoord[agent.agentID] = (int)intArray[1];
				
			signalPhase = agent.phaseSignal;
			cycleLength  = agent.cycleLen;
			DS = Math.floor(agent.ds*1000)/10;
			PhaseCounter = agent.phaseCount;
				
			carDirection[agent.agentID] = agent.carDir;
				
			frameNumLabel.setText("Frame #: " + curFrameNum);	
	
		}
	} 
	
	public void actionPerformed(ActionEvent e) { 
		if(e.getSource().equals(startButton)) {
			timer.start();
		}
		else if(e.getSource().equals(stopButton)) {
			timer.stop();
		}
		else if(e.getSource().equals(timer)) {
			//Done?
			if (curFrameNum==ticks.size()-1) {
				timer.stop();
				return;
			}
			
			//Increment, paint
			curFrameNum++;
			setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(stepForwardButton)) {
			if (curFrameNum<ticks.size()-1) {
				curFrameNum++;
				setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(stepBackButton)) {
			if (curFrameNum>1) {
				curFrameNum--;
				setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(resetButton)) {
			timer.stop();
			curFrameNum = 1;  //NOTE: The first tick is actually tick 0. ~Seth
			setAgentPosition();
			repaint();
		}
		else {  // Reset button
		// Complete this code: update clock and labels 
			repaint();}
	}

	
	
}
