package sim_mob.vis.simultion;


import java.awt.Color;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.regex.Matcher;

import sim_mob.vis.network.Intersection;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.TrafficSignalLine;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Contains everything needed to display the results of a simulation.
 */
public class SimulationResults {
	public ArrayList<TimeTick> ticks;
	
	private static double[] xBounds;
	private static double[] yBounds;
	
	private static boolean OutOfBounds(double x, double y, RoadNetwork rn) {
		return     (x < rn.getTopLeft().x) || (x > rn.getLowerRight().x)
				|| (y < rn.getTopLeft().y) || (y > rn.getLowerRight().y);
	}
	
	public SimulationResults(BufferedReader inFile, RoadNetwork rn) throws IOException {
		ticks = new ArrayList<TimeTick>();
		
		//TEMP: Hack for agents which are out of bounds
		xBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		yBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		
		//Read
		String line;
		while ((line=inFile.readLine())!=null) {
			//Comment?
			line = line.trim();
			if (line.isEmpty() || !line.startsWith("(") || !line.endsWith(")")) {
				continue;
			}
			
			//Parse basic
		    Matcher m = Utility.LOG_LHS_REGEX.matcher(line);
		    if (!m.matches()) {
		      throw new IOException("Invalid line: " + line);
		    }
		    if (m.groupCount()!=4) {
		      throw new IOException("Unexpected group count (" + m.groupCount() + ") for: " + line);
		    }

		    //Known fields: type, id, rhs
		    String type = m.group(1);
		    int frameID = Integer.parseInt(m.group(2));
		    int objID = Utility.ParseIntOptionalHex(m.group(3));
		    String rhs = m.group(4);
		    
		    //Pass this off to a different function based on the type
		    try {
		    	dispatchConstructionRequest(type, frameID, objID, rhs, rn);
		    } catch (IOException ex) {
		    	throw new IOException(ex.getMessage() + "\n...on line: " + line);
		    }
		    
		    
		}
		
		//Modify traffic signal to make it stable
		Hashtable<Integer,SignalLineTick> oldSignal = new Hashtable<Integer, SignalLineTick>();

		//Now that the file has been loaded, scale agent positions to the RoadNetwork (so we can at least
		//  see something.)
		for (TimeTick tt : ticks) {
			for (AgentTick at : tt.agentTicks.values()) {
				//Skip pedestrians; they're already using the right coordinates
				if (!OutOfBounds(at.getPos().getUnscaledX(), at.getPos().getUnscaledY(), rn)) {
					continue;
				}
				
				//Get percent
				double percX = at.pos.getUnscaledX()/(xBounds[1]-xBounds[0]);
				double percY = at.pos.getUnscaledY()/(yBounds[1]-yBounds[0]);
				
				//Scale to RN
				double amtX = percX * (rn.getLowerRight().x - rn.getTopLeft().x);
				double amtY = percY * (rn.getLowerRight().y - rn.getTopLeft().y);
				
				//Translate to RN
				double resX = amtX + rn.getTopLeft().x;
				double resY = amtY + rn.getTopLeft().y;
				
				//Save
				at.pos = new ScaledPoint(resX, resY, tt.tickScaleGroup);
			}
		    
			
			if(tt.signalLineTicks.size()>0)
			{
				//Clean previous data
				oldSignal = new Hashtable<Integer, SignalLineTick>();
				//Assign new data
				oldSignal = tt.signalLineTicks;
			}
			else if(tt.signalLineTicks.size() == 0){
			
				if(oldSignal.size()!=0){
					tt.signalLineTicks = oldSignal;
				}
				else{
					System.out.println("Error, in modification of signal line ticks -- SimulationResults, constructor");
				}
				
			}
		}

		
	}
	
	//We assume the x/y bounds will be within those saved by the RoadNetwork.
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
		if (objType.equals("Driver")) {
			parseDriver(frameID, objID, rhs, rn);
		} else if (objType.equals("Signal")) {
//			parseSignal(frameID, objID, rhs);
			parseSignalLines(frameID, objID, rhs);
		} else if (objType.equals("pedestrian")) {
			parsePedestrian(frameID, objID, rhs, rn);
		}
	}
	
	private static Color ReadColor(int id) {
		if (id==1) {
			return Color.RED;
		} else if (id==2) {
			return Color.YELLOW;
		} else if (id==3) {
			return new Color(0x00, 0x99, 0x00);
		}
		throw new RuntimeException("Invalid traffic light color: " + id);
	}
	
	private static ArrayList<Integer> parseEachSignal(String signal){
		ArrayList<Integer> signalLights =  new ArrayList<Integer>();
		String [] items = signal.split(",");
		
		int leftLight, straightLight, rightLight;
		leftLight = Integer.parseInt(items[0]);
		straightLight = Integer.parseInt(items[1]);
		rightLight = Integer.parseInt(items[2]);
		
		signalLights.add(leftLight);
		signalLights.add(straightLight);
		signalLights.add(rightLight);
		
		
		return signalLights;	
	}
	
	private void parseSignal(int frameID, int objID, String rhs) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"va", "vb", "vc", "vd", "pa", "pb", "pc", "pd"});
	    
	    //Now save the relevant information.
	    Color[] vehicleLights = new Color[4];
	    Color[] pedestrianLights = new Color[4];
	    for (int i=0; i<4; i++) {
	    	String c = Character.toString((char)('a'+i));
	    	
	    	//NOTE: We are ignoring the other 2 signals in each TrafficLight; right now we just
	    	//      need to see if they roughly work.
	    	String tmp = props.get("v"+c);
	    	tmp = Character.toString(tmp.charAt(0));
	    	
	    	vehicleLights[i] = ReadColor(Integer.parseInt(tmp));
	    	pedestrianLights[i] = ReadColor(Integer.parseInt(props.get("p"+c)));
	    }

	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	t.signalTicks = new Hashtable<Integer, SignalTick>();
	    	t.signalLineTicks = new Hashtable<Integer,SignalLineTick>();
	    	ticks.add(t);
	    }
	    
	    //For now, just push multiple signals further in on the X axis
	    int xPos = (10+SignalTick.EstVisualSize())*ticks.get(frameID).signalTicks.size() + 10;
	    int yPos = 50;

	    //Add this Signal to the array
	    ticks.get(frameID).signalTicks.put(objID, new SignalTick(xPos, yPos, vehicleLights, pedestrianLights));

	}
	
	private void parseSignalLines(int frameID, int objID, String rhs) throws IOException{
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"va", "vb", "vc", "vd", "pa", "pb", "pc", "pd"});
	    
	    //Now save the relevant information.  
	    ArrayList<ArrayList<Integer>> allVehicleLights = new ArrayList<ArrayList<Integer>>();
	    allVehicleLights.add(parseEachSignal(props.get("va")));
	    allVehicleLights.add(parseEachSignal(props.get("vb")));
	    allVehicleLights.add(parseEachSignal(props.get("vc")));
	    allVehicleLights.add(parseEachSignal(props.get("vd")));
	    
	    ArrayList<Integer> allPedestrainLights = new ArrayList<Integer>();
	    allPedestrainLights.add(Integer.parseInt(props.get("pa")));
	    allPedestrainLights.add(Integer.parseInt(props.get("pb")));
	    allPedestrainLights.add(Integer.parseInt(props.get("pc")));
	    allPedestrainLights.add(Integer.parseInt(props.get("pd")));
	    
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	t.signalTicks = new Hashtable<Integer, SignalTick>();
	    	t.signalLineTicks = new Hashtable<Integer,SignalLineTick>();
	    	ticks.add(t);
	    }
	  

	    ticks.get(frameID).signalLineTicks.put(objID, new SignalLineTick(allVehicleLights, allPedestrainLights ,objID));
	 
	    
	}
	
	
	private void parseDriver(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    double angle = Double.parseDouble(props.get("angle"));
	    
	    //TEMP: Hack for out-of-bounds agents
	    if (OutOfBounds(xPos, yPos, rn)) {
	    	Utility.CheckBounds(xBounds, xPos);
	    	Utility.CheckBounds(yBounds, yPos);
	    }
	    
	    //Double-check angle
	    if (angle<0 || angle>360) {
	    	throw new IOException("Bad angle: " + angle + " for driver: " + objID);
	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	t.signalTicks = new Hashtable<Integer, SignalTick>();
	    	t.signalLineTicks = new Hashtable<Integer,SignalLineTick>();

	    	ticks.add(t);
	    }
	    
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, new DriverTick(xPos, yPos, angle, ticks.get(frameID).tickScaleGroup));
	}
	
	private void parsePedestrian(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    //Integer.parseInt(props.get("pedSig")); //Currently not used
	    
	    //TEMP: Hack for out-of-bounds agents
	    if (OutOfBounds(xPos, yPos, rn)) {
	    	Utility.CheckBounds(xBounds, xPos);
	    	Utility.CheckBounds(yBounds, yPos);
	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	t.signalTicks = new Hashtable<Integer, SignalTick>();
	    	t.signalLineTicks = new Hashtable<Integer,SignalLineTick>();
	    	ticks.add(t);
	    }
	    
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, new PedestrianTick(xPos, yPos, ticks.get(frameID).tickScaleGroup));
	}
}

