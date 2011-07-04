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
	
	private int numAgents = 11;
	private int curFrameNum = 0;
	private int signalPhase = 0;
	private int initialCrossingX1 = 498;
	private int initialCrossingX2 = 502;
	private int initialCrossingY1 = 190;
	private int initialCrossingY2 = 240;
	private int scaledCrossingX1 = 360;
	private int scaledCrossingX2 = 440;
	private int scaledCrossingY1 = 115;
	private int scaledCrossingY2 = 515;
	
	private int pedestrianOvalSize = 7;
	private int timerSpeed = 1000; //1 sec
	//private double scaleFactor = 1.0;
	
	//Array list for each pedestrian output in log.out
	private ArrayList<String> arl = new ArrayList<String>();
	
	//X, Y coord of agents
	private int[] agentXCoord = new int[numAgents];
	private int[] agentYCoord = new int[numAgents];
	
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
		//add(bottomPanel, BorderLayout.EAST);
		
		// Who will listen to the button events? Your code here 
		startButton.addActionListener(this);
		stopButton.addActionListener(this);
		stepForwardButton.addActionListener(this);
		stepBackButton.addActionListener(this);
		resetButton.addActionListener(this);
		timerSpeedInput.addActionListener(this);
		
		readFile();
	}

	public void readFile() {
		char[] boundaryBottomLeft, boundaryBottomRight, boundaryTopLeft, boundaryTopRight;
		
		
		try {
			FileReader fin = new FileReader("log2s.out");
			BufferedReader b = new BufferedReader(fin);
			
			String currentLine = "";
			while((currentLine = b.readLine()) != null) {
				//if(currentLine.compareTo("Boundary[bottomleft]")==0) {
				//	currentLine
					
				//}
						
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
	
	//Re-adjust pedestrian's position to accommodate x-axis scaling
	public double[] scaleCoord(int x, int y) {
		double[] dblArray = new double[2];
		double newX, newY;
		double fractionX, fractionY;
		
		//What if x-initialCrossingX1 == 0?
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
		//g.drawRect(50, 50, 650, 650);	//x, y, width, height
		
		//Draw world boundary
		//g.drawRect(100, 50, 600, 550);
		
		//Draw crossing boundaries
		g.drawLine(scaledCrossingX1, scaledCrossingY1, scaledCrossingX1, scaledCrossingY2);	//(x1,y1), (x2,y2)
		g.drawLine(scaledCrossingX2, scaledCrossingY1, scaledCrossingX2, scaledCrossingY2);
		//g.drawLine(400, 240, 500, 240);
		//g.drawLine(400, 290, 500, 290);
		//g.drawLine(400, 240, 500, 290);
		//g.drawLine(400, 290, 500, 240);
		
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
			//g.drawString(Integer.toString(i), agentXCoord[i]+pedestrianOvalSize, agentYCoord[i]+pedestrianOvalSize);
			g.setColor(Color.PINK);
			g.fillOval(agentXCoord[i]-(pedestrianOvalSize/2), agentYCoord[i]-(pedestrianOvalSize/2), pedestrianOvalSize, pedestrianOvalSize);
		}
	}

	public void setAgentPosition(){  
		
		//Iterate thru all the lines(agents) read from log.out
		for(int i=0; i<arl.size(); i++) {	
			String temp;
			temp = arl.get(i);
			temp = temp.substring(1, temp.length()-1);
			//System.out.println(temp);
			
			String[] temp2;
			temp2 = temp.split(",");
			//System.out.println(temp2[4]);
		
			int tempId = Integer.parseInt(temp2[0]);
			int tempFrameNum = Integer.parseInt(temp2[1]);
			//int tempX = Integer.parseInt(temp2[2]);
			//int tempY = Integer.parseInt(temp2[3]);
			int tempX = (int)Double.parseDouble(temp2[2]);
			int tempY = (int)Double.parseDouble(temp2[3]);
			//signalPhase = Integer.parseInt(temp2[4]);
			
			double[] intArray = new double[2];
			intArray = scaleCoord(tempX, tempY);
				
			if(tempFrameNum == curFrameNum) {
				signalPhase = Integer.parseInt(temp2[4]);
				
				agentXCoord[tempId] = (int)intArray[0];
				agentYCoord[tempId] = (int)intArray[1];
			}
		
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
			curFrameNum++;
			setAgentPosition();
			repaint();
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
			//scaleFactor = Double.parseDouble(scaleFactorTextField.getText());
			//System.out.println("Scale Factor: " + scaleFactor);
		}
		else {  // Reset button
		// Complete this code: update clock and labels 
			repaint();}
	}
}	



