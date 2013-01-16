package sim_mob.vis.simultion;


import java.awt.Color;
import java.io.*;
import java.util.*;
import java.util.Map.Entry;
import javax.swing.SwingUtilities;

import sim_mob.act.BifurcatedActivity;
import sim_mob.act.SimpleThreadPool;
import sim_mob.vis.ProgressUpdateRunner;
import sim_mob.vis.controls.NetworkPanel;
import sim_mob.vis.network.*;
import sim_mob.vis.util.FastLineParser;
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
	
	//private static double[] xBounds;
	//private static double[] yBounds;
	
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
	
	public SimulationResults() {}
	
	
	
	public void loadFileAndReport(BufferedReader inFile, RoadNetwork rn, HashSet<Long> uniqueAgentIDs, long fileLength, NetworkPanel progressUpdate) throws IOException {
		ticks = new ArrayList<TimeTick>();
		frame_length_ms = -1;
		
		//Provide feedback to the user
		long totalBytesRead = 0;
		long lastKnownTotalBytesRead = 0;
		if (progressUpdate!=null) {
			SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, 0.0, false, new Color(0x00, 0x00, 0xFF), ""));
		}
		
		//TEMP: Hack for agents which are out of bounds
	//	xBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
	//	yBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		
		//Read
		String line;	
		ArrayList<LogFileLine> lineBuffer = new ArrayList<LogFileLine>();
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
				stp.newTask(new SimResLineParser(temp, this, rn, uniqueAgentIDs));
			}
		}
		
		
		//Any remaining lines?
		if (!lineBuffer.isEmpty()) {
			//Push to thread
			stp.newTask(new SimResLineParser(lineBuffer, this, rn, uniqueAgentIDs), true);
		}
		
		//Wait
		stp.joinAll();
		
		
		if (frame_length_ms==-1) {
			throw new RuntimeException("Error: missing \"simulation\" tag.");
		}
		
		//Modify traffic signal to make it stable
		Hashtable<Long,SignalLineTick> oldSignal = new Hashtable<Long, SignalLineTick>();
		for (TimeTick tt : ticks) {
			if(tt.signalLineTicks.size()>0) {
				//Clean previous data
				oldSignal = new Hashtable<Long, SignalLineTick>();
				
				//Assign new data
				oldSignal = tt.signalLineTicks;
			} else if(tt.signalLineTicks.size() == 0) {
				if(oldSignal.size()!=0){
					tt.signalLineTicks = oldSignal;
				} else {
					//System.out.println("Error, in modification of signal line ticks -- SimulationResults, constructor");
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
		HashSet<Long> uniqueAgentIDs;
		TemporarySimObjects resObj;
		RoadNetwork network;
		//TEMP
		FastLineParser flp;
		
		SimResLineParser(ArrayList<LogFileLine> lines, SimulationResults sim, RoadNetwork rn, HashSet<Long> uniqueAgentIDs) {
			this.lines = lines;
			this.sim = sim;
			this.uniqueAgentIDs = uniqueAgentIDs;
			this.resObj = new TemporarySimObjects();
			network = rn;
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
						if(tTick < 0) continue;//TODO find a way to get rid of this. time tick 0 i creating problem
						//Save this object for later.
						if (!resObj.gsonObjectsToAdd.containsKey(tTick)) {
							resObj.gsonObjectsToAdd.put(tTick, new ArrayList<GsonResObj>());
						}
						resObj.gsonObjectsToAdd.get(tTick).add(gRes);
					} else {
						//Parse this line as text and pseudo-json.
						Utility.ParseResults pRes = Utility.ParseLogLine(flp, logLine.line);
						if (pRes.isError()) {
							throw new RuntimeException("Error parsing line: \n  " + logLine.line + "\nError:\n"+pRes.errorMsg);
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
						
						gRes.addSelfToSimulation(network, sim);
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
						//ticks.get(sigTimeTick.getKey()).signalLineTicks.put(sigTick.getID(), sigTick);

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
			if (pRes.type.equals("Activity")) {
				parseActivity(pRes);
			} 
			else if (pRes.type.equals("Driver")) {
				parseDriver(pRes);
			} else if (pRes.type.equals("BusDriver")) {
				parseBusDriver(pRes);
			} else if (pRes.type.equals("Signal")) {
				parseSignalLines(pRes);
			} else if (pRes.type.equals("pedestrian")) {
				parsePedestrian(pRes);
				
			}
			else if (pRes.type.equals("passenger")) {
				parsePassenger(pRes);
			}
			else if (pRes.type.equals("simulation")) {
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
//		    DriverTick tempDriver = new DriverTick(pRes.objID, xPos, yPos, angle, msgLoc);
		    //demo
		    DriverTick tempDriver = new DriverTick(pRes.objID, xPos, yPos, angle, msgLoc,pRes.frame);
		    
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
		
		void parseActivity(Utility.ParseResults pRes) throws IOException {			
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"xPos", "yPos"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(pRes.properties.get("xPos"));
		    double yPos = Double.parseDouble(pRes.properties.get("yPos"));
		    
		    
		    //Create temp driver
		    ActivityTick tempActivity = new ActivityTick((int)pRes.objID, xPos, yPos);
		    
		    
		  //Add it to our temporary list
//		    System.out.println("Adding activity for frame " + pRes.frame);
		  saveTempAgent(pRes.frame, tempActivity, false);
		  
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
		
		
		void parsePedestrian(Utility.ParseResults pRes) throws IOException {
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"xPos", "yPos"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(pRes.properties.get("xPos"));
		    double yPos = Double.parseDouble(pRes.properties.get("yPos"));

		    
		    //Create a temp pedestrian
		    PedestrianTick tempPedestrian = new PedestrianTick(pRes.objID, xPos, yPos);
		    
		    //Check if the pedestrian is fake
		    if(pRes.properties.containsKey("fake")){
		    	if(pRes.properties.get("fake").equals("true")){
		    		tempPedestrian.setItFake();
		    	}
		    }

		    
		    //Add this agent to the proper frame.
		    saveTempAgent(pRes.frame, tempPedestrian, false);
		}
		
		void parsePassenger(Utility.ParseResults pRes) throws IOException {
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"xPos", "yPos"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    double xPos = Double.parseDouble(pRes.properties.get("xPos"));
		    double yPos = Double.parseDouble(pRes.properties.get("yPos"));

		    
		    //Create a temp passenger
		    PassengerTick tempPassenger = new PassengerTick(pRes.objID, xPos, yPos);
		    
		    //Check if the pedestrian is fake
		    if(pRes.properties.containsKey("fake")){
		    	if(pRes.properties.get("fake").equals("true")){
		    		tempPassenger.setItFake();
		    	}
		    }

		    
		    //Add this agent to the proper frame.
		    saveTempAgent(pRes.frame, tempPassenger, false);
		}
		
		void parseSignalLines(Utility.ParseResults pRes) throws IOException {
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"va", "vb", "vc", "vd", "pa", "pb", "pc", "pd"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information.  
		    ArrayList<ArrayList<Integer>> allVehicleLights = new ArrayList<ArrayList<Integer>>();
		    allVehicleLights.add(parseEachSignal(pRes.properties.get("va")));
		    allVehicleLights.add(parseEachSignal(pRes.properties.get("vb")));
		    allVehicleLights.add(parseEachSignal(pRes.properties.get("vc")));
		    allVehicleLights.add(parseEachSignal(pRes.properties.get("vd")));
		    
		    ArrayList<Integer> allPedestrainLights = new ArrayList<Integer>();
		    allPedestrainLights.add(Integer.parseInt(pRes.properties.get("pa")));
		    allPedestrainLights.add(Integer.parseInt(pRes.properties.get("pb")));
		    allPedestrainLights.add(Integer.parseInt(pRes.properties.get("pc")));
		    allPedestrainLights.add(Integer.parseInt(pRes.properties.get("pd")));
		    
		  
		    //Create a temp signal
		    SignalLineTick tempSignalLineTick = new SignalLineTick(pRes.objID, allVehicleLights, allPedestrainLights ,pRes.objID);

		    //Check if signal is fake
		    if(pRes.properties.containsKey("fake")){
		    	if(pRes.properties.get("fake").equals("true")){
		    		tempSignalLineTick.setItFake();
		    	}
		    }
		    
		    //Save it to add later
		    saveTempSignal(pRes.frame, tempSignalLineTick);

		}
		
		void parseSimulation(Utility.ParseResults pRes) throws IOException {
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"frame-time-ms"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Check
		    if (pRes.frame!=0) { throw new RuntimeException("Simulation block must have frame-id=0"); }
		    if (pRes.objID!=0) { throw new RuntimeException("Simulation block must have agent-id=0"); }
		    
		    //Now save the relevant information
		    resObj.tempFrameLenMS = Integer.parseInt(pRes.properties.get("frame-time-ms"));
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

