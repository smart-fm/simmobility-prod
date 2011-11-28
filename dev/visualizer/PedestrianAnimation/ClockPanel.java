import java.awt.*; 
import javax.swing.*; 
import java.awt.event.*; 
import java.io.*;
import java.util.*;
import javax.swing.Timer;

public class ClockPanel extends JPanel implements ActionListener {  
	private JButton startButton, stopButton, stepForwardButton, stepBackButton, resetButton; 
	private JLabel frameNumLabel, timerSpeedLabel;
	private JTextField timerSpeedInput;
	
	//Visualizer output customization
	private int pedestrianOvalSize = 7;		//Size of circles representing pedestrian
	private int timerSpeed = 1000; 			//Default set to 1 sec
	
	//Control variables
	String logFileName = "log_file";
	private int numAgents = 11;
	private int curFrameNum = 0;
	private int signalPhase = 0;
	private int lastFrameNum = 0;
	
	//Crossing boundaries read from output log
	private int readCrossingBottomLeftX = 0;
	private int readCrossingBottomLeftY = 0;
	private int readCrossingBottomRightX = 0;
	private int readCrossingBottomRightY = 0;
	private int readCrossingTopLeftX = 0;
	private int readCrossingTopLeftY = 0;
	private int readCrossingTopRightX = 0;
	private int readCrossingTopRightY = 0;
	
	private int initialCrossingX1 = readCrossingBottomLeftX;
	private int initialCrossingX2 = readCrossingBottomRightX;
	private int initialCrossingY1 = readCrossingBottomRightY;
	private int initialCrossingY2 = readCrossingTopLeftY;
	
	//Currently value is hard coded. Does not dynamically change to fit screen size of user.
	private int scaledCrossingX1 = 360;
	private int scaledCrossingX2 = 440;
	private int scaledCrossingY1 = 115;
	private int scaledCrossingY2 = 515;
	
	//Array list for each (id, frameNum, x, y, signalPhase) line read in output log
	private ArrayList<String> arl = new ArrayList<String>();
	
	//int array of (x,y) coordinates of pedestrians
	
	private int[] agentXCoord;
	//= new int[numAgents];
	private int[] agentYCoord;
	//= new int[numAgents];
	
	//Timer for speed adjustment of visualizer refresh rate
	Timer timer = new Timer(timerSpeed, this);
	
	public ClockPanel(){ 	
		
		JPanel bottomPanel = new JPanel(); 
		JPanel topPanel = new JPanel();
		
		startButton = new JButton("Start");
		stopButton = new JButton("Stop");
		stepForwardButton = new JButton("Step Forward");
		stepBackButton = new JButton("Step Backward");
		resetButton = new JButton("Reset");    
		frameNumLabel = new JLabel("Frame #: " + curFrameNum);	//frame number
		timerSpeedLabel = new JLabel("	Timer Speed (ms): ");
		timerSpeedInput = new JTextField(4);
				
		bottomPanel.add(startButton);
		bottomPanel.add(stopButton);
		bottomPanel.add(stepForwardButton);
		bottomPanel.add(stepBackButton);
		bottomPanel.add(resetButton); 
		topPanel.add(frameNumLabel);
		topPanel.add(timerSpeedLabel);
		topPanel.add(timerSpeedInput);
		
		setLayout(new BorderLayout()); 
		add(bottomPanel, BorderLayout.SOUTH);
		add(topPanel, BorderLayout.NORTH);
		
		startButton.addActionListener(this);
		stopButton.addActionListener(this);
		stepForwardButton.addActionListener(this);
		stepBackButton.addActionListener(this);
		resetButton.addActionListener(this);
		timerSpeedInput.addActionListener(this);
		
		readFile();
		System.out.println("File Read Done!");
	}

	public void readFile() {
		
		try {
			FileReader fin = new FileReader(logFileName);
			BufferedReader b = new BufferedReader(fin);
			
			String currentLine = "";
			while((currentLine = b.readLine()) != null) {		
				int startPos = 0, midPos = 0, endPos = 0;
				
				if(currentLine.contains("Boundary[bottomleft]")) {
					//not used for now
				}
				else if(currentLine.contains("Boundary[bottomright]")) {
					//not used for now
				}
				else if(currentLine.contains("Boundary[topleft]")) {
					//not used for now
				}
				else if(currentLine.contains("Boundary[topright]")) {
					//not used for now
				}
				else if(currentLine.contains("Crossing[bottomleft]")) {
					for(int i=0; i<currentLine.length(); i++) {
						if(currentLine.charAt(i) == '(')
							startPos = i;
						else if(currentLine.charAt(i) == ',')
							midPos = i;
						else if(currentLine.charAt(i) == ')')
							endPos = i;
					}
					if(startPos < midPos)
						readCrossingBottomLeftX = Integer.parseInt(currentLine.substring(startPos+1, midPos));
					else {}
						//throw exception
						
					if(endPos > midPos)
						readCrossingBottomLeftY = Integer.parseInt(currentLine.substring(midPos+1, endPos));
					else {}
						//throw exception
					
					initialCrossingX1 = readCrossingBottomLeftX;
					System.out.println("Crossing[bottomleft]: " + readCrossingBottomLeftX + ", " + readCrossingBottomLeftY);
				}
				else if(currentLine.contains("Crossing[bottomright]")) {
					for(int i=0; i<currentLine.length(); i++) {
						if(currentLine.charAt(i) == '(')
							startPos = i;
						else if(currentLine.charAt(i) == ',')
							midPos = i;
						else if(currentLine.charAt(i) == ')')
							endPos = i;
					}
					if(startPos < midPos)
						readCrossingBottomRightX = Integer.parseInt(currentLine.substring(startPos+1, midPos));
					else {}
						//throw exception
						
					if(endPos > midPos)
						readCrossingBottomRightY = Integer.parseInt(currentLine.substring(midPos+1, endPos));
					else {}
						//throw exception
					
					initialCrossingX2 = readCrossingBottomRightX;
					initialCrossingY1 = readCrossingBottomRightY;
					System.out.println("Crossing[bottomright]: " + readCrossingBottomRightX + ", " + readCrossingBottomRightY);
				}
				else if(currentLine.contains("Crossing[topleft]")) {
					for(int i=0; i<currentLine.length(); i++) {
						if(currentLine.charAt(i) == '(')
							startPos = i;
						else if(currentLine.charAt(i) == ',')
							midPos = i;
						else if(currentLine.charAt(i) == ')')
							endPos = i;
					}
					if(startPos < midPos)
						readCrossingTopLeftX = Integer.parseInt(currentLine.substring(startPos+1, midPos));
					else {}
						//throw exception
						
					if(endPos > midPos)
						readCrossingTopLeftY = Integer.parseInt(currentLine.substring(midPos+1, endPos));
					else {}
						//throw exception
					
					initialCrossingY2 = readCrossingTopLeftY;
					System.out.println("Crossing[topleft]: " + readCrossingTopLeftX + ", " + readCrossingTopLeftY);
				}
				else if(currentLine.contains("Crossing[topright]")) {
					for(int i=0; i<currentLine.length(); i++) {
						if(currentLine.charAt(i) == '(')
							startPos = i;
						else if(currentLine.charAt(i) == ',')
							midPos = i;
						else if(currentLine.charAt(i) == ')')
							endPos = i;
					}
					if(startPos < midPos)
						readCrossingTopRightX = Integer.parseInt(currentLine.substring(startPos+1, midPos));
					else {}
						//throw exception
						
					if(endPos > midPos)
						readCrossingTopRightY = Integer.parseInt(currentLine.substring(midPos+1, endPos));
					else {}
						//throw exception
					
					System.out.println("Crossing[topright]: " + readCrossingTopRightX + ", " + readCrossingTopRightY);
				}
				else if(currentLine.contains("Agents Initialized:")) {
					numAgents = Integer.parseInt(currentLine.substring(22));
					agentXCoord = new int[numAgents];
					agentYCoord = new int[numAgents];
				}
				else if(currentLine.startsWith("(Agent")) {		//Adding Agent Finish Message to array list
					//System.out.println(currentLine);
					
					arl.add(currentLine);
					System.out.println(currentLine);
					
				}
				else if(currentLine.startsWith("(")) {
					String[] temp;
					temp = currentLine.split(",");
					lastFrameNum = Integer.parseInt(temp[1]);	//not that ingenious, might change
					
					arl.add(currentLine);
					System.out.println(currentLine);
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
		
		/*
		try {
			FileWriter fstream = new FileWriter("log2.out");
			BufferedWriter out = new BufferedWriter(fstream);
			
			for(int i=0; i<arl.size(); i++) {
				if(arl.get(i).charAt(0)=='(') {
					out.write(arl.get(i));
					out.write("\n");
				}
			}
			out.close();
		}
		catch(Exception e) {
			System.err.println("Error");
		}
		*/
		
		//System.out.println(arl);
	}
	
	//Scale (x,y) of pedestrian to fit into screen size
	public double[] scaleCoord(int x, int y) {
		double[] dblArray = new double[2];
		double newX, newY;
		double fractionX, fractionY;
		
		fractionX = (double)(x-initialCrossingX1) / (initialCrossingX2-initialCrossingX1);
		newX = scaledCrossingX1 + (fractionX*(double)(scaledCrossingX2-scaledCrossingX1));
		
		fractionY = (double)(y-initialCrossingY1) / (initialCrossingY2-initialCrossingY1);
		newY = scaledCrossingY1 + (fractionY*(double)(scaledCrossingY2-scaledCrossingY1));
		
		dblArray[0] = newX;
		dblArray[1] = newY;
		
		return dblArray;
	}
	
	public void paintComponent(Graphics g) { 
		super.paintComponent(g); 
		
		//Draw crossing boundaries
		g.drawLine(scaledCrossingX1, scaledCrossingY1, scaledCrossingX1, scaledCrossingY2);	//(x1,y1), (x2,y2)
		g.drawLine(scaledCrossingX2, scaledCrossingY1, scaledCrossingX2, scaledCrossingY2);
		
		//Draw road
		int roadSideWidth = 5;
		g.setColor(Color.BLACK);
		g.drawRect(0, scaledCrossingY1-roadSideWidth, 1000, roadSideWidth);
		g.fillRect(0, scaledCrossingY1-roadSideWidth, 1000, roadSideWidth);
		g.drawRect(0, scaledCrossingY2, 1000, roadSideWidth);
		g.fillRect(0, scaledCrossingY2, 1000, roadSideWidth);
		
		//Draw road divider
		int dividerWidth = 40;
		for(int i=0; i<10; i++) {
			if((dividerWidth*2*i < scaledCrossingX1-dividerWidth) || (dividerWidth*2*i > scaledCrossingX2)) {
				g.drawRect(dividerWidth*2*i, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);
				g.setColor(Color.BLACK);
				g.fillRect(dividerWidth*2*i, scaledCrossingY1+((scaledCrossingY2-scaledCrossingY1)/2), dividerWidth, roadSideWidth);
			}
		}
		
		//Draw traffic light
		g.drawOval(scaledCrossingX2+10, scaledCrossingY1+10, 20, 20);
		if(signalPhase==0)
			g.setColor(Color.GREEN);
		else
			g.setColor(Color.RED);
		g.fillOval(scaledCrossingX2+10, scaledCrossingY1+10, 20, 20);
		g.setColor(Color.BLACK);
		g.drawString("Pedestrian Traffic Light", scaledCrossingX2+40, scaledCrossingY1+20);
		
		//Draw pedestrians
		setAgentPosition();
		
		for(int i=0; i<numAgents; i++) {
			g.setColor(Color.BLACK);
			g.drawOval(agentXCoord[i]-(pedestrianOvalSize/2), agentYCoord[i]-(pedestrianOvalSize/2), pedestrianOvalSize, pedestrianOvalSize);
			g.drawString(Integer.toString(i), agentXCoord[i]+pedestrianOvalSize, agentYCoord[i]+pedestrianOvalSize);
			g.setColor(Color.PINK);
			g.fillOval(agentXCoord[i]-(pedestrianOvalSize/2), agentYCoord[i]-(pedestrianOvalSize/2), pedestrianOvalSize, pedestrianOvalSize);
		}
	}

	public void setAgentPosition(){  
		
		//1. Check whether it is a Position Update Vector or Agent Finish Message
		//2. If Position Update Vector, check whether Frame Number == Current Frame Number for updating position coord
				
		//Iterate thru all the output log lines
		for(int i=0; i<arl.size(); i++) {			
			String temp;
			temp = arl.get(i);
			temp = temp.substring(1, temp.length()-1);	//strip brackets from read line
			
			if(temp.startsWith("Agent")) {	//if it's "Agent X has reached goal" message
			/*
				int agentNum;
				String[] temp3;
				temp3 = temp.split(" ");	//tokenize by white space
								
				agentNum = Integer.parseInt(temp3[1]);	//convert agent number to integer
				
				System.out.println("Test agent " + i);
				//System.out.println("Agent " + agentNum + " has finished!");
			*/
			}
			else {	//normal position update message
				for(int j=0; j<numAgents; j++) {		
					String[] temp2;
					temp2 = temp.split(",");	//tokenize numbers separated by colon					
					
					int tempId = Integer.parseInt(temp2[0]);
					int tempFrameNum = Integer.parseInt(temp2[1]);
					int tempX = (int)Double.parseDouble(temp2[2]);
					int tempY = (int)Double.parseDouble(temp2[3]);
					
					double[] intArray = new double[2];
					intArray = scaleCoord(tempX, tempY);
						
					if(tempFrameNum == curFrameNum) {	//might be redundant to check for this
						signalPhase = Integer.parseInt(temp2[4]);
						
						agentXCoord[tempId] = (int)intArray[0];
						agentYCoord[tempId] = (int)intArray[1];					
					}
				}//end for
			}//end else
						
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
			if(curFrameNum == lastFrameNum)
				timer.stop();
			else {
				curFrameNum++;
				setAgentPosition();
				repaint();
			}
		}
		else if(e.getSource().equals(stepForwardButton)) {
			curFrameNum++;
			setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(stepBackButton)) {
			curFrameNum--;
			setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(resetButton)) {
			timer.stop();
			curFrameNum = 0;
			setAgentPosition();
			repaint();
		}
		else if(e.getSource().equals(timerSpeedInput)) {
			timerSpeed = Integer.parseInt(timerSpeedInput.getText());
			timer.setDelay(timerSpeed);
		}
		else {
			repaint();}
	}
}	



