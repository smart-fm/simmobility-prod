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
 * \author Anirudh Sivaraman
 */
public class SimulationResults {
	public ArrayList<TimeTick> ticks;
	
	private static double[] xBounds;
	private static double[] yBounds;
	
	public int frame_length_ms;
	
	//Class for holding log lines and a tag as to the type
	public static class LogFileLine {
		public String line;
		public boolean isNewStyle;
		public LogFileLine(String line, boolean isNewStyle) {
			this.line = line;
			this.isNewStyle = isNewStyle;
		}
	}
	
	public SimulationResults(BufferedReader inFile, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		ticks = new ArrayList<TimeTick>();
		frame_length_ms = -1;
		
		//TEMP: Hack for agents which are out of bounds
		xBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		yBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		
		//Read
		String line;
		ArrayList<LogFileLine> lineBuffer = new ArrayList<LogFileLine>();
		SimpleThreadPool stp = new SimpleThreadPool(10); //No more than 10 threads at once.
		while ((line=inFile.readLine())!=null) {
			//Comment?
			line = line.trim();
			if (line.isEmpty() || line.startsWith("#")) { continue; }
			
			//There are three types of lines. "Old style" begin and end with (, ). "New style" are 
			// json-formatted and begin/end with {,}. Anything else is a comment.
			boolean oldStyle = line.startsWith("(") && line.endsWith(")");
			boolean newStyle = line.startsWith("{") && line.endsWith("}");
			if (!oldStyle && !newStyle) {
				continue;
			}
			//Add to array
			lineBuffer.add(new LogFileLine(line, newStyle));
			if (lineBuffer.size()>LINE_BUFFER_LIMIT) {
				//Push to thread, clear buffer.
				ArrayList<LogFileLine> temp = lineBuffer;
				lineBuffer = new ArrayList<LogFileLine>(); //Can't "clear", because we keep a reference.
				stp.newTask(new SimResLineParser(temp, this, uniqueAgentIDs));
			}
		}
		
		
		//Any remaining lines?
		if (!lineBuffer.isEmpty()) {
			//Push to thread
			stp.newTask(new SimResLineParser(lineBuffer, this, uniqueAgentIDs), true);
		}
		
		if (frame_length_ms==-1) {
			throw new RuntimeException("Error: missing \"simulation\" tag.");
		}
		
		//Modify traffic signal to make it stable
		Hashtable<Integer,SignalLineTick> oldSignal = new Hashtable<Integer, SignalLineTick>();

		//Now that the file has been loaded, scale agent positions to the RoadNetwork (so we can at least
		//  see something.)
		for (TimeTick tt : ticks) {
			for (AgentTick at : tt.agentTicks.values()) {
				//Skip pedestrians; they're already using the right coordinates
				/*if (!OutOfBounds(at.getPos().getUnscaledX(), at.getPos().getUnscaledY(), rn)) {
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
				at.pos = new ScaledPoint(resX, resY);*/
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
	
	
	private static class TemporarySimObjects {
		Hashtable<Integer, ArrayList<AgentTick>> agentTicksToAdd = new Hashtable<Integer, ArrayList<AgentTick>>();
		Hashtable<Integer, ArrayList<AgentTick>> trackingTicksToAdd = new Hashtable<Integer, ArrayList<AgentTick>>();
		Hashtable<Integer, ArrayList<SignalLineTick>> signalLineTicksToAdd = new Hashtable<Integer, ArrayList<SignalLineTick>>();
		Hashtable<Integer, ArrayList<GsonResObj>> gsonObjectsToAdd = new Hashtable<Integer, ArrayList<GsonResObj>>();
		int tempFrameLenMS = -1;
	}
	
	
	
	//Begin = parse all objects (no sync.)
	//End = save all objects (sync)
	private static class SimResLineParser extends BifurcatedActivity {
		ArrayList<LogFileLine> lines;
		SimulationResults sim;
		HashSet<Integer> uniqueAgentIDs;
		TemporarySimObjects resObj;
		
		//TEMP
		FastLineParser flp;
		
		SimResLineParser(ArrayList<LogFileLine> lines, SimulationResults sim, HashSet<Integer> uniqueAgentIDs) {
			this.lines = lines;
			this.sim = sim;
			this.uniqueAgentIDs = uniqueAgentIDs;
			this.resObj = new TemporarySimObjects();
			
			flp = new FastLineParser();
		}
		
		public Object begin(Object... args) {
			try {
				for (LogFileLine logLine : lines) {
					//Parsing depends on how the line is structured.
					if (logLine.isNewStyle) {
						//Parse this line as json.
						GsonResObj gRes = Utility.ParseGsonLine(logLine.line);
						int tTick = gRes.getTimeTick();
						
						//Save this object for later.
						if (!resObj.gsonObjectsToAdd.containsKey(tTick)) {
							resObj.gsonObjectsToAdd.put(tTick, new ArrayList<GsonResObj>());
						}
						resObj.gsonObjectsToAdd.get(tTick).add(gRes);
					} else {
						//Parse this line as text and pseudo-json.
						Utility.ParseResults pRes = Utility.ParseLogLine(flp, logLine.line);
						if (pRes.isError()) {
							throw new RuntimeException("Error parsing line: \n  " + pRes.errorMsg);
						}

					    //Pass this off to a different function based on the type
					    try {
					    	dispatchConstructionRequest(pRes);
					    } catch (IOException ex) {
					    	throw new IOException(ex.getMessage() + "\n...on line: " + logLine.line);
					    }
					}
				}
			} catch (IOException ex) {
				//TODO: Handle in a more thread-safe way.
				throw new RuntimeException(ex);
			}
			
			//N//A
			return null;
		}
		
		
		public Object end(Object... args) {
			//Synchronize on all adds
			synchronized (sim) {
				//Save the simulation time tick.
				if (resObj.tempFrameLenMS != -1) {
					sim.frame_length_ms = resObj.tempFrameLenMS;
				}
				
				//Add all Gson items
				for (Entry<Integer, ArrayList<GsonResObj>> gsonResObjs : resObj.gsonObjectsToAdd.entrySet()) {
					for (GsonResObj gRes : gsonResObjs.getValue()) {
						gRes.addSelfToSimulation(null, sim);
					}
				}
				
				//Add all agents
				for (Entry<Integer, ArrayList<AgentTick>> agTimeTick : resObj.agentTicksToAdd.entrySet()) {
					for (AgentTick agTick : agTimeTick.getValue()) {
					    //Ensure the frame has been created
						sim.reserveTimeTick(agTimeTick.getKey());
						
					    //Add this agent to the proper frame. If it's a "tracking" item, add it a parallel 
					    // list which contains tracking Agents
						sim.addAgent(agTimeTick.getKey(), agTick, true);
						
						//Update unique IDs
						uniqueAgentIDs.add(agTick.getID());
					}
				}
				
				//Add all pending "tracking" objects.
				for (Entry<Integer, ArrayList<AgentTick>> agTimeTick : resObj.trackingTicksToAdd.entrySet()) {
					for (AgentTick agTick : agTimeTick.getValue()) {
					    //Ensure the frame has been created
						sim.reserveTimeTick(agTimeTick.getKey());
						
					    //Add this agent to the proper frame. If it's a "tracking" item, add it a parallel 
					    // list which contains tracking Agents
						sim.addAgent(agTimeTick.getKey(),agTick, false);
						
						//Update unique IDs
						uniqueAgentIDs.add(agTick.getID());
					}
				}
				
				//Add all pending "signal" objects
				for (Entry<Integer, ArrayList<SignalLineTick>> sigTimeTick : resObj.signalLineTicksToAdd.entrySet()) {
					for (SignalLineTick sigTick : sigTimeTick.getValue()) {
					    //Ensure the frame has been created
						sim.reserveTimeTick(sigTimeTick.getKey());
						
						//Add it
						sim.addSignal(sigTimeTick.getKey(), sigTick);
						

						//TODO: This should work! Check signal code.
						//uniqueAgentIDs.add(agTick.getID());
					}
				}
			}
			
			
			//N/A
			return null;
		}
		
		
		//Returns true if this was a known property
		boolean dispatchConstructionRequest(Utility.ParseResults pRes) throws IOException {
			if (pRes.type.equals("Driver")) {
				parseDriver(pRes);
			} else if (pRes.type.equals("BusDriver")) {
				parseBusDriver(pRes);
			} else if (pRes.type.equals("Signal")) {
				parseSignalLines(pRes);
			} else if (pRes.type.equals("pedestrian")) {
				parsePedestrian(pRes);
			} else if (pRes.type.equals("simulation")) {
				parseSimulation(pRes);
			} else {
				if (pRes.frame>0) {
					System.out.println("WARNING: Unknown type: " + pRes.type);
				}
				return false; //Couldn't process
			}
			return true;
		}
		
		
		//TODO: Use generics
		void saveTempAgent(int frameID, AgentTick at, boolean isTracking) {
			//Expand array, add it
		    Hashtable<Integer, ArrayList<AgentTick>> toAddHash = isTracking ? resObj.trackingTicksToAdd : resObj.agentTicksToAdd;
		    if (!toAddHash.containsKey(frameID)) {
		    	toAddHash.put(frameID, new ArrayList<AgentTick>());
		    }
		    toAddHash.get(frameID).add(at);
		}
		
		void saveTempSignal(int frameID, SignalLineTick st) {
		    Hashtable<Integer, ArrayList<SignalLineTick>> toAddHash = resObj.signalLineTicksToAdd;
		    if (!toAddHash.containsKey(frameID)) {
		    	toAddHash.put(frameID, new ArrayList<SignalLineTick>());
		    }
		    toAddHash.get(frameID).add(st);
		}
		
		
		void parseDriver(Utility.ParseResults pRes) throws IOException {			
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"xPos", "yPos", "angle"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(pRes.properties.get("xPos"));
		    double yPos = Double.parseDouble(pRes.properties.get("yPos"));
		    double angle = Double.parseDouble(pRes.properties.get("angle"));
		    
		    //See if we have a message icon to show
		    DriverTick.RxLocation msgLoc = null;
		    if (pRes.properties.containsKey("rxLong") && pRes.properties.containsKey("rxLat")) {
		    	msgLoc = new DriverTick.RxLocation();
		    	msgLoc.longitude = Double.parseDouble(pRes.properties.get("rxLong"));
		    	msgLoc.latitude = Double.parseDouble(pRes.properties.get("rxLat"));
		    }
		    
		    //Double-check angle
		    if (angle<0 || angle>360) {
		    	throw new RuntimeException("Angle must be in bounds.");
		    }
		  
		    //Create temp driver
		    DriverTick tempDriver = new DriverTick(pRes.objID, xPos, yPos, angle, msgLoc);
		    
		    //Check if the driver is fake
		    if(pRes.properties.containsKey("fake")){
		    	if(pRes.properties.get("fake").equals("true")){
		    		tempDriver.setItFake();
		    	}
		    }
		    //Check if it's a "tracking" version of this car
		    boolean tracking = false;
		    if (pRes.properties.containsKey("tracking")) {
		    	tracking = pRes.properties.get("tracking").toLowerCase().equals("true");
		    }
		    //Check if the car has a length and width or not
		    if(pRes.properties.containsKey("length") && pRes.properties.containsKey("width")){
		    	tempDriver.setLenth(Integer.parseInt(pRes.properties.get("length")));
		    	tempDriver.setWidth(Integer.parseInt(pRes.properties.get("width")));
		    }
		    
		    
		    if (tracking) {
		    	//For now, just reuse the "fake" prperty
		    	tempDriver.setItFake();
		    }
		    
		    
		  //Add it to our temporary list
		  saveTempAgent(pRes.frame, tempDriver, tracking);
		}
		
		
		//TODO: This shares a lot of functionality with parseDriver(). Can we merge some of it?
		void parseBusDriver(Utility.ParseResults pRes) throws IOException {
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"xPos", "yPos", "angle", "passengers"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(pRes.properties.get("xPos"));
		    double yPos = Double.parseDouble(pRes.properties.get("yPos"));
		    double angle = Double.parseDouble(pRes.properties.get("angle"));
		    int numPassengers = Integer.parseInt(pRes.properties.get("passengers"));
		  
		    //Create temp driver
		    BusDriverTick tempBusDriver = new BusDriverTick(pRes.objID, xPos, yPos, angle, numPassengers);
		    
		    //Check if the driver is fake
		    if(pRes.properties.containsKey("fake")){
		    	if(pRes.properties.get("fake").equals("true")){
		    		tempBusDriver.setItFake();
		    	}
		    }
		    //Check if the car has a length and width or not
		    if(pRes.properties.containsKey("length") && pRes.properties.containsKey("width")){
		    	tempBusDriver.setLenth(Integer.parseInt(pRes.properties.get("length")));
		    	tempBusDriver.setWidth(Integer.parseInt(pRes.properties.get("width")));
		    }

		    //Add it to our temporary list
		    saveTempAgent(pRes.frame, tempBusDriver, false);
		}

		
	}
	
	//We assume the x/y bounds will be within those saved by the RoadNetwork.
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs) throws IOException {
		if (objType.equals("Driver")) {
			parseDriver(frameID, objID, rhs, rn);
			uniqueAgentIDs.add(objID);
		} else if (objType.equals("BusDriver")) {
			parseBusDriver(frameID, objID, rhs, rn);
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
	    //double rxLong=Double.parseDouble(props.get("rxLong"));
	    //double rxLat=Double.parseDouble(props.get("rxLat"));
	    
	    //See if we have a message icon to show
	    DriverTick.RxLocation msgLoc = null;
	    if (props.containsKey("rxLong") && props.containsKey("rxLat")) {
	    	msgLoc = new DriverTick.RxLocation();
	    	msgLoc.longitude = Double.parseDouble(props.get("rxLong"));
	    	msgLoc.latitude = Double.parseDouble(props.get("rxLat"));
	    }
	    
	    
	    //TEMP: Hack for out-of-bounds agents
	    /*if (OutOfBounds(xPos, yPos, rn)) {
	    	Utility.CheckBounds(xBounds, xPos);
	    	Utility.CheckBounds(yBounds, yPos);
	    }*/
	    
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
	    DriverTick tempDriver = new DriverTick(xPos, yPos, angle, msgLoc);
	    
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
	    
	    tempDriver.setID(objID);
	    
	    //Add this agent to the proper frame. If it's a "tracking" item, add it a parallel 
	    // list which contains tracking Agents
	    TimeTick currTick = ticks.get(frameID);
	    if (tracking) {
	    	//For now, just reuse the "fake" prperty
	    	tempDriver.setItFake();
	    	currTick.trackingTicks.put(objID, tempDriver);
	    } else {
	    	currTick.agentTicks.put(objID, tempDriver);
	    }
	}
	
	
	//TODO: This shares a lot of functionality with parseDriver(). Can we merge some of it?
	private void parseBusDriver(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle","passengers"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    double angle = Double.parseDouble(props.get("angle"));
	    int numPassengers = Integer.parseInt(props.get("passengers"));
	    //double rxLong=Double.parseDouble(props.get("rxLong"));
	    //double rxLat=Double.parseDouble(props.get("rxLat"));
	    
	    //See if we have a message icon to show
	    BusDriverTick.RxLocation msgLoc = null;
	    if (props.containsKey("rxLong") && props.containsKey("rxLat")) {
	    	msgLoc = new BusDriverTick.RxLocation();
	    	msgLoc.longitude = Double.parseDouble(props.get("rxLong"));
	    	msgLoc.latitude = Double.parseDouble(props.get("rxLat"));
	    }
	    
	    
	    //TEMP: Hack for out-of-bounds agents
	    /*if (OutOfBounds(xPos, yPos, rn)) {
	    	Utility.CheckBounds(xBounds, xPos);
	    	Utility.CheckBounds(yBounds, yPos);
	    }*/
	    
	    //Double-check angle
	    if (angle<0 || angle>360) {
	    	throw new RuntimeException("Angle must be in bounds.");
	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	ticks.add(t);
	    }
	  
	    //Create temp Busdriver
	    BusDriverTick tempBusDriver = new BusDriverTick(xPos, yPos, angle,numPassengers, msgLoc);
	    
	    //Check if the Busdriver is fake
	    if(props.containsKey("fake")){
	    	if(props.get("fake").equals("true")){
	    		tempBusDriver.setItFake();
	    	}
	    }
	    //Check if it's a "tracking" version of this car
	    boolean tracking = false;
	    if (props.containsKey("tracking")) {
	    	tracking = props.get("tracking").toLowerCase().equals("true");
	    }
	    //Check if the car has a length and width or not
	    if(props.containsKey("length") && props.containsKey("width")){
	    	tempBusDriver.setLenth(Integer.parseInt(props.get("length")));
	    	tempBusDriver.setWidth(Integer.parseInt(props.get("width")));
	    }
	    
	    tempBusDriver.setID(objID);
	    
	    //Add this agent to the proper frame. If it's a "tracking" item, add it a parallel 
	    // list which contains tracking Agents
	    TimeTick currTick = ticks.get(frameID);
	    if (tracking) {
	    	//For now, just reuse the "fake" prperty
	    	tempBusDriver.setItFake();
	    	currTick.trackingTicks.put(objID, tempBusDriver);
	    } else {
	    	currTick.agentTicks.put(objID, tempBusDriver);
	    }
	}
	
	private void parsePedestrian(int frameID, int objID, String rhs, RoadNetwork rn) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    //Integer.parseInt(props.get("pedSig")); //Currently not used
	    
	    //TEMP: Hack for out-of-bounds agents
	    /*if (OutOfBounds(xPos, yPos, rn)) {
	    	Utility.CheckBounds(xBounds, xPos);
	    	Utility.CheckBounds(yBounds, yPos);
	    }*/
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	ticks.add(t);
	    }
	    
	    //Create a temp pedestrian
	    PedestrianTick tempPedestrian = new PedestrianTick(xPos, yPos);
	    
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

