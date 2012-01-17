package sim_mob.vis.simultion;


import java.io.*;
import java.util.*;
import java.util.regex.Matcher;

import sim_mob.vis.network.*;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Contains everything needed to display the results of a simulation.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class SimulationResults {
	public ArrayList<TimeTick> ticks;
	
	private static double[] xBounds;
	private static double[] yBounds;
	
	public int frame_length_ms;
	
	private static boolean OutOfBounds(double x, double y, RoadNetwork rn) {
		return     (x < rn.getTopLeft().x) || (x > rn.getLowerRight().x)
				|| (y < rn.getTopLeft().y) || (y > rn.getLowerRight().y);
	}
	
	public SimulationResults(BufferedReader inFile, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		ticks = new ArrayList<TimeTick>();
		frame_length_ms = -1;
		
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
		    	dispatchConstructionRequest(type, frameID, objID, rhs, rn, uniqueAgentIDs);
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
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		if (objType.equals("Driver")) {
			parseDriver(frameID, objID, rhs, rn);
			uniqueAgentIDs.add(objID);
		} else if (objType.equals("Signal")) {
//			parseSignal(frameID, objID, rhs);
			parseSignalLines(frameID, objID, rhs);
			//uniqueAgentIDs.add(objID); //NOTE: This should work! Need to check Signal ID code....
		} else if (objType.equals("pedestrian")) {
			parsePedestrian(frameID, objID, rhs, rn);
			uniqueAgentIDs.add(objID);
		} else if (objType.equals("simulation")) {
			parseSimulation(frameID, objID, rhs, rn);
		}
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
	  
	    //Create a temp signal
	    SignalLineTick tempSignalLineTick = new SignalLineTick(allVehicleLights, allPedestrainLights ,objID);

	    //Check if signal is fake
	    if(props.containsKey("fake")){
	    	if(props.get("fake").equals("true")){
	    		tempSignalLineTick.setItFake();
	    	}
	    }
	    
	    //Add it to current time tick
	    ticks.get(frameID).signalLineTicks.put(objID, tempSignalLineTick);

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

	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	t.signalTicks = new Hashtable<Integer, SignalTick>();
	    	t.signalLineTicks = new Hashtable<Integer,SignalLineTick>();

	    	ticks.add(t);
	    }
	  
	    //Create temp driver
	    DriverTick tempDriver = new DriverTick(xPos, yPos, angle, ticks.get(frameID).tickScaleGroup);
	    
	    //Check if the driver is fake
	    if(props.containsKey("fake")){
	    	if(props.get("fake").equals("true")){
	    		tempDriver.setItFake();
	    	}
	    }
	    //Check if the car has a length and width or not
	    if(props.containsKey("length") && props.containsKey("width")){
	    	tempDriver.setLenth(Integer.parseInt(props.get("length")));
	    	tempDriver.setWidth(Integer.parseInt(props.get("width")));
	    }
	    
	    tempDriver.setID(objID);
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, tempDriver);
	    

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
	    
	    //Create a temp pedestrian
	    PedestrianTick tempPedestrian = new PedestrianTick(xPos, yPos, ticks.get(frameID).tickScaleGroup);
	    
	    //Check if the pedestrian is fake
	    if(props.containsKey("fake")){
	    	if(props.get("fake").equals("true")){
	    		tempPedestrian.setItFake();
	    	}
	    }
	  	
	    //Set ID
	    tempPedestrian.setID(objID);
	    
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, tempPedestrian);
	}

	
	private void parseSimulation(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"frame-time-ms"});
	    
	    //Check
	    if (frameID!=0) { throw new RuntimeException("Simulation block must have frame-id=0"); }
	    if (objID!=0) { throw new RuntimeException("Simulation block must have agent-id=0"); }
	    
	    //Now save the relevant information
	    this.frame_length_ms = Integer.parseInt(props.get("frame-time-ms"));
	}


}

