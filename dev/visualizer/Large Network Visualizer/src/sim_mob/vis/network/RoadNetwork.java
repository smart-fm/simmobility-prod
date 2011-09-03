package sim_mob.vis.network;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.regex.Matcher;

import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.util.Utility;

/**
 * The RoadNetwork is the top-level object containing all relevant details about our 
 * static RoadNetwork.
 */
public class RoadNetwork {
	private DPoint cornerTL;
	private DPoint cornerLR;
	
	private ArrayList<Node> nodes;
	
	public DPoint getTopLeft() { return cornerTL; }
	public DPoint getLowerRight() { return cornerLR; }
	public ArrayList<Node> getNodes() { return nodes; }
	
	
	/**
	 * Load the network from a filestream.
	 */
	public RoadNetwork(BufferedReader inFile) throws IOException {
		nodes = new ArrayList<Node>();
		
		//Also track min/max x/y pos
		double[] xBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		double[] yBounds = new double[]{Double.MAX_VALUE, Double.MIN_VALUE};
		
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
		    	dispatchConstructionRequest(type, frameID, objID, rhs, xBounds, yBounds);
		    } catch (IOException ex) {
		    	throw new IOException(ex.getMessage() + "\n...on line: " + line);
		    }
		}
		
		//Save bounds
		cornerTL = new DPoint(xBounds[0], yBounds[0]);
		cornerLR = new DPoint(xBounds[1], yBounds[1]);
		
		//Scale (to 95%, allows better viewing)
		//double fivePercW = (xBounds[1]-xBounds[0])*5 / 100;
		//double fivePercH = (yBounds[1]-yBounds[0])*5 / 100;
		
		//ScaledPoint.ScaleAllPoints(new DPoint(xBounds[0]-fivePercW, xBounds[1]+fivePercW), new DPoint(yBounds[0]-fivePercH, yBounds[1]+fivePercH));
	}
	
	
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, double[] xBounds, double[] yBounds) throws IOException {
		//Nodes are displayed the same
		if (objType.equals("multi-node") || objType.equals("uni-node")) {
			parseNode(frameID, objID, rhs, objType.equals("uni-node"), xBounds, yBounds);
		}
	}
	
	
	private Hashtable<String, String> parseRHS(String rhs, String[] ensure) throws IOException {
		//Json-esque matching
		Hashtable<String, String> properties = new Hashtable<String, String>();
		Matcher m = Utility.LOG_RHS_REGEX.matcher(rhs);
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
	
	
	
	private void parseNode(int frameID, int objID, String rhs, boolean isUni, double[] xBounds, double[] yBounds) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = parseRHS(rhs, new String[]{"xPos", "yPos"});
	    
	    //Now save the position information
	    double x = Double.parseDouble(props.get("xPos"));
	    double y = Double.parseDouble(props.get("yPos"));
	    
	    Utility.CheckBounds(xBounds, x);
	    Utility.CheckBounds(yBounds, y);
	    
	    nodes.add(new Node(x, y, isUni));
	}
	
	
}






