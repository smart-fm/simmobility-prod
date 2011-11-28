package sim_mob;

import java.awt.*; 

import javax.swing.*; 

import java.awt.event.*; 
import java.awt.image.BufferedImage;
import java.io.*;
import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import javax.swing.Timer;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.imageio.ImageIO;

public class TrafficPanel extends JPanel implements ActionListener, ChangeListener {
	private static final long serialVersionUID = 1L;
	private static final int DRIVER = 1;
	private static final int SIGNAL = 2;
	private static final int PEDESTRIAN = 3;
	
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
	
	
	private static final String rhs = "\\{([^}]*)\\}"; //NOTE: Contains a capture group
	private static final String sep = ", *";
	private static final String strn = "\"([^\"]+)\"";
	private static final String num = "([0-9]+)";
	//private static final String numH = "((?:0x)?[0-9a-fA-F]+)";
	public static final Pattern LOG_LHS_REGEX = Pattern.compile("\\(" + strn + sep + num + sep + num + sep  + rhs + "\\)");
	public static final Pattern LOG_RHS_REGEX = Pattern.compile(strn + ":" + strn + ",?");
	
	//Saved data
	private ArrayList<TimeTick> ticks;
	
	// Add buttons and labels
	private JButton startButton, stopButton, stepForwardButton, stepBackButton, resetButton; 
	private JLabel frameNumLabel;
	private JSlider frameSlider;
	
	
	private int numAgents = 100;		// Set number of agents
	private int curFrameNum = 1;	// Set the default current frame number
	public Hashtable<String,ArrayList<Integer>> phaseValue = new Hashtable<String,ArrayList<Integer>>();
	private int signalPhase = 0;	// Set the default traffic signal phase
	
	private int vehicleOvalSize = 7; // Set the vehicle size
	private int pedestrianOvalSize = 5; // Set the pedestrian size
	
	private int timerSpeed = 100; // Define frame speed:  default 10 frames/second
	
	//frame length
	private static final int FRAMEHEIGHT = 800;
	private static final int FRAMELENGTH = 1200;
	
	// frame center
	private static final int FRAME_CENTER_X = 500;
	private static final int FRAME_CENTER_Y = 300;
	
	// lane width
	private static final int LANE_WIDTH = 20;
	private static final int NUMBER_OF_LANES = 3;

	
	// The (x, y) position for the road 
	private int scaledCrossingX1 = FRAME_CENTER_X - LANE_WIDTH * NUMBER_OF_LANES;
	private int scaledCrossingX2 = FRAME_CENTER_X + LANE_WIDTH * NUMBER_OF_LANES;	
	private int scaledCrossingY1 = FRAME_CENTER_Y - LANE_WIDTH * NUMBER_OF_LANES;
	private int scaledCrossingY2 = FRAME_CENTER_Y + LANE_WIDTH * NUMBER_OF_LANES;
	private int roadSideWidth = 3;
	private int pedestrianCrossingWidth = 20;
	private int pedestrianRoadWidth = 2;
	private int lengthRoadY = scaledCrossingY1;
	
	// Array list for vehicle image
	private ArrayList<BufferedImage> imageList = new ArrayList<BufferedImage>();
	private ArrayList<BufferedImage> pedestrianImageList = new ArrayList<BufferedImage>();
	
	
	//X, Y coord of car agents
	private int[] agentXCoord = new int[numAgents];
	private int[] agentYCoord = new int[numAgents];
	
	//X,Y coord of pedestrian agents
	private int[] pedestrianXCoord = new int[numAgents];
	private int[] pedestrianYCoord = new int[numAgents];
	
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
	private Graphics Graphics;
	
	
	public JPanel bottomPanel;
	public JPanel bottomContainer;
	public JPanel topPanel;
	// Constructor
	public TrafficPanel(String inFileName) throws IOException{ 	
		bottomPanel = new JPanel(); 
		bottomContainer = new JPanel();
		topPanel = new JPanel();
		
		startButton = new JButton("Start");
		stopButton = new JButton("Stop");
		stepForwardButton = new JButton("Step Forward");
		stepBackButton = new JButton("Step Backward");
		resetButton = new JButton("Reset");    
		frameNumLabel = new JLabel("Frame #: " + curFrameNum);	//frame number

		frameSlider = new JSlider(JSlider.HORIZONTAL);
		

		bottomPanel.add(startButton);
		bottomPanel.add(stopButton);
		bottomPanel.add(stepForwardButton);
		bottomPanel.add(stepBackButton);
		bottomPanel.add(resetButton);
		
		bottomContainer.setLayout(new BorderLayout());
		bottomContainer.add(frameSlider, BorderLayout.NORTH);
		bottomContainer.add(bottomPanel, BorderLayout.SOUTH);
		
		topPanel.add(frameNumLabel);
		
		
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
		
		
		try {
			FileReader fin = new FileReader(filename);
		
			BufferedReader b = new BufferedReader(fin);
			
			
			String currentLine = "";
			while((currentLine = b.readLine()) != null) {
				
				if(currentLine.isEmpty() || !currentLine.startsWith("(") || !currentLine.endsWith(")"))
				{
					
					System.out.println("Skip Line - : " + currentLine);
					continue;
				}
				arl.add(currentLine);
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
		
		//read pedestrian image
		try {
			BufferedImage pedestrian = LoadImgResource("res/p3.png");
			pedestrianImageList.add(pedestrian);
		} catch (IOException ePedes) {
			System.out.println("Error:"+ ePedes.getMessage());
		}
		
	}
	
	// Re-adjust vehicles' position to accommodate x-axis scaling
	public double[] scaleCoord(int x, int y) {
		
		double[] dblArray = new double[2];
		double newX, newY;

		newX = (500 + (x-500)*2);
		newY = (300 + (y-300)*2);
		
		dblArray[0] = newX;
		dblArray[1] = newY;

		return dblArray;
	}
	

	// Draw all the components on the panel
	public void paintComponent(Graphics g) {
		
		super.paintComponent(g); 
		
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
		//Draw pedestrian crossing
		
		//Pedestrian Crossing 1
		g.setColor(Color.GRAY);
		g.drawRect(scaledCrossingX1, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.fillRect(scaledCrossingX1, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.drawRect(scaledCrossingX1 - pedestrianCrossingWidth, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.fillRect(scaledCrossingX1 - pedestrianCrossingWidth, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		
		//Pedestrian Crossing 2		
		g.drawRect(scaledCrossingX1, scaledCrossingY1 - pedestrianRoadWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		g.fillRect(scaledCrossingX1, scaledCrossingY1 - pedestrianRoadWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);	
		g.drawRect(scaledCrossingX1, scaledCrossingY1 - pedestrianCrossingWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		g.fillRect(scaledCrossingX1, scaledCrossingY1 - pedestrianCrossingWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		
		//Pedestrian Crossing 3
		g.setColor(Color.GRAY);
		g.drawRect(scaledCrossingX2, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.fillRect(scaledCrossingX2, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.drawRect(scaledCrossingX2 + pedestrianCrossingWidth, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		g.fillRect(scaledCrossingX2 + pedestrianCrossingWidth, scaledCrossingY1, pedestrianRoadWidth, scaledCrossingY2-scaledCrossingY1);
		
		//Pedestrian Crossing 4		
		g.drawRect(scaledCrossingX1, scaledCrossingY2 + pedestrianRoadWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		g.fillRect(scaledCrossingX1, scaledCrossingY2 + pedestrianRoadWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);	
		g.drawRect(scaledCrossingX1, scaledCrossingY2 + pedestrianCrossingWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		g.fillRect(scaledCrossingX1, scaledCrossingY2 + pedestrianCrossingWidth, scaledCrossingX2 - scaledCrossingX1, pedestrianRoadWidth);
		
		
		
		//Draw zebra crossing
		
		int zebraGaps = 10;
		
		g.setColor(Color.gray);
		for(int i = 1; i< (scaledCrossingY2 - scaledCrossingY1)/10 ;i++)
		{
			//zebra crossing 1
			g.drawLine(scaledCrossingX1 - pedestrianCrossingWidth, scaledCrossingY1 + zebraGaps*(i - 1), scaledCrossingX1, scaledCrossingY1 + zebraGaps*(i));
			

			//zebra crossing 3
			g.drawLine(scaledCrossingX2, scaledCrossingY1 + zebraGaps*(i - 1), scaledCrossingX2 + pedestrianCrossingWidth, scaledCrossingY1 + zebraGaps*(i));

		}
		
		for(int i = 0;i< (scaledCrossingX2 - scaledCrossingX1)/10;i++)
		{
			//zebra crossing 2
			g.drawLine(scaledCrossingX1 + zebraGaps*(i), scaledCrossingY1 - pedestrianCrossingWidth , scaledCrossingX1 + zebraGaps*(i + 1), scaledCrossingY1);
			
			//zebra crossing 4
			g.drawLine(scaledCrossingX1 + zebraGaps*(i), scaledCrossingY2 + pedestrianRoadWidth, scaledCrossingX1 + zebraGaps*(i + 1), scaledCrossingY2 + pedestrianCrossingWidth);
		}
		
		//Draw road
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
		
		g.drawRect(scaledCrossingX1, startPointY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX1, startPointY2, roadSideWidth, lengthRoadY);
		g.drawRect(scaledCrossingX2, startPointY2, roadSideWidth, lengthRoadY);
		g.fillRect(scaledCrossingX2, startPointY2, roadSideWidth, lengthRoadY);
		
		g.drawRect(scaledCrossingX1, scaledCrossingY2, roadSideWidth, FRAMEHEIGHT - scaledCrossingY2 );
		g.fillRect(scaledCrossingX1, scaledCrossingY2, roadSideWidth, FRAMEHEIGHT - scaledCrossingY2);
		g.drawRect(scaledCrossingX2, scaledCrossingY2, roadSideWidth, FRAMEHEIGHT - scaledCrossingY2);
		g.fillRect(scaledCrossingX2, scaledCrossingY2, roadSideWidth, FRAMEHEIGHT - scaledCrossingY2);
		
		

		
		//Draw road divider
		int dividerWidth = 40;
		
		// Horizontal road divider
		int dividerStartXPos = 0;
		int dividerCounterX = 0;
		
		while(dividerStartXPos < FRAMELENGTH)
		{
			if( (dividerStartXPos + dividerWidth) > (scaledCrossingX1 - pedestrianCrossingWidth) && (dividerStartXPos + dividerWidth)<= (scaledCrossingX2+pedestrianCrossingWidth) )
			{
			    dividerStartXPos = dividerCounterX*dividerWidth*2;
				dividerCounterX++;
				continue;
			}			
			g.setColor(Color.BLACK);
			g.drawRect(dividerStartXPos, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);
			g.fillRect(dividerStartXPos, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);

			dividerStartXPos = dividerCounterX*dividerWidth*2;
			dividerCounterX++;

		}
		
		int dividerStartYPos = 0;
		int dividerCounterY = 0;
		
		while(dividerStartYPos < FRAMEHEIGHT)
		{
			if( (dividerStartYPos + dividerWidth) > (scaledCrossingY1 - pedestrianCrossingWidth) && (dividerStartYPos + dividerWidth)<= (scaledCrossingY2+pedestrianCrossingWidth) )
			{
			    dividerStartYPos = dividerCounterY*dividerWidth*2;
				dividerCounterY++;
				continue;
			}			
			
			g.setColor(Color.BLACK);
			g.drawRect(scaledCrossingX1+((scaledCrossingX2-scaledCrossingX1)/2), dividerStartYPos, roadSideWidth, dividerWidth);
			g.fillRect(scaledCrossingX1+((scaledCrossingX2-scaledCrossingX1)/2), dividerStartYPos, roadSideWidth, dividerWidth);

			dividerStartYPos = dividerCounterY*dividerWidth*2;
			dividerCounterY++;

		}

		
	}
	
	public void displaySingalPhase(Graphics g){
		
		
		// Draw traffic light
		int size = 10;  //light size
		int pSize = 10; //pedestrian light size
		int offset = 5; //pedestrian offset
		
        // light zero
		int light_01_X = FRAME_CENTER_X - LANE_WIDTH * NUMBER_OF_LANES + roadSideWidth;
		int light_01_Y = FRAME_CENTER_Y - LANE_WIDTH * NUMBER_OF_LANES;
     
		int light_02_X = light_01_X;
		int light_02_Y = light_01_Y + size;

		int light_03_X = light_01_X;
		int light_03_Y = light_02_Y + size;
     
		// light one
		int light_11_X = FRAME_CENTER_X + LANE_WIDTH * NUMBER_OF_LANES - size*3;
		int light_11_Y = FRAME_CENTER_Y - LANE_WIDTH * NUMBER_OF_LANES;
     
		int light_12_X = light_11_X + size;
		int light_12_Y = light_11_Y;

		int light_13_X = light_12_X + size;	
		int light_13_Y = light_11_Y;

		// light two 
		int light_21_X = FRAME_CENTER_X + LANE_WIDTH * NUMBER_OF_LANES - size;	
		int light_21_Y = FRAME_CENTER_Y + LANE_WIDTH * NUMBER_OF_LANES - size*3;
     
		int light_22_X = light_21_X;
		int light_22_Y = light_21_Y + size;

		int light_23_X = light_21_X;
		int light_23_Y = light_22_Y + size;

		// light three
		int light_31_X = FRAME_CENTER_X - LANE_WIDTH * NUMBER_OF_LANES + roadSideWidth;
		int light_31_Y = FRAME_CENTER_Y + LANE_WIDTH * NUMBER_OF_LANES - size;
     
		int light_32_X = light_31_X + size;
		int light_32_Y = light_31_Y;

		int light_33_X = light_32_X + size;
		int light_33_Y = light_31_Y;
		
		// pedestrian light zero
		int pLight_0_X = scaledCrossingX1 - pSize - offset;
		int pLight_0_Y = scaledCrossingY1 - pSize - offset;
		// pedestrian light one
		int pLight_1_X = scaledCrossingX2 + pSize;
		int pLight_1_Y = scaledCrossingY1 - pSize - offset;
		// pedestrian light two
		int pLight_2_X = scaledCrossingX2 + pSize;
		int pLight_2_Y = scaledCrossingY2 + pSize - offset;
		// pedestrian light three
		int pLight_3_X = scaledCrossingX1 - pSize - offset;
		int pLight_3_Y = scaledCrossingY2 + pSize - offset;
		
		// vehicle light
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
		
		// pedestrian light
		g.drawOval(pLight_0_X,pLight_0_Y,pSize, pSize);
		g.drawOval(pLight_1_X,pLight_1_Y,pSize, pSize);
		g.drawOval(pLight_2_X,pLight_2_Y,pSize, pSize);
		g.drawOval(pLight_3_X,pLight_3_Y,pSize, pSize);
		
		
		Enumeration<String> keys = phaseValue.keys();
		
		
		while(keys.hasMoreElements()){
			
			Object key = keys.nextElement();
    		ArrayList<Integer> value = phaseValue.get(key);
    		// vehicle light 0
    		if(String.valueOf(key).equals("va")){
    			int lightColor1 = value.get(0);
    			int lightColor2 = value.get(1);
    			int lightColor3 = value.get(2);
    			// light 1
    			setLightColor(g,lightColor1,light_01_X,light_01_Y,size);
    			// light 2
    			setLightColor(g,lightColor2,light_02_X,light_02_Y,size);
    			// light 3
    			setLightColor(g,lightColor3,light_03_X,light_03_Y,size);
    			
    		}
    		// vehicle light 1
    		else if(String.valueOf(key).equals("vb")){
    			int lightColor1 = value.get(0);
    			int lightColor2 = value.get(1);
    			int lightColor3 = value.get(2);
    			// light 1
    			setLightColor(g,lightColor1,light_11_X,light_11_Y,size);
    			// light 2
    			setLightColor(g,lightColor2,light_12_X,light_12_Y,size);
    			// light 3
    			setLightColor(g,lightColor3,light_13_X,light_13_Y,size);
    			
    		}
    		// vehicle light 2
    		else if(String.valueOf(key).equals("vc")){
    			int lightColor1 = value.get(0);
    			int lightColor2 = value.get(1);
    			int lightColor3 = value.get(2);
    			// light 1
    			setLightColor(g,lightColor1,light_21_X,light_21_Y,size);
    			// light 2
    			setLightColor(g,lightColor2,light_22_X,light_22_Y,size);
    			// light 3
    			setLightColor(g,lightColor3,light_23_X,light_23_Y,size);
    			
    		}
    		// vehicle light 3
    		else if(String.valueOf(key).equals("vd")){
    			int lightColor1 = value.get(0);
    			int lightColor2 = value.get(1);
    			int lightColor3 = value.get(2);
    			// light 1
    			setLightColor(g,lightColor1,light_31_X,light_31_Y,size);
    			// light 2
    			setLightColor(g,lightColor2,light_32_X,light_32_Y,size);
    			// light 3
    			setLightColor(g,lightColor3,light_33_X,light_33_Y,size);
    		}
    		// pedestrian light 0
    		else if(String.valueOf(key).equals("pa")){
    			int lightColor1 = value.get(0);
    			setLightColor(g,lightColor1,pLight_0_X,pLight_0_Y,size);
    		}
    		// pedestrian light 1
    		else if(String.valueOf(key).equals("pb")){
    			int lightColor1 = value.get(0);
    			setLightColor(g,lightColor1,pLight_1_X,pLight_1_Y,size);
    		}
    		// pedestrian light 2
    		else if(String.valueOf(key).equals("pc")){
    			int lightColor1 = value.get(0);
    			setLightColor(g,lightColor1,pLight_2_X,pLight_2_Y,size);
    		}
    		// pedestrian light 3
    		else if(String.valueOf(key).equals("pd")){
    			int lightColor1 = value.get(0);
    			setLightColor(g,lightColor1,pLight_3_X,pLight_3_Y,size);
    		}	
			
		}
		
		
	} 
	
	public void setLightColor(Graphics g, int lightColor,int lightXPos, int lightYPos, int lightSize){
		
		switch (lightColor){
		// red light
		case 1:
			g.setColor(Color.RED);
			g.fillOval(lightXPos,lightYPos,lightSize, lightSize);
			break;
		// yellow light
		case 2:
			g.setColor(Color.YELLOW);
			g.fillOval(lightXPos,lightYPos,lightSize, lightSize);
			break;
		// green light
		case 3:
			g.setColor(Color.GREEN);
			g.fillOval(lightXPos,lightYPos,lightSize, lightSize);
			break;	
		//default
		default:
			System.out.println("No color FOR: LightXPosition "+lightYPos + " , "+ "LightyPosition "+lightYPos);
		
		}
		
		
	}
	
	public void displayAgents(Graphics g){
		
		for(int i=0; i<numAgents; i++) {
			
			// car
			
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
		
			
			// draw pedestrian image
			g.drawImage(pedestrianImageList.get(0),pedestrianXCoord[i]-(pedestrianOvalSize/2), pedestrianYCoord[i]-(pedestrianOvalSize/2),null);
			
			//g.setColor(Color.BLUE);
			//g.drawOval(pedestrianXCoord[i]-(pedestrianOvalSize/2), pedestrianYCoord[i]-(pedestrianOvalSize/2), pedestrianOvalSize, pedestrianOvalSize);		
			//g.fillOval(pedestrianXCoord[i]-(pedestrianOvalSize/2), pedestrianYCoord[i]-(pedestrianOvalSize/2), pedestrianOvalSize, pedestrianOvalSize);		
		
		}
	}
	
	
	//Parse the agent data array. 
	private void readTicks(ArrayList<String> lines) throws IOException  {
		ticks = new ArrayList<TimeTick>();
		
		for (String line : lines) {
		 
			line = line.trim();
			//System.out.println(line);
			
			if(line.isEmpty() || !line.startsWith("(") || !line.endsWith(")"))
			{
				
				System.out.println("Skip Line: " + line);
				continue;
			}
			
			Matcher m = LOG_LHS_REGEX.matcher(line);
			
		    if (!m.matches()) {
			   System.out.println("m not mactch!");
			   break;
			}
			 
		    if (m.groupCount()!=4) {
				   System.out.println("there is no group 4");
				   break;
		    }
		    
		    String type = m.group(1);
		    int frameNum = Integer.parseInt(m.group(2));
		    String rhs = m.group(4);
		    
		    AgentTick agent = new AgentTick();
		    
		    // set agent ID
		    agent.agentID = Integer.parseInt(m.group(3));
		    //System.out.println("type       " + type);
		    
		    if(type.equals("Signal")){    	
		    	
		    	Hashtable<String, String> signalPhase = ParseLogRHS(rhs, new String[]{"va", "vb", "vc", "vd", "pa", "pb", "pc", "pd"});
		    	//System.out.println("va "+ signalPhase.get("va"));
		    	
		    	agent.agentType = SIGNAL;
		    	
		    	Enumeration<String> keys = signalPhase.keys();
		    	
		    	while(keys.hasMoreElements()){
		    		Object key = keys.nextElement();
		    		String value = signalPhase.get(key);
		    		ArrayList<Integer> itemsInt = new ArrayList<Integer>();
		    		
		    		String[] items = value.split(",");
		    		for(int i=0;i<items.length;i++)
		    		{
		    			itemsInt.add(Integer.parseInt(items[i]));
		    			
		    		}
		    		
		    		agent.phaseValue.put(String.valueOf(key), itemsInt);
		    				    		
		    	}
		    }
		    else if(type.equals("Driver"))
		    {
		    	Hashtable<String, String> driverDetails = ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle"});
		    	agent.agentType = DRIVER;
		    	agent.agentX = Double.parseDouble(driverDetails.get("xPos"));
		    	agent.agentY = Double.parseDouble(driverDetails.get("yPos"));
		    	agent.carDir = Double.parseDouble(driverDetails.get("angle"));
		    	
		    }
		    else if(type.equals("pedestrian"))
		    {
		    	Hashtable<String, String> pedestrianDetails = ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
		    	agent.agentType = PEDESTRIAN;
		    	agent.agentX = Double.parseDouble(pedestrianDetails.get("xPos"));
		    	agent.agentY = Double.parseDouble(pedestrianDetails.get("yPos"));
		    	
		    }
		    
			
	
			
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
	
	public Hashtable<String, String> ParseLogRHS(String rhs, String[] ensure) throws IOException {
		//Json-esque matching
	
		Hashtable<String, String> properties = new Hashtable<String, String>();
		
		Matcher m = LOG_RHS_REGEX.matcher(rhs);
		
		while (m.find()) {
			if (m.groupCount()!=2) {
				throw new IOException("Unexpected group count (" + m.groupCount() + ") for: " + rhs);
			}
			
			String keyStr = m.group(1);
			String value = m.group(2);
			if (properties.containsKey(keyStr)) {
				throw new IOException("Duplicate key: " + keyStr);
			}
			properties.put(keyStr, value);
		}
		
		//Now confirm
		for (String reqKey : ensure) {
			if (!properties.containsKey(reqKey)) {
				throw new IOException("Missing key: " + reqKey + " in: " + rhs);
			}
		}
		
		return properties;
	}
	
	
	
	public void setAgentPosition(){
	
		//Iterate through all agents in this tick
		TimeTick tick = ticks.get(curFrameNum);
		
		for (AgentTick agent : tick.agentTicks.values()) {		
			
			
			if(agent.agentType == SIGNAL)
			{
				phaseValue = agent.phaseValue;
				
			}
			else if(agent.agentType == DRIVER)
			{				
				//Scale the agent position
				double[] intArray = new double[2];
				intArray = scaleCoord((int)agent.agentX, (int)agent.agentY);
										
				agentXCoord[agent.agentID] = (int)intArray[0];
				agentYCoord[agent.agentID] = (int)intArray[1];			
				carDirection[agent.agentID] = agent.carDir;
				
			}
			else if(agent.agentType == PEDESTRIAN)
			{
				//Scale the agent position
				
				double[] intArray = new double[2];
				intArray = scaleCoord((int)agent.agentX, (int)agent.agentY);
										
				pedestrianXCoord[agent.agentID] = (int)intArray[0];
				pedestrianYCoord[agent.agentID] = (int)intArray[1];	
				
				// if agent has reached the destination, remove it from global array
				if(agent.agentPedestrianStatus == 1)
				{
					pedestrianXCoord[agent.agentID] = 0;
					pedestrianYCoord[agent.agentID] = 0;	
					
				}
				
				
			}
				
			frameNumLabel.setText("Frame #: " + curFrameNum);	
	
		}
	}
	
	public void resetCoord(int[] coordinationX, int[] coordinationY, int length){
		
		for(int i = 0; i< length; i++){
			
			coordinationX[i] = 0;
			coordinationY[i] = 0;
			
		}
		
		
	}
	
	public void stateChanged(ChangeEvent e) {
		if (e.getSource().equals(frameSlider)) {

			curFrameNum = frameSlider.getValue();
			setAgentPosition();
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
			
			resetCoord(pedestrianXCoord,pedestrianYCoord, numAgents);
			setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(stepForwardButton)) {
			if (curFrameNum<ticks.size()-1) {
				curFrameNum++;
				frameSlider.setValue(curFrameNum); //This should really be done with a listener.
				resetCoord(pedestrianXCoord,pedestrianYCoord, numAgents);
				setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(stepBackButton)) {
			if (curFrameNum>1) {
				curFrameNum--;
		
				frameSlider.setValue(curFrameNum); //This should really be done with a listener.
				resetCoord(pedestrianXCoord,pedestrianYCoord, numAgents);
				setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(resetButton)) {
			timer.stop();
			curFrameNum = 0;  //NOTE: The first tick is actually tick 0. ~Seth
	
			frameSlider.setValue(curFrameNum); //This should really be done with a listener.		
			resetCoord(pedestrianXCoord,pedestrianYCoord, numAgents);		
			// reset the global variable for pedestrian

			setAgentPosition();
			
			repaint();
		}
		else {  // Reset button
		// Complete this code: update clock and labels 
			//repaint();
		}
	}

	
	
}
