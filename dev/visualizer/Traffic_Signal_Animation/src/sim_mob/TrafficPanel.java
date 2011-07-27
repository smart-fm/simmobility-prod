package sim_mob;

import java.awt.*; 

import javax.swing.*; 

import java.awt.event.*; 
import java.awt.image.BufferedImage;
import java.io.*;
import java.util.*;
import java.util.regex.Pattern;

import javax.swing.Timer;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.imageio.ImageIO;

public class TrafficPanel extends JPanel implements ActionListener, ChangeListener {
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
	private static final Pattern LEGACY_TIME_TICK_LINE_REGEX = Pattern.compile(
		  "\\(" 
	    + RG_INT + "," + RG_INT + ","       //Agent ID, Time Tick 
		+ RG_DOUBLE + "," + RG_DOUBLE + "," //Agent X, Y
		+ RG_INT                            //Signal; unused.
		+ "\\)"
	);
	
	//Which type of model are we reading?
	enum OutputTypes {
		STANDARD,
		LEGACY
	}
	OutputTypes mode; 
	
	//Saved data
	private ArrayList<TimeTick> ticks;
	
	// Add buttons and labels
	private JButton startButton, stopButton, stepForwardButton, stepBackButton, resetButton; 
	private JLabel frameNumLabel, cycleNumLabel, blanks, DSLabel, PhaseCounterLabel;
	private JSlider frameSlider;
	
	private int numAgents = 60;		// Set number of agents
	private int curFrameNum = 1;	// Set the default current frame number
	private int vehicleOvalSize = 7; // Set the vehicle size
	private int timerSpeed = 100; // Define frame speed:  default 10 frames/second
	
	
	// The (x, y) position for the road 
	private int scaledCrossingX1 = 460;
	private int scaledCrossingX2 = 730;	
	private int scaledCrossingY1 = 245;
	private int scaledCrossingY2 = 465;
	
	// Array list for vehicle image
	private ArrayList<BufferedImage> imageList = new ArrayList<BufferedImage>();
	
	//NOTE: We have an array of agents at all given time ticks, so there's no need to 
	//      cache the data here. 
	//X, Y coord of agents
	//private int[] agentXCoord = new int[numAgents];
	//private int[] agentYCoord = new int[numAgents];
	//direction
	//private double[] carDirection = new double[numAgents];
	
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
		JPanel bottomContainer = new JPanel();
		JPanel topPanel = new JPanel();
		
		startButton = new JButton("Start");
		stopButton = new JButton("Stop");
		stepForwardButton = new JButton("Step Forward");
		stepBackButton = new JButton("Step Backward");
		resetButton = new JButton("Reset");    
		frameNumLabel = new JLabel("Frame #: " + curFrameNum);	//frame number
		blanks =  new JLabel("                                                                                        " +
				"                                                        ");
		frameSlider = new JSlider(JSlider.HORIZONTAL);
		
		cycleNumLabel = new JLabel("Cycle Length: " + 0);
		DSLabel = new JLabel("  DS: " + 0);
		PhaseCounterLabel = new JLabel("  Phase Counter: " + 0);
				
		bottomPanel.add(startButton);
		bottomPanel.add(stopButton);
		bottomPanel.add(stepForwardButton);
		bottomPanel.add(stepBackButton);
		bottomPanel.add(resetButton);
		
		bottomContainer.setLayout(new BorderLayout());
		bottomContainer.add(frameSlider, BorderLayout.NORTH);
		bottomContainer.add(bottomPanel, BorderLayout.SOUTH);
		
		topPanel.add(frameNumLabel);
		topPanel.add(blanks);
		topPanel.add(cycleNumLabel);
		topPanel.add(DSLabel);
		topPanel.add(PhaseCounterLabel);
		
		setLayout(new BorderLayout()); 
		add(bottomContainer, BorderLayout.SOUTH);
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
		
		//Some properties change in legacy mode
		if (mode==OutputTypes.LEGACY) {
			scaledCrossingX1 = 360;
			scaledCrossingX2 = 440;
			scaledCrossingY1 = 304;
			scaledCrossingY2 = 336;
		}
		
		//Now, set the Slider's value
		frameSlider.setMinimum(1);
		frameSlider.setMaximum(ticks.size()-1);
		frameSlider.setMajorTickSpacing(ticks.size()/10);
		frameSlider.setMinorTickSpacing(ticks.size()/50);
		frameSlider.setValue(1);
		frameSlider.addChangeListener(this);
	}
	
	// Read the input from the log file
	public void readFile(String filename, ArrayList<String> arl) {
		//@SuppressWarnings("unused")
		//char[] boundaryBottomLeft, boundaryBottomRight, boundaryTopLeft, boundaryTopRight;
		
		boolean modeFound = false;
		try {
			FileReader fin = new FileReader(filename);
			BufferedReader b = new BufferedReader(fin);
			
			String currentLine = "";
			while((currentLine = b.readLine()) != null) {
				if (TIME_TICK_LINE_REGEX.matcher(currentLine).matches()) {
					if (modeFound && mode==OutputTypes.LEGACY) {
						throw new RuntimeException("Mixed modes: " + currentLine);
					}
					arl.add(currentLine.substring(1, currentLine.length()-1));
					mode = OutputTypes.STANDARD;
					modeFound = true;
				} else if (LEGACY_TIME_TICK_LINE_REGEX.matcher(currentLine).matches()) {
					if (modeFound && mode==OutputTypes.STANDARD) {
						throw new RuntimeException("Mixed modes: " + currentLine);
					}
					arl.add(currentLine.substring(1, currentLine.length()-1));
					mode = OutputTypes.LEGACY;
					modeFound = true;
				} else {
					System.out.println("Skipped line: " + currentLine);
				}
			}
			fin.close();
		}
		catch(FileNotFoundException ef) {
			throw new RuntimeException("File not found");
		}
		catch(IOException io) {
			throw new RuntimeException("IO Exception");
		}
		
		if (!modeFound) {
			throw new RuntimeException("Couldn't match any reasonable lines.");
		}
		System.out.println("Current mode is: " + (mode==OutputTypes.STANDARD ? "Standard" : "Legacy"));
		
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
	/*public double[] scaleCoord(int x, int y) {
		
		double[] dblArray = new double[2];
		double newX, newY;

		newX = (500 + (x-500)*3.2) +90;
		newY = (300 + (y-300)*2.6) +50;
		
		dblArray[0] = newX;
		dblArray[1] = newY;

		return dblArray;
	}
	
	public double[] scaleCoordLegacy(int x, int y) {
		double[] dblArray = new double[2];
		dblArray[0] = (double)(x);
		dblArray[1] = (double)(y);
		return dblArray;
	}*/
	
	
	//Set the agent's "scaled" coordinates
	private void scaleCoordinates(AgentTick ag, boolean isLegacy)
	{
		if (isLegacy) {
			ag.agentScaledX = (int)ag.agentX;
			ag.agentScaledY = (int)ag.agentY;
		} else {
			ag.agentScaledX = (int)(500 + (ag.agentX-500)*3.2) +90;
			ag.agentScaledY = (int)(300 + (ag.agentY-300)*2.6) +50;
		}
	}
	
	
	// Draw all the components on the panel
	public void paintComponent(Graphics g) {
		super.paintComponent(g); 
		
		if (mode==OutputTypes.STANDARD) {
			paintComponentStandard(g);
		} else {
			paintComponentLegacy(g);
		}
	}
	
	
	private void paintComponentLegacy(Graphics g) {
		// Draw road
		int roadSideWidth = 2;
		g.setColor(Color.BLACK);
		g.drawRect(0, scaledCrossingY1 - roadSideWidth, 1200, roadSideWidth);
		g.fillRect(0, scaledCrossingY1 - roadSideWidth, 1200, roadSideWidth);
		g.drawRect(0, scaledCrossingY2, 1200, roadSideWidth);
		g.fillRect(0, scaledCrossingY2, 1200, roadSideWidth);
		
		// bad area
		int badAreaStart = 200;
		int badAreaEnd = 350;
		int badAreaLane = 0;
		g.drawRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);
		g.fillRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);
		badAreaStart = 200;
		badAreaEnd = 350;
		badAreaLane = 2;
		g.drawRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);
		g.fillRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);
		badAreaStart = 750;
		badAreaEnd = 800;
		badAreaLane = 1;
		g.drawRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);
		g.fillRect(badAreaStart, scaledCrossingY1
				+ ((scaledCrossingY2 - scaledCrossingY1) * badAreaLane / 3),
				badAreaEnd - badAreaStart,
				(scaledCrossingY2 - scaledCrossingY1) / 3 + roadSideWidth);

		for (int i = 0; i < 10; i++) {
			g.drawRect(i * 100, 200, 2, 2);
			g.fillRect(i * 100, 200, 2, 2);
		}

		// Draw road divider
		int dividerWidth = 40;
		for (int i = 0; i < 20; i++) {
			g.drawRect(dividerWidth * 2 * i, scaledCrossingY1
					+ ((scaledCrossingY2 - scaledCrossingY1) * 2 / 3),
					dividerWidth, roadSideWidth / 2);
			g.setColor(Color.BLACK);
			g.fillRect(dividerWidth * 2 * i, scaledCrossingY1
					+ ((scaledCrossingY2 - scaledCrossingY1) * 2 / 3),
					dividerWidth, roadSideWidth / 2);
		}
		for (int i = 0; i < 20; i++) {
			g.drawRect(dividerWidth * 2 * i, scaledCrossingY1
					+ ((scaledCrossingY2 - scaledCrossingY1) / 3),
					dividerWidth, roadSideWidth / 2);
			g.setColor(Color.BLACK);
			g.fillRect(dividerWidth * 2 * i, scaledCrossingY1
					+ ((scaledCrossingY2 - scaledCrossingY1) / 3),
					dividerWidth, roadSideWidth / 2);
		}

		//Set positions
		//setAgentPosition();

		//Draw cars
		int carlength = 10;
		int carwidth  = 6;
		for (AgentTick ag : ticks.get(curFrameNum).agentTicks.values()) {
			g.setColor(Color.BLACK);
			g.drawRect(ag.agentScaledX - (carlength / 2), ag.agentScaledY - (carwidth / 2), carlength, carwidth);
			// g.drawString(Integer.toString(i),
			// ag.agentScaledX+pedestrianOvalSize,
			// ag.agentScaledY+pedestrianOvalSize);
			g.setColor(Color.PINK);
			g.fillRect(ag.agentScaledX - (carlength / 2), ag.agentScaledY - (carwidth / 2), carlength, carwidth);
		}
	}
	
	
	private void paintComponentStandard(Graphics g) {
		//Request a sample agent.
		//NOTE: Why does each agent have a copy of the cycle length, DS, and Phase Counter variables?
		Iterator<AgentTick> it = ticks.get(curFrameNum).agentTicks.values().iterator();
		AgentTick ag = it.hasNext() ? it.next() : null;
		
		// Labels on the panel
		cycleNumLabel.setText("Cycle Length: " + (ag!=null?ag.cycleLen:0));
		DSLabel.setText("  DS: " + (ag!=null?ag.ds:0) + "%");	
		PhaseCounterLabel.setText("  Phase Counter: " + (ag!=null?ag.phaseCount:0));
		
		// Draw Road
		drawRoad(g);

		// Determine vehicle position
		//setAgentPosition();
		
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
		
		
		//Retrive a sample agent.
		//NOTE: Why does each agent maintain a copy of the signal phase?
		Iterator<AgentTick> it = ticks.get(curFrameNum).agentTicks.values().iterator();
		AgentTick ag = it.hasNext() ? it.next() : null;
		int signalPhase = ag!=null ? ag.phaseSignal : 0;
		
		
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
		
		for(AgentTick ag : ticks.get(curFrameNum).agentTicks.values()) {
			
			if(ag.carDir > 0 && ag.carDir <=30)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(0), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 30 && ag.carDir <= 60 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(1), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 60 && ag.carDir < 90 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(2), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
		    else if(ag.carDir == 90 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(3), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);
			}
			
			
		    else if(ag.carDir > 90 && ag.carDir <=120)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(4), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 120 && ag.carDir <= 150 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(5), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 150 && ag.carDir < 180 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(6), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}			
			else if (ag.carDir == 180)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(7), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			
			
			
			else if(ag.carDir > 180 && ag.carDir <=210)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(8), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 210 && ag.carDir <= 240 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(9), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 240 && ag.carDir < 270 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(10), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir == 270)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(11), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			
			
			
			
			else if(ag.carDir > 270 && ag.carDir <=300)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(12), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 300 && ag.carDir <= 330 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(13), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			else if (ag.carDir > 330 && ag.carDir < 360 )
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(14), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

			}
			
			else if (ag.carDir == 360)
			{
//				System.out.println(ag.carDir);
				g.drawImage( imageList.get(15), ag.agentScaledX-(vehicleOvalSize/2), ag.agentScaledY-(vehicleOvalSize/2), null);

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
			if (mode==OutputTypes.STANDARD) {
				agent.ds = Double.parseDouble(items[5]);
				agent.cycleLen = Double.parseDouble(items[6]);
				agent.phaseCount = Integer.parseInt(items[7]);
				agent.carDir = Double.parseDouble(items[8]);
			}
			
			//Scale the agent's x/y positions
			scaleCoordinates(agent, mode==OutputTypes.LEGACY);
			
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
	
	
	
	/*private void setAgentPosition() {
		//Iterate through all agents in this tick
		TimeTick tick = ticks.get(curFrameNum);
		for (AgentTick agent : tick.agentTicks.values()) {		
			//Scale the agent position*/
			/*double[] intArray = null;
			if (mode==OutputTypes.LEGACY) {
				intArray = scaleCoordLegacy((int)agent.agentX, (int)agent.agentY);
			} else {
				intArray = scaleCoord((int)agent.agentX, (int)agent.agentY);
			}*/
									
			/*agentXCoord[agent.agentID] = (int)intArray[0];
			agentYCoord[agent.agentID] = (int)intArray[1];*/
			/*	
			signalPhase = agent.phaseSignal;
			cycleLength  = agent.cycleLen;
			DS = Math.floor(agent.ds*1000)/10;
			PhaseCounter = agent.phaseCount;
				
			//carDirection[agent.agentID] = agent.carDir;
				
			frameNumLabel.setText("Frame #: " + curFrameNum);	
	
		}
	}*/
	
	public void stateChanged(ChangeEvent e) {
		if (e.getSource().equals(frameSlider)) {
			//Paint
			curFrameNum = frameSlider.getValue();
			//setAgentPosition();
			repaint();
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
			frameSlider.setValue(curFrameNum); //This should really be done with a listener.
			//setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(stepForwardButton)) {
			if (curFrameNum<ticks.size()-1) {
				curFrameNum++;
				frameSlider.setValue(curFrameNum); //This should really be done with a listener.
				//setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(stepBackButton)) {
			if (curFrameNum>1) {
				curFrameNum--;
				frameSlider.setValue(curFrameNum); //This should really be done with a listener.
				//setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(resetButton)) {
			timer.stop();
			curFrameNum = 1;  //NOTE: The first tick is actually tick 0. ~Seth
			frameSlider.setValue(curFrameNum); //This should really be done with a listener.
			//setAgentPosition();
			repaint();
		}
		else {  // Reset button
		// Complete this code: update clock and labels 
			repaint();}
	}

	
	
}
