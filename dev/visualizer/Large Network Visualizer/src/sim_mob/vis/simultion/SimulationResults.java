package sim_mob.vis.simultion;


import java.awt.Color;
import java.io.*;
import java.util.*;
import java.util.Map.Entry;
import java.util.regex.Matcher;

import javax.lang.model.element.Element;
import javax.swing.SwingUtilities;

import sim_mob.act.BifurcatedActivity;
import sim_mob.act.SimpleThreadPool;
import sim_mob.vis.ProgressUpdateRunner;
import sim_mob.vis.controls.NetworkPanel;
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
	private static final int LINE_BUFFER_LIMIT = 50; //X lines per thread.
	
	public ArrayList<TimeTick> ticks;
	
	private static double[] xBounds;
	private static double[] yBounds;
	
	public int frame_length_ms;
	
	/*private static boolean OutOfBounds(double x, double y, RoadNetwork rn) {
		return     (x < rn.getTopLeft().x) || (x > rn.getLowerRight().x)
				|| (y < rn.getTopLeft().y) || (y > rn.getLowerRight().y);
	}*/
	
	public SimulationResults() {}
	
	
	
	public void loadFileAndReport(BufferedReader inFile, RoadNetwork rn, HashSet<Integer> uniqueAgentIDs, long fileLength, NetworkPanel progressUpdate) throws IOException {
		ticks = new ArrayList<TimeTick>();
		frame_length_ms = -1;
		
		//Provide feedback to the user
		long totalBytesRead = 0;
		long lastKnownTotalBytesRead = 0;
		if (progressUpdate!=null) {
			SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, 0.0, false, new Color(0x00, 0x00, 0xFF), ""));
		}
		
		//TEMP: Hack for agents which are out of bounds
		xBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		yBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		
		//Read
		String line;
		ArrayList<String> lineBuffer = new ArrayList<String>();
		SimpleThreadPool stp = new SimpleThreadPool(10); //No more than 10 threads at once.
		while ((line=inFile.readLine())!=null) {
			//Update
			totalBytesRead += line.length();
			boolean pushUpdate = (totalBytesRead - lastKnownTotalBytesRead) > 1024;
			
			//Send a message
			if (pushUpdate && progressUpdate!=null) {
				lastKnownTotalBytesRead = totalBytesRead;
				if (fileLength>0) {
					SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, totalBytesRead/((double)fileLength), true, new Color(0x00, 0x00, 0xFF), ""));
				} else {
					SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, totalBytesRead, false, new Color(0x00, 0x00, 0xFF), ""));
				}
			}
			
			//Comment?
			line = line.trim();
			if (line.isEmpty() || !line.startsWith("(") || !line.endsWith(")")) {
				continue;
			}
			
			//Add to array
			lineBuffer.add(line);
			if (lineBuffer.size()>LINE_BUFFER_LIMIT) {
				//Push to thread, clear buffer.
				ArrayList<String> temp = lineBuffer;
				lineBuffer = new ArrayList<String>(); //Can't "clear", because we keep a reference.
				stp.newTask(new SimResLineParser(temp, this, uniqueAgentIDs));
			}
		}
		
		
		//Any remaining lines?
		if (!lineBuffer.isEmpty()) {
			//Push to thread
			stp.newTask(new SimResLineParser(lineBuffer, this, uniqueAgentIDs), true);
		}
		
		//Wait
		stp.joinAll();
		
		
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
		int tempFrameLenMS = -1;
	}
	
	
	
	//Begin = parse all objects (no sync.)
	//End = save all objects (sync)
	private static class SimResLineParser extends BifurcatedActivity {
		ArrayList<String> lines;
		SimulationResults sim;
		HashSet<Integer> uniqueAgentIDs;
		TemporarySimObjects resObj;
		
		SimResLineParser(ArrayList<String> lines, SimulationResults sim, HashSet<Integer> uniqueAgentIDs) {
			this.lines = lines;
			this.sim = sim;
			this.uniqueAgentIDs = uniqueAgentIDs;
			this.resObj = new TemporarySimObjects();
		}
		
		public Object begin(Object... args) {
			try {
				for (String line : lines) {
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
				    	dispatchConstructionRequest(type, frameID, objID, rhs);
				    } catch (IOException ex) {
				    	throw new IOException(ex.getMessage() + "\n...on line: " + line);
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
		boolean dispatchConstructionRequest(String objType, int frameID, int objID, String rhs) throws IOException {
			if (objType.equals("Driver")) {
				parseDriver(frameID, objID, rhs);
			} else if (objType.equals("BusDriver")) {
				parseBusDriver(frameID, objID, rhs);
			} else if (objType.equals("Signal")) {
				parseSignalLines(frameID, objID, rhs);
			} else if (objType.equals("pedestrian")) {
				parsePedestrian(frameID, objID, rhs);
			} else if (objType.equals("simulation")) {
				parseSimulation(frameID, objID, rhs);
			} else {
				return false; //Done
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
		
		
		void parseDriver(int frameID, int objID, String rhs) throws IOException {
		    //Check and parse properties.
		    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle"});
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(props.get("xPos"));
		    double yPos = Double.parseDouble(props.get("yPos"));
		    double angle = Double.parseDouble(props.get("angle"));
		    
		    //See if we have a message icon to show
		    DriverTick.RxLocation msgLoc = null;
		    if (props.containsKey("rxLong") && props.containsKey("rxLat")) {
		    	msgLoc = new DriverTick.RxLocation();
		    	msgLoc.longitude = Double.parseDouble(props.get("rxLong"));
		    	msgLoc.latitude = Double.parseDouble(props.get("rxLat"));
		    }
		    
		    //Double-check angle
		    if (angle<0 || angle>360) {
		    	throw new RuntimeException("Angle must be in bounds.");
		    }
		  
		    //Create temp driver
		    DriverTick tempDriver = new DriverTick(objID, xPos, yPos, angle, msgLoc);
		    
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
		    
		    
		    if (tracking) {
		    	//For now, just reuse the "fake" prperty
		    	tempDriver.setItFake();
		    }
		    
		    
		  //Add it to our temporary list
		  saveTempAgent(frameID, tempDriver, tracking);
		}
		
		
		//TODO: This shares a lot of functionality with parseDriver(). Can we merge some of it?
		void parseBusDriver(int frameID, int objID, String rhs) throws IOException {
		    //Check and parse properties.
		    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle", "passengers"});
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(props.get("xPos"));
		    double yPos = Double.parseDouble(props.get("yPos"));
		    double angle = Double.parseDouble(props.get("angle"));
		    int numPassengers = Integer.parseInt(props.get("passengers"));
		  
		    //Create temp driver
		    BusDriverTick tempBusDriver = new BusDriverTick(objID, xPos, yPos, angle, numPassengers);
		    
		    //Check if the driver is fake
		    if(props.containsKey("fake")){
		    	if(props.get("fake").equals("true")){
		    		tempBusDriver.setItFake();
		    	}
		    }
		    //Check if the car has a length and width or not
		    if(props.containsKey("length") && props.containsKey("width")){
		    	tempBusDriver.setLenth(Integer.parseInt(props.get("length")));
		    	tempBusDriver.setWidth(Integer.parseInt(props.get("width")));
		    }
		    tempBusDriver.setID(objID);

		    //Add it to our temporary list
		    saveTempAgent(frameID, tempBusDriver, false);
		}
		
		
		void parsePedestrian(int frameID, int objID, String rhs) throws IOException {
		    //Check and parse properties.
		    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(props.get("xPos"));
		    double yPos = Double.parseDouble(props.get("yPos"));

		    
		    //Create a temp pedestrian
		    PedestrianTick tempPedestrian = new PedestrianTick(objID, xPos, yPos);
		    
		    //Check if the pedestrian is fake
		    if(props.containsKey("fake")){
		    	if(props.get("fake").equals("true")){
		    		tempPedestrian.setItFake();
		    	}
		    }
		  	
		    //Set ID
		    tempPedestrian.setID(objID);
		    
		    //Add this agent to the proper frame.
		    saveTempAgent(frameID, tempPedestrian, false);
		}
		
		
		void parseSignalLines(int frameID, int objID, String rhs) throws IOException{
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
		    
		  
		    //Create a temp signal
		    SignalLineTick tempSignalLineTick = new SignalLineTick(objID, allVehicleLights, allPedestrainLights ,objID);

		    //Check if signal is fake
		    if(props.containsKey("fake")){
		    	if(props.get("fake").equals("true")){
		    		tempSignalLineTick.setItFake();
		    	}
		    }
		    
		    //Save it to add later
		    saveTempSignal(frameID, tempSignalLineTick);

		}
		
		void parseSimulation(int frameID, int objID, String rhs) throws IOException {
		    //Check and parse properties.
		    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"frame-time-ms"});
		    
		    //Check
		    if (frameID!=0) { throw new RuntimeException("Simulation block must have frame-id=0"); }
		    if (objID!=0) { throw new RuntimeException("Simulation block must have agent-id=0"); }
		    
		    //Now save the relevant information
		    resObj.tempFrameLenMS = Integer.parseInt(props.get("frame-time-ms"));
		}
	}
	
	
	public void reserveTimeTick(int frameID) {
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	ticks.add(t);
	    }
	}
	
	public void addAgent(int frameID, AgentTick agTick, boolean isReal) {
		TimeTick t = ticks.get(frameID);
		if (isReal) {
			t.agentTicks.put(agTick.getID(), agTick);
		} else {
			t.trackingTicks.put(agTick.getID(), agTick);
		}
	}
	
	public void addSignal(int frameID, SignalLineTick sigTick) {
		ticks.get(frameID).signalLineTicks.put(sigTick.getID(), sigTick);
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

