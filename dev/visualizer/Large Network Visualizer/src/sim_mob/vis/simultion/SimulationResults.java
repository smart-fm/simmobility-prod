package sim_mob.vis.simultion;


import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;

import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Contains everything needed to display the results of a simulation.
 */
public class SimulationResults {
	public ArrayList<TimeTick> ticks;
	
	private static double[] xBounds;
	private static double[] yBounds;
	
	public SimulationResults(BufferedReader inFile, RoadNetwork rn) throws IOException {
		ticks = new ArrayList<TimeTick>();
		
		//TEMP: Hack for drivers
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
		    	dispatchConstructionRequest(type, frameID, objID, rhs);
		    } catch (IOException ex) {
		    	throw new IOException(ex.getMessage() + "\n...on line: " + line);
		    }
		}
		
		//Now that the file has been loaded, scale driver positions to the RoadNetwork (so we can at least
		//  see something.)
		for (TimeTick tt : ticks) {
			for (AgentTick at : tt.agentTicks.values()) {
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
				at.pos = new ScaledPoint(resX, resY);
			}
		}
	}
	
	//We assume the x/y bounds will be within those saved by the RoadNetwork.
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs) throws IOException {
		if (objType.equals("Driver")) {
			parseDriver(frameID, objID, rhs);
		} else if (objType.equals("Signal")) {
			//parseSignal(frameID, objID, rhs);
		} else if (objType.equals("pedestrian")) {
			parsePedestrian(frameID, objID, rhs);
		}
	}
	
	
	private void parseDriver(int frameID, int objID, String rhs) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "angle"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    int angle = Integer.parseInt(props.get("angle"));
	    
	    //TEMP: Hack for drivers
	    Utility.CheckBounds(xBounds, xPos);
	    Utility.CheckBounds(yBounds, yPos);
	    
	    //Double-check angle
	    if (angle<0 || angle>360) {
	    	throw new IOException("Bad angle: " + angle + " for driver: " + objID);
	    }
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	ticks.add(t);
	    }
	    
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, new DriverTick(xPos, yPos, angle));
	}
	
	private void parsePedestrian(int frameID, int objID, String rhs) throws IOException {
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos", "pedSig"});
	    
	    //Now save the relevant information
	    double xPos = Double.parseDouble(props.get("xPos"));
	    double yPos = Double.parseDouble(props.get("yPos"));
	    Integer.parseInt(props.get("pedSig")); //Currently not used
	    
	    //Ensure the frame has been created
	    while (ticks.size()<=frameID) {
	    	TimeTick t = new TimeTick();
	    	t.agentTicks = new Hashtable<Integer, AgentTick>();
	    	ticks.add(t);
	    }
	    
	    //Add this agent to the proper frame.
	    ticks.get(frameID).agentTicks.put(objID, new PedestrianTick(xPos, yPos));
	}
}

