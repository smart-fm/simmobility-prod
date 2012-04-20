package sim_mob.vis.simultion;


import java.awt.Dimension;
import java.io.*;
import java.util.*;
import java.util.regex.Matcher;

import edu.umd.cs.piccolo.nodes.PPath;

import sim_mob.vis.network.*;
import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.network.basic.LocalPoint;
import sim_mob.vis.util.Utility;


/**
 * Contains everything needed to display the results of a simulation.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Anirudh Sivaraman
 */
public class SimulationResults {
	
	public ArrayList<TimeTick> ticks;
	
	public Hashtable<Integer, DriverTick> virtualDrivers;

	private static double[] xBounds;
	private static double[] yBounds;
	
	public int frame_length_ms;
	
	private static boolean OutOfBounds(double x, double y, RoadNetwork rn) {
		return     (x < rn.getTopLeft().x) || (x > rn.getLowerRight().x)
				|| (y < rn.getTopLeft().y) || (y > rn.getLowerRight().y);
	}
	
	public SimulationResults(BufferedReader inFile, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		ticks = new ArrayList<TimeTick>();
		virtualDrivers = new Hashtable<Integer, DriverTick>();
		
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

		//To fix those points that out of bounds
		for (TimeTick tt : ticks) {
			for (AgentTick at : tt.agentTicks.values()) {
				convertToLocalContext(at, rn);
			}
		}
		
		//TODO: Set up the initial position, currently this way is not efficient since it does twice
		for(DriverTick dt : virtualDrivers.values()){
			convertToLocalContext(dt,rn);
		}
						
	}
	
	//We assume the x/y bounds will be within those saved by the RoadNetwork.
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		if (objType.equals("Driver")) {
			parseDriver(frameID, objID, rhs, rn);
			uniqueAgentIDs.add(objID);
		} else if (objType.equals("BusDriver")) {
			//parseBusDriver(frameID, objID, rhs, rn);
			//uniqueAgentIDs.add(objID);
		} else if (objType.equals("Signal")) {
			parseSignal(frameID, objID, rhs,rn);
			//uniqueAgentIDs.add(objID); //NOTE: This should work! Need to check Signal ID code....
		} else if (objType.equals("pedestrian")) {
			//parsePedestrian(frameID, objID, rhs, rn);
			//uniqueAgentIDs.add(objID);
		} else if (objType.equals("simulation")) {
			parseSimulation(frameID, objID, rhs, rn);
		}
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
	    	throw new RuntimeException("Angle must be in bounds.");
	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	ticks.add(t);
	    }
	  
	    //Create temp driver
	    DriverTick tempDriver = new DriverTick(xPos, yPos, angle);
	    
	    
	    
//	    VirtualDriverTick tempVirtualDriver = new VirtualDriverTick();
	    
	    //Check if the driver is fake
	    if(props.containsKey("fake")){
	    	if(props.get("fake").equals("true")){
	    		tempDriver.setItFake();
	    	}
	    }
	    //Check if it's a "tracking" version of this car
	    boolean tracking = false;
	    if (props.containsKey("tracking")) {
	    	tracking = props.get("tracking").toLowerCase().equals("true");
	    }
	    
	    //Check if the car has a length and width or not
	    if(props.containsKey("length") && props.containsKey("width")){
	    	tempDriver.setLenth(Integer.parseInt(props.get("length")));
	    	tempDriver.setWidth(Integer.parseInt(props.get("width")));
	    }
	    
	    //Set its ID
	    tempDriver.setID(objID);
	    
	    //Set its type, 1 -> car
	    tempDriver.setType(1);

	    //Add this agent to the proper frame. If it's a "tracking" item, add it a parallel 
	    // list which contains tracking Agents
	    TimeTick currTick = ticks.get(frameID);
	    if (tracking) {
	    	//For now, just reuse the "fake" prperty
	    	tempDriver.setItFake();
	    	currTick.trackingTicks.put(objID, tempDriver);
	    } else {
	    	
	    	currTick.agentTicks.put(objID, tempDriver);
	    	currTick.agentIDs.add(objID);
	    }
	    
	    
	    //Driver list, used to determine the first frame ID it occurs
	    if(!virtualDrivers.containsKey(objID)){
	    	//Set the first frame ID
	    	tempDriver.setFirstFrameID(frameID);
	    	virtualDrivers.put(objID, tempDriver);
	    }
	    
	}

	private void parseSignal(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException{
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
	    	ticks.add(t);
	    }
	   
	    if(rn.getIntersection().containsKey(objID)){
	    	
	    	Intersection tempIntersection = rn.getIntersection().get(objID);
	    	ticks.get(frameID).crossingLightTicks.put(objID, new CrossingLightTick(objID,tempIntersection.getSigalCrossingIDs(),allPedestrainLights));
	    	
	    }else{
	    	System.out.println("Error, no such intersection -- parseSignal(), SimulationResults");
	    }
	    
	    
	    /*
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
	    */
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

	private void convertToLocalContext(AgentTick at, RoadNetwork rn){
		if (!OutOfBounds(at.getLocalPos().getUnscaledX(), at.getLocalPos().getUnscaledY(), rn)) {
			
			//Convert The coordinates to local system
			DPoint cornerTL = rn.getTopLeft();
			DPoint cornerLR = rn.getLowerRight();
			
			double width5Percent = 0.05 * (cornerLR.x - cornerTL.x);
			double height5Percent = 0.05 * (cornerLR.y - cornerTL.y);
			
			DPoint newTL = new DPoint(cornerTL.x-width5Percent, cornerTL.y-height5Percent);
			DPoint newLR = new DPoint(cornerLR.x+width5Percent, cornerLR.y+height5Percent);
			
			at.localPos.scaleVia(newTL, newLR, rn.getCanvasWidth(), rn.getCanvasHeight());
			at.angle = findProperAngle(at.angle, new Dimension(rn.getCanvasWidth(), rn.getCanvasHeight()));
			return;
		}
		//Get percent
		double percX = at.localPos.getUnscaledX()/(xBounds[1]-xBounds[0]);
		double percY = at.localPos.getUnscaledY()/(yBounds[1]-yBounds[0]);
		
		//Scale to RN
		double amtX = percX * (rn.getLowerRight().x - rn.getTopLeft().x);
		double amtY = percY * (rn.getLowerRight().y - rn.getTopLeft().y);
		
		//Translate to RN
		double resX = amtX + rn.getTopLeft().x;
		double resY = amtY + rn.getTopLeft().y;
		
		//Save
		at.localPos = new LocalPoint(resX, resY); 
		
		//Convert The coordinates to local system
		at.localPos.scaleVia(rn.getTopLeft(), rn.getLowerRight(), rn.getCanvasWidth(), rn.getCanvasHeight());
	}
	
	public double findProperAngle(double angle, Dimension naturalSize){
		double angleD = angle;
		
		if (naturalSize.width != naturalSize.height) {
			if (angle>0 && angle!=90 && angle!=180 && angle!=270 && angle<360) {
				//Guaranteed to be working with angles with non-zero x/y components, and a non-trivial skew factor.
				double xScale = (double)(naturalSize.width) / Math.max(naturalSize.width, naturalSize.height);
				double yScale = (double)(naturalSize.height) / Math.max(naturalSize.width,naturalSize.height);
				
				//System.out.println("Original angle: " + angleD);
				
				//Save the quadrant (y-mirrored). Also reduce angleD to Q1 (real)
				int quadrant = 0;

				if (angle<90) {
					quadrant = 1;
				} else if (angle<180) {
					quadrant = 2;
					angleD = 180-angleD;
				} else if (angle<270) {
					quadrant = 3;
					angleD -= 180;
				} else if (angle<360) {
					quadrant = 4;
					angleD = 360-angleD;
				} else { throw new RuntimeException("Bad angle: " + angle); }
				
				//Project the angle, scale it
				xScale *= Math.cos(angleD * Math.PI/180);
				yScale *= Math.sin(angleD * Math.PI/180);
				
				//Now retrieve the visual angle
				angleD = Math.atan(yScale/xScale) * 180/Math.PI;
				
				//Factor in quadrants again.
				if (quadrant==2) {
					angleD = 180 - angleD;
				} else if (quadrant==3) {
					angleD = 180 + angleD;
				} else if (quadrant==4) {
					angleD = 360 - angleD;
				} else if (quadrant!=1) { throw new RuntimeException("Bad quadrant: " + quadrant); }
				
				//System.out.println("   Fixed angle: " + angleD);
			}
		}
		
		//System.out.println(angle + "	"+angleD);
		return angleD;
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
}

