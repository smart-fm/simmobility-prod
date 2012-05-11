package sim_mob.vis.network;

import java.awt.geom.Point2D;
import java.io.*;
import java.util.*;
import java.util.regex.*;

import sim_mob.vis.FileOpenThread;
import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.network.basic.Vect;
import sim_mob.vis.util.Mapping;
import sim_mob.vis.util.Utility;

/**
 * The RoadNetwork is the top-level object containing all relevant details about  
 * static RoadNetwork.
 */
public class RoadNetwork {
	private DPoint cornerTL;
	private DPoint cornerLR;
	
	//private int canvasWidth;
	//private int canvasHeight;
	
	private Hashtable<Integer, Node> nodes;
	//private ArrayList<Node> localPoints;
	private Hashtable<Integer,Link> links;
	private Hashtable<Integer, RoadName> roadNames;
	private Hashtable<Integer, Segment> segments;
	private Hashtable<Integer, Crossing> crossings;
	
	//For signal
	private Hashtable<Integer, Hashtable<Integer,Lane> > lanes;
	private Hashtable<Integer, TrafficSignalLine> trafficSignalLines;
	private Hashtable<Integer, TrafficSignalCrossing> trafficSignalCrossings;
	private Hashtable<Integer, Intersection> intersections; 
	
	//Stores a reference to Lane Connectors on each segment...? Can we put this in
	//  the segment class?
	//private Hashtable<Segment, ArrayList<Integer>> segmentRefTable;
	
		
	public DPoint getTopLeft() { return cornerTL; }
	public DPoint getLowerRight() { return cornerLR; }

	public Hashtable<Integer, Node> getNodes(){ return nodes; }
	public Hashtable<Integer, Link> getLinks() { return links; }
	public Hashtable<Integer, RoadName> getRoadNames() { return roadNames; }
	public Hashtable<Integer, Segment> getSegments() { return segments; }
	public Hashtable<Integer, Crossing> getCrossing() { return crossings; }
	
	//For Signal
	public Hashtable<Integer, Hashtable<Integer,Lane> > getLanes(){return lanes;}
	public Hashtable<Integer, TrafficSignalLine> getTrafficSignalLine(){return trafficSignalLines;}
	public Hashtable<Integer, TrafficSignalCrossing> getTrafficSignalCrossing() {return trafficSignalCrossings;}
	public Hashtable<Integer, Intersection> getIntersection(){return intersections;}

	
	/**
	 * Load the network from a filestream.
	 */
	
	public RoadNetwork(BufferedReader inFile, long totalFileSize, FileOpenThread progressUpdater) throws IOException {

		//this.canvasWidth = canvasWidth;
		//this.canvasHeight = canvasHeight;
		
		nodes = new Hashtable<Integer, Node>();
		//localPoints = new ArrayList<Node>();
		
		links = new Hashtable<Integer, Link>();
		roadNames = new Hashtable<Integer, RoadName>();
		segments = new Hashtable<Integer, Segment>();
		//laneMarkings = new Hashtable<Integer,Hashtable<Integer,LaneMarking>>();
		crossings = new Hashtable<Integer,Crossing>();
	
		//laneConnectors = new Hashtable<Integer, LaneConnector>();
		trafficSignalLines = new Hashtable<Integer, TrafficSignalLine>(); 
		trafficSignalCrossings = new Hashtable<Integer, TrafficSignalCrossing>();
		intersections = new Hashtable<Integer, Intersection>();
		lanes = new Hashtable<Integer, Hashtable<Integer,Lane>>();

		//fromToSegmentRefTable =  new Hashtable<String, Integer>();
		//segmentRefTable = new  Hashtable<Integer , ArrayList<Integer>>(); 
		//segmentToLanesTable = new Hashtable<Integer,Hashtable<Integer,Integer>>();
		
		//Also track min/max x/y pos
		double[] xBounds = new double[]{java.lang.Double.MAX_VALUE, java.lang.Double.MIN_VALUE};
		double[] yBounds = new double[]{java.lang.Double.MAX_VALUE, java.lang.Double.MIN_VALUE};
		
		//Read
		String line;
		double totalBytesRead = 0;
		while ((line=inFile.readLine())!=null) {
			//Update bytes read
			totalBytesRead += line.length();
			
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
		    
		    //Update bytes parsed.
		    progressUpdater.setPercentDone(totalBytesRead / totalFileSize);
		}
		
		
		//Save bounds
		cornerTL = new DPoint(xBounds[0], yBounds[0]);
		cornerLR = new DPoint(xBounds[1], yBounds[1]);
	
		this.populateIntersections();

		//Convert points to local coordinate system
		//convertToLocal(this.canvasWidth, this.canvasHeight);
	}
	
			
	private void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs, double[] xBounds, double[] yBounds) throws IOException {
		//Nodes are displayed the same
		if (objType.equals("multi-node") || objType.equals("uni-node")) {
			parseNode(frameID, objID, rhs, objType.equals("uni-node"), xBounds, yBounds);
		} else if (objType.equals("link")) {
			parseLink(frameID, objID, rhs);
		} else if (objType.equals("road-segment")) {
			parseSegment(frameID, objID, rhs);
		} else if (objType.equals("lane")){
			parseLineMarking(frameID, objID, rhs);
		} else if(objType.equals("crossing")){
			parseCrossing(frameID, objID, rhs);
		} else if(objType.equals("lane-connector")){
			parseLaneConnector(frameID, objID, rhs);
		} else if(objType.equals("Signal-location")){
			parseSignalLocation(frameID, objID, rhs);
		}

		
	}
		
	private void parseLink(int frameID, int objID, String rhs) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"road-name", "start-node", "end-node", "fwd-path", "rev-path"});
	    
	    //Now save the relevant information
	    String name = props.get("road-name");
	    int startNodeKEY = Utility.ParseIntOptionalHex(props.get("start-node"));
	    int endNodeKEY = Utility.ParseIntOptionalHex(props.get("end-node"));
	    
	    Node startNode = nodes.get(startNodeKEY);
	    Node endNode = nodes.get(endNodeKEY);
	    
	    //Ensure nodes exist
	    if (startNode==null) { throw new IOException("Unknown node id: " + Integer.toHexString(startNodeKEY)); }
	    if (endNode==null) { throw new IOException("Unknown node id: " + Integer.toHexString(endNodeKEY)); }
	    

	    Link toAdd = new Link(name, startNode, endNode);
	    toAdd.setFwdPathSegmentIDs(Utility.ParseLinkPaths(props.get("fwd-path")));
	    toAdd.setRevPathSegmentIDs(Utility.ParseLinkPaths(props.get("rev-path")));
	    
	    links.put(objID, toAdd);
	    roadNames.put(objID, new RoadName(name, startNode, endNode));
	
	}
	
	
	//Create a "simple lane" line (which contains only the start and end points of a lane line)
	//  which is midway between an outer and an inner Lane Marking.
	private static final Lane getSimpleMidlane(int laneID, LaneMarking innerLine, LaneMarking outerLine) {
		Vect startPoint = new Vect(innerLine.getPoint(0), outerLine.getPoint(0));
		startPoint.scaleVect(startPoint.getMagnitude()/2.0);
		startPoint.translateVect();
		Vect endPoint = new Vect(innerLine.getPoint(-1), outerLine.getPoint(-1));
		endPoint.scaleVect(endPoint.getMagnitude()/2.0);
		endPoint.translateVect();
		
		return new Lane(laneID, new Point2D.Double(startPoint.getX(), startPoint.getY()), new Point2D.Double(endPoint.getX(), endPoint.getY()));
	}
	
	
	private void parseLineMarking(int frameID, int objID, String rhs) throws IOException {	 
		//Check frameID
	    if (frameID!=0) { throw new IOException("Unexpected frame ID, should be zero"); }
	    
	    //Check and parse properties. for lanes, it checks only parent-segment only as the number of lanes is not fixed
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"parent-segment"});
	    
	    Segment parent = null;
	    {
	      int pSegID = Utility.ParseIntOptionalHex(props.get("parent-segment"));
	      parent = segments.get(pSegID);
	      if (parent==null) { throw new RuntimeException("Lane line references invalid parent-segment: " + pSegID); }
	    }
	    
	    Enumeration<String> keys = props.keys();	   
	    
	    //Hashtable<Integer,LaneMarking> tempVirtualLaneTable = new Hashtable<Integer, LaneMarking>();
	    ArrayList<Integer> lineNumbers = new ArrayList<Integer>();
	    Hashtable<Integer, ArrayList<Integer>> lineMarkingPositions = new Hashtable<Integer, ArrayList<Integer>>();
	    
	    ArrayList<Integer> sidewalkLaneLines = new ArrayList<Integer>();
	    while(keys.hasMoreElements()){		    
	    	String key = keys.nextElement().toString();
	    	if(key.contains("parent-segment")){ continue; } //Already parsed it.
	    	
	    	//Check whether the lane is a sidewalk
	    	Matcher mS = Utility.LANE_SIDEWALK_REGEX.matcher(key);
	    	Matcher mL = Utility.LANE_LINE_REGEX.matcher(key);
	    	int lineNumber = -1;
	    	if (mS.matches()) {
	    		//Sidewalks actually cover both the current lane line and the next.
	    		lineNumber = Integer.parseInt(mS.group(1));
	    		sidewalkLaneLines.add(lineNumber);
	    		sidewalkLaneLines.add(lineNumber+1);
	    	} else if (mL.matches()) {
	    		lineNumber = Integer.parseInt(mL.group(1));
	    		
	    		//Extract Node information
	    		ArrayList<Integer> pos = Utility.ParseLaneNodePos(props.get(key));
	    		
	    		//Add the lane to this segment.
	    		LaneMarking marking = new LaneMarking(pos,false,lineNumber,parent.getSegmentID());
		    	parent.addLaneEdge(lineNumber, marking);
	    
	    		//Add lane number to the tracking list
		    	if(lineNumber != -1) {
		    		lineNumbers.add(lineNumber);
		    		lineMarkingPositions.put(lineNumber, pos);
		    	}
	    	} else {
	    		throw new RuntimeException("Badly-formatted lane line: " + key); 
	    	}	   
	    } 
	    
	    //Find the sidewalk line(s) and mark them.
	    int numEdges = parent.getNumLaneEdges();
	    for (int swLaneLine : sidewalkLaneLines) {
	    	if (swLaneLine<numEdges) {
	    		parent.getLaneEdge(swLaneLine).setSideWalk(true);
	    	}
	    }
	    
	    
	    //Create "Lanes" for each midline (start/end points only) between each lane edge.
	    Hashtable<Integer, Lane> tempLaneTable = new Hashtable<Integer, Lane>();
	    for (int laneID=0; laneID<parent.getNumLaneEdges()-1; laneID++) {
	    	Lane simpleLane = getSimpleMidlane(laneID, parent.getLaneEdge(laneID), parent.getLaneEdge(laneID+1));
	    	tempLaneTable.put(laneID, simpleLane);

	    	//For some reason we maintain a lookup? 
	    	// (NOTE: This should already be part of the segment class.)
	    	/*if(segmentToLanesTable.containsKey(parent.getSegmentID())) {
	    		segmentToLanesTable.get(parent.getSegmentID()).put(laneID, objID);
	    	} else {
	    		Hashtable<Integer, Integer> lanesOnSegment = new Hashtable<Integer,Integer>();
	    		lanesOnSegment.put(i, objID);
	    		segmentToLanesTable.put(parent.getSegmentID(), lanesOnSegment);
	    	}*/
	    }
	    
	    


	    
	    
	    //For usage of signal
	    lanes.put(parent.getSegmentID(), tempLaneTable);	 

	    //Create a new Lane, save it
	    //laneMarkings.put(objID, tempVirtualLaneTable);
	    
	    
	    //Segments should only be specified once
	    parent.sealLaneEdges();
	}
	
	private void parseSegment(int frameID, int objID, String rhs) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"parent-link", "max-speed", "lanes", "from-node", "to-node"});
	    
	    //Now save the relevant information
	    int parentLinkID = Utility.ParseIntOptionalHex(props.get("parent-link"));
	    Link parent = links.get(parentLinkID);
	    int fromNodeID = Utility.ParseIntOptionalHex(props.get("from-node"));
	    int toNodeID = Utility.ParseIntOptionalHex(props.get("to-node"));
	    Node fromNode = nodes.get(fromNodeID);
	    Node toNode = nodes.get(toNodeID);
	    
	    //Ensure nodes exist
	    if (parent==null) { throw new IOException("Unknown Link id: " + Integer.toHexString(parentLinkID)); }
	    if (fromNode==null) { throw new IOException("Unknown node id: " + Integer.toHexString(fromNodeID)); }
	    if (toNode==null) { throw new IOException("Unknown node id: " + Integer.toHexString(toNodeID)); }
	    
	    //Create a new Link, save it
	    //segments.put(objID, new Segment(parent, fromNode, toNode, parentLinkID));
	    segments.put(objID, new Segment(parent, fromNode, toNode, parentLinkID, objID));
	    
	}
	
	private void parseLaneConnector(int frameID, int objID, String rhs) throws IOException{

		//Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
		//Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"from-segment", "from-lane","to-segment","to-lane"});

	    
	    //Now save the relevant information
	    int fromSegmentKEY = Utility.ParseIntOptionalHex(props.get("from-segment"));
	    int toSegmentKEY = Utility.ParseIntOptionalHex(props.get("to-segment"));
	    int fromLane = Utility.ParseIntOptionalHex(props.get("from-lane"));
	    int toLane = Utility.ParseIntOptionalHex(props.get("to-lane"));
	    Segment fromSegment = segments.get(fromSegmentKEY);
	    Segment toSegment = segments.get(toSegmentKEY);

	    //Ensure segment exist
	    if (fromSegment==null) {
	    	throw new IOException("Unknown Segment id: " + Integer.toHexString(fromSegmentKEY));
	    }
	    if (toSegment==null) {
	    	throw new IOException("Unknown Segment id: " + Integer.toHexString(toSegmentKEY));
	    }
	    
	    //Put into lane connector table
	    LaneConnector tempLaneConnector = new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane);
	    collectSignalLineInfo(objID,tempLaneConnector);
	    
	    //Use from-segment and to-segment form a reference table, to check from-segment & to-segment pair against lane connector id
	    fromSegment.setLaneConnector(fromLane, objID, toSegment);
	    
	    /*if(segmentRefTable.containsKey(fromSegmentKEY)){
	    	segmentRefTable.get(fromSegmentKEY).add(objID);	
	    	segmentRefTable.get(fromSegmentKEY).add(toSegmentKEY);
	    } else{
	    	ArrayList<Integer> toSegmentList = new ArrayList<Integer>();
	    	toSegmentList.add(objID);
	    	toSegmentList.add(toSegmentKEY);
	    	segmentRefTable.put(fromSegmentKEY, toSegmentList);
	    }*/
	    
	    
	}

	private void parseNode(int frameID, int objID, String rhs, boolean isUni, double[] xBounds, double[] yBounds) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
	    
	    //Now save the position information
	    double x = java.lang.Double.parseDouble(props.get("xPos"));
	    double y = java.lang.Double.parseDouble(props.get("yPos"));
	    
	    Utility.CheckBounds(xBounds, x);
	    Utility.CheckBounds(yBounds, y);


	    Node tempVirtualNode = new Node(x, y, isUni,objID);
	    nodes.put(objID, tempVirtualNode);
	    
	    //Testing
	    //localPoints.add(tempVirtualNode);
	    
	}
		
	private void parseCrossing(int frameID, int objID, String rhs) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"near-1", "near-2", "far-1", "far-2"});
	    
	    //Now save the relevant information
	    Node nearOneNode = Utility.ParseCrossingNodePos(props.get("near-1"));
	    Node nearTwoNode = Utility.ParseCrossingNodePos(props.get("near-2"));
	    Node farOneNode = Utility.ParseCrossingNodePos(props.get("far-1"));
	    Node farTwoNode = Utility.ParseCrossingNodePos(props.get("far-2"));
	   
	    //localPoints.add(nearOneNode);
	    //localPoints.add(nearTwoNode);
	    //localPoints.add(farOneNode);
	    //localPoints.add(farTwoNode);
	    
	    //Create a new Crossing, save it
	    crossings.put(objID, new Crossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,objID));
	    trafficSignalCrossings.put(objID, new TrafficSignalCrossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,objID));

	}

	private void parseSignalLocation(int frameID, int objID, String rhs) throws IOException{
		
		//Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
		//Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"node","va","aa","pa","vb","ab","pb","vc","ac","pc","vd","ad","pd"});

	    //Now save the relevant information
	    int intersectionNodeID = Utility.ParseIntOptionalHex(props.get("node"));
	    int linkVaID = Utility.ParseIntOptionalHex(props.get("va"));
	    int linkVbID = Utility.ParseIntOptionalHex(props.get("vb"));
	    int linkVcID = Utility.ParseIntOptionalHex(props.get("vc"));
	    int linkVdID = Utility.ParseIntOptionalHex(props.get("vd"));
	    int linkPaID = Utility.ParseIntOptionalHex(props.get("pa"));
	    int linkPbID = Utility.ParseIntOptionalHex(props.get("pb"));
	    int linkPcID = Utility.ParseIntOptionalHex(props.get("pc"));
	    int linkPdID = Utility.ParseIntOptionalHex(props.get("pd"));

	    ArrayList <Integer>  tempLinkIDs = new ArrayList<Integer>(
	    			Arrays.asList(linkVaID, linkVbID, linkVcID, linkVdID)); 
	    
	    ArrayList <Integer> tempCrossingIDs = new ArrayList<Integer>(
	    			Arrays.asList(linkPaID,linkPbID,linkPcID,linkPdID));
	    
	    intersections.put(objID, new Intersection(intersectionNodeID,tempLinkIDs, tempCrossingIDs));		
	
	}

	private void collectSignalLineInfo(int objID, LaneConnector laneConnector){				
		if(lanes.containsKey(laneConnector.getFromSegment()) && lanes.containsKey(laneConnector.getToSegment()) ){
	
			int fromLaneNo = laneConnector.getFromLane();
			int toLaneNo = laneConnector.getToLane();
			Lane fromLane = lanes.get(laneConnector.getFromSegment()).get(fromLaneNo);
			Lane toLane = lanes.get(laneConnector.getToSegment()).get(toLaneNo);
			TrafficSignalLine tempSignalLine = new TrafficSignalLine(fromLane, toLane); 
			trafficSignalLines.put(objID, tempSignalLine);	
			
		} else{
			System.out.println("Error, No such segment -- RoadNetwork, collectSignalInfo()");
		}
	}
	
	
	private void populateIntersection(int intersectionNodeID, char charKey, int sigLinkID, int[] fromSegmentList, int[] toSegmentList) {
		//Build up a list of segment IDs which also occur in this parentLink.
		ArrayList<Integer> tempSegmentIDs = new ArrayList<Integer>();
		for (Integer segmentID : segments.keySet()) {
			Segment tempSegment = segments.get(segmentID);
			int parentLinkID = tempSegment.getparentLinkID();
			if(sigLinkID == parentLinkID ){
				tempSegmentIDs.add(segmentID);
			}
		}
		
		//Find segment that related to intersection node
		for (int segID : tempSegmentIDs) {
			if(segments.containsKey(segID)){
				Segment tempSegment = segments.get(segID);
				
				//Convert 'a','b',... to 0,1,...
				int charKeyVal = ((int)charKey) - ((int)'a');
				if (charKey<'a' || charKey>'d') { throw new RuntimeException("Invalid character key: " + charKey); }
				
				//Check if the segment is come from the intersection node
				if(tempSegment.getTo().getID() == intersectionNodeID) {
					fromSegmentList[charKeyVal] = segID;
				} else if(tempSegment.getFrom().getID() == intersectionNodeID){
					toSegmentList[charKeyVal] = segID;	
				}
			} else {
				System.out.println("Error, no such segments in segment table "+Integer.toHexString(segID)+" -- RoadNetwork,populateIntersection ");
			}
		}	
	}
	
	
	private void populateIntersections(){
		for(Intersection intersection : intersections.values()){		
			int[] fromSegmentList = new int[]{-1,-1,-1,-1};
			int[] toSegmentList = new int[]{-1,-1,-1,-1}; 
						
			//Search all the Links
			Hashtable<Character, Integer> sigLinkIDs = intersection.getSigalLinkIDs();
			for (char charKey : sigLinkIDs.keySet()) {
				populateIntersection(intersection.getIntersectNodeID(), charKey, sigLinkIDs.get(charKey), fromSegmentList, toSegmentList);
			}
							
			ArrayList<DirectionHelper> signalList = helperAllocateDirection(fromSegmentList,toSegmentList);			
			//for (Integer linkNumber : signalList.keySet()) {
			for (DirectionHelper dirHelp : signalList) {
				//ArrayList<ArrayList<TrafficSignalLine>> signalListPerLink = signalList.get(linkNumber);
				
				if(dirHelp.linkNumber == 0) {
					intersection.setVaTrafficSignal(dirHelp);
				} else if (dirHelp.linkNumber == 1) {
					intersection.setVbTrafficSignal(dirHelp);
				} else if (dirHelp.linkNumber == 2) {
					intersection.setVcTrafficSignal(dirHelp);
				} else if (dirHelp.linkNumber == 3) {
					intersection.setVdTrafficSignal(dirHelp);
				}
			
			}
			
			//Fill crossing signals
			ArrayList<Integer> crossingIDs = intersection.getSigalCrossingIDs();
			ArrayList<TrafficSignalCrossing> crossingSignals =  new ArrayList<TrafficSignalCrossing>();
			int linkPaID = crossingIDs.get(0);	
			int linkPbID = crossingIDs.get(1);
			int linkPcID = crossingIDs.get(2);
			int linkPdID = crossingIDs.get(3);
			
			//System.out.println("Intersection ID: " + Integer.toHexString(intersection.getIntersectNodeID()));
			//System.out.println("linkPaID: "+ linkPaID +" linkPbID: "+ linkPbID + " linkPcID: "+ linkPcID +" linkPdID: "+ linkPdID);
					
			if(trafficSignalCrossings.containsKey(linkPaID) && trafficSignalCrossings.containsKey(linkPbID) 
		    		&& trafficSignalCrossings.containsKey(linkPcID) && trafficSignalCrossings.containsKey(linkPdID)){	    	
		    	
				crossingSignals.add(trafficSignalCrossings.get(linkPaID));
				crossingSignals.add(trafficSignalCrossings.get(linkPbID));
				crossingSignals.add(trafficSignalCrossings.get(linkPcID));
				crossingSignals.add(trafficSignalCrossings.get(linkPdID));
				
				intersection.setSignalCrossing(crossingSignals);
				
				
		    }else{
		    	System.out.println("Error, no such pedestrain crossings -- RoadNetwork, parseSignalLocation");
		    }
			
			
		} // End for loop
		
	}
	
	
	public static class DirectionHelper {
		public int linkNumber;
		
		public ArrayList<TrafficSignalLine> leftTurnConnectors = new ArrayList<TrafficSignalLine>();
		public ArrayList<TrafficSignalLine> rightTurnConnectors = new ArrayList<TrafficSignalLine>();
		public ArrayList<TrafficSignalLine> straightConnectors = new ArrayList<TrafficSignalLine>();
	}
	
	
	private ArrayList<DirectionHelper> helperAllocateDirection(int[] fromSegmentList, int [] toSegmentList){
		ArrayList<DirectionHelper> res = new ArrayList<DirectionHelper>();		
		
		for(int fromSegmentKey : fromSegmentList) {
			//Prepre a new result item; add it.
			DirectionHelper currRes = new DirectionHelper();
			res.add(currRes);
			
			//Set its link number:
			currRes.linkNumber = res.size()-1;

			//Set relevant segment data (if possible)
			if (segments.containsKey(fromSegmentKey)) {
				ArrayList<Segment.LC> tempToSegmentList = segments.get(fromSegmentKey).getAllLaneConnectors();

				//TODO: We don't need a hash table for this, but I want to make sure I match the original algorithm
				//      exactly. Hopefully we remove this needless complexity later.
				Hashtable<Integer, Hashtable<Integer, Character>> sideLookup = new Hashtable<Integer, Hashtable<Integer,Character>>();
				for (int i=0; i<tempToSegmentList.size(); i++) {
					sideLookup.put(i, new Hashtable<Integer, Character>());
				}
				sideLookup.get(0).put(-3, 'L');
				sideLookup.get(0).put(-2, 'S');
				sideLookup.get(0).put(-1, 'R');
				sideLookup.get(1).put(1,  'L');
				sideLookup.get(1).put(-2, 'S');
				sideLookup.get(1).put(-1, 'R');
				sideLookup.get(2).put(1,  'L');
				sideLookup.get(2).put(2,  'S');
				sideLookup.get(2).put(-1, 'R');
				sideLookup.get(3).put(1,  'L');
				sideLookup.get(3).put(2,  'S');
				sideLookup.get(3).put(3,  'R');
				
	
				//TODO: This loop is very messy; there's lots of repeated information and copied code. 
				//      Clean it up using temporary classes (and better math) as needed. ~Seth
				//for(int j=1;j<tempToSegmentList.size();j+=2){
				for (Segment.LC connect : tempToSegmentList) {
					for(int k = 0; k < toSegmentList.length;k++){
						//No U-turns.
						if (currRes.linkNumber == k) {
							continue;
						}
						
						//Determine our direction.
						int direction = currRes.linkNumber - k;
						char actDir = sideLookup.get(currRes.linkNumber).get(direction);
						ArrayList<TrafficSignalLine> currTurn = currRes.straightConnectors;
						if (actDir=='L') {
							currTurn = currRes.leftTurnConnectors;
						} else if (actDir=='R') {
							currTurn = currRes.rightTurnConnectors;
						} else if (actDir != 'S') {
							throw new RuntimeException("Unexpected direction: " + actDir);
						}
						
						//Add this if the "to" field matches.
						if (connect.toSegment.getSegmentID() == toSegmentList[k]) {
							TrafficSignalLine tempSignal = trafficSignalLines.get(connect.lcID);
							currTurn.add(tempSignal);
						}
					} // End k loop
				} 
			
			}
		}

		return res;
	}
}






