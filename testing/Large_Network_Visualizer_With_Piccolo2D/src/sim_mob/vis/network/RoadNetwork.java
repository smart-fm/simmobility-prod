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
	
	//NOTE: I'm disabling these for now; you can access Lane Markings from their parent segment. ~Seth
	//private Hashtable<Integer,Hashtable<Integer,LaneMarking>> laneMarkings;
	//public Hashtable<Integer,Hashtable<Integer,LaneMarking>> getLaneMarkings() { return laneMarkings; }
	
	//For signal
	private Hashtable<Integer, LaneConnector> laneConnectors;
	private Hashtable<Integer, Hashtable<Integer,Lane> > lanes;
	private Hashtable<Integer, TrafficSignalLine> trafficSignalLines;
	private Hashtable<Integer, TrafficSignalCrossing> trafficSignalCrossings;
	private Hashtable<Integer, Intersection> intersections; 
	
	private Hashtable<String, Integer> fromToSegmentRefTable;
	private Hashtable<Integer,ArrayList<Integer>> segmentRefTable;
	//                segID              lane#   laneID
	//private Hashtable<Integer,Hashtable<Integer,Integer>> segmentToLanesTable;
	
		
	public DPoint getTopLeft() { return cornerTL; }
	public DPoint getLowerRight() { return cornerLR; }
	//public int getCanvasWidth() { return canvasWidth; }
	//public int getCanvasHeight() { return canvasHeight; }

	public Hashtable<Integer, Node> getNodes(){ return nodes; }
	//public ArrayList<Node> getLocalPosPoints(){return localPoints;}
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
	
		laneConnectors = new Hashtable<Integer, LaneConnector>();
		trafficSignalLines = new Hashtable<Integer, TrafficSignalLine>(); 
		trafficSignalCrossings = new Hashtable<Integer, TrafficSignalCrossing>();
		intersections = new Hashtable<Integer, Intersection>();
		lanes = new Hashtable<Integer, Hashtable<Integer,Lane>>();

		fromToSegmentRefTable =  new Hashtable<String, Integer>();
		segmentRefTable = new  Hashtable<Integer , ArrayList<Integer>>(); 
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
	    
	    
	    LaneConnector tempLaneConnector = new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane);
	    //Put into lane connector table
	    laneConnectors.put(objID, new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane));
	    collectSignalLineInfo(objID,tempLaneConnector);
	    
	    //Use from-segment and to-segment form a reference table, to check from-segment & to-segment pair against lane connector id
	    String fromToSegmentKey = Integer.toHexString(fromSegmentKEY)+"&"+Integer.toHexString(toSegmentKEY);
	    fromToSegmentRefTable.put(fromToSegmentKey, objID);

	    
	    
/*		System.out.println(fromSegmentKEY +"	" + toSegmentKEY);
		System.out.println(props.get("from-segment") + "	" + props.get("to-segment"));
		System.out.println();
*/				
	    if(segmentRefTable.containsKey(fromSegmentKEY)){
	    	segmentRefTable.get(fromSegmentKEY).add(objID);	
	    	segmentRefTable.get(fromSegmentKEY).add(toSegmentKEY);
	    
	    } else{
	    	ArrayList<Integer> toSegmentList = new ArrayList<Integer>();
	    	toSegmentList.add(objID);
	    	toSegmentList.add(toSegmentKEY);
	    	segmentRefTable.put(fromSegmentKEY, toSegmentList);
	    }
	    
	    
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
	
	private void populateIntersections(){

		for(Intersection intersection : intersections.values()){		
			ArrayList <Integer> tempIntersectLinkIDs = intersection.getSigalLinkIDs();
			Hashtable<Integer, Integer> intersectLinkSegmentIDTable = new Hashtable<Integer, Integer>();
			
			int[] fromSegmentList = new int[]{-1,-1,-1,-1};
			int[] toSegmentList = new int[]{-1,-1,-1,-1};
			
			int intersectionNodeID = intersection.getIntersectNodeID(); 
						
			//Search all the Links
			for(int i = 0; i<tempIntersectLinkIDs.size();i++ ){	
				int tempLinkID = tempIntersectLinkIDs.get(i);
				//ArrayList<Integer> tempSegmentIDs = roadNetworkItemsMapTable.findSegmentIDWithLinkID(tempLinkID);
				ArrayList<Integer> tempSegmentIDs = new ArrayList<Integer>();
				Enumeration<Integer> segmentKeys = segments.keys();
							
				while(segmentKeys.hasMoreElements()){
					
					Object aKey = segmentKeys.nextElement();
					Integer segmentID = (Integer) aKey;
					Segment tempSegment = segments.get(aKey);
					int parentLinkID = tempSegment.getparentLinkID();
				
					if(tempLinkID == parentLinkID ){
						tempSegmentIDs.add(segmentID);
					}
				
				}
				
				//Find segment that related to intersection node
				for(int j = 0; j< tempSegmentIDs.size();j++){
					
					if(segments.containsKey(tempSegmentIDs.get(j))){
						
						Segment tempSegment = segments.get(tempSegmentIDs.get(j));	
						
						//Check if the segment is come from the intersection node
						if(tempSegment.getTo().getID() == intersectionNodeID)
						{
							//Put it into respective table
							if(i == 0){
								intersectLinkSegmentIDTable.put(Mapping.VA_FROM_SEGMENT, tempSegmentIDs.get(j));
								fromSegmentList[0] = tempSegmentIDs.get(j);
							} else if( i == 1){
								intersectLinkSegmentIDTable.put(Mapping.VB_FROM_SEGMENT, tempSegmentIDs.get(j));
								fromSegmentList[1] = tempSegmentIDs.get(j);
							} else if(i == 2){
								intersectLinkSegmentIDTable.put(Mapping.VC_FROM_SEGMENT, tempSegmentIDs.get(j));
								fromSegmentList[2] = tempSegmentIDs.get(j);
							} else if(i == 3){
								intersectLinkSegmentIDTable.put(Mapping.VD_FROM_SEGMENT, tempSegmentIDs.get(j));
								fromSegmentList[3] = tempSegmentIDs.get(j);
							}
							
						} else if(tempSegment.getFrom().getID() == intersectionNodeID){
					
							//Put it into respective table
							if(i == 0){
								intersectLinkSegmentIDTable.put(Mapping.VA_TO_SEGMENT, tempSegmentIDs.get(j));
								toSegmentList[0] = tempSegmentIDs.get(j);
							} else if( i == 1){
								intersectLinkSegmentIDTable.put(Mapping.VB_TO_SEGMENT, tempSegmentIDs.get(j));
								toSegmentList[1] = tempSegmentIDs.get(j);
							} else if(i == 2){
								intersectLinkSegmentIDTable.put(Mapping.VC_TO_SEGMENT, tempSegmentIDs.get(j));
								toSegmentList[2] = tempSegmentIDs.get(j);
							} else if(i == 3){
								intersectLinkSegmentIDTable.put(Mapping.VD_TO_SEGMENT, tempSegmentIDs.get(j));
								toSegmentList[3] = tempSegmentIDs.get(j);
							}	
						}
							
							
					} else {
						
						System.out.println("Error, no such segments in segment table "+Integer.toHexString(tempSegmentIDs.get(j))+" -- RoadNetwork,populateIntersection ");
						
					}
					
				}	
						
			}
							
			Hashtable<Integer, ArrayList<ArrayList<TrafficSignalLine>>> signalList = helperAllocateDirection(fromSegmentList,toSegmentList);			
			Enumeration<Integer> signalListKeys = signalList.keys();
			
			while(signalListKeys.hasMoreElements()){
				
				Object aKey = signalListKeys.nextElement();
				Integer linkNumber = (Integer)aKey;
				ArrayList<ArrayList<TrafficSignalLine>> signalListPerLink = signalList.get(linkNumber);
				
				if(linkNumber == 0)
				{
					intersection.setVaTrafficSignal(signalListPerLink);
					
				}else if (linkNumber == 1){
					intersection.setVbTrafficSignal(signalListPerLink);
					
				}else if (linkNumber == 2){

					intersection.setVcTrafficSignal(signalListPerLink);
					
				}else if (linkNumber == 3){
					intersection.setVdTrafficSignal(signalListPerLink);

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
	
	private Hashtable<Integer, ArrayList<ArrayList<TrafficSignalLine>>> helperAllocateDirection(int[] fromSegmentList, int [] toSegmentList){
		
		Hashtable<Integer, ArrayList<ArrayList<TrafficSignalLine>>> list = new Hashtable<Integer,ArrayList<ArrayList<TrafficSignalLine>>>();
		
		for(int i = 0;i<fromSegmentList.length;i++){
			
			
			ArrayList<ArrayList<TrafficSignalLine>> tempDirectionalSignalLines = new ArrayList<ArrayList<TrafficSignalLine>>();
			
			int fromSegmentKey = fromSegmentList[i]; 	
			
			if(segmentRefTable.containsKey(fromSegmentKey)){	
			
				ArrayList<Integer> tempToSegmentList = segmentRefTable.get(fromSegmentKey);

				ArrayList<TrafficSignalLine> tempLeftTurn = new ArrayList<TrafficSignalLine>();
				ArrayList<TrafficSignalLine> tempStraightTurn = new ArrayList<TrafficSignalLine>();
				ArrayList<TrafficSignalLine> tempRightTurn = new ArrayList<TrafficSignalLine>();

				tempDirectionalSignalLines.add(tempLeftTurn);
				tempDirectionalSignalLines.add(tempStraightTurn);
				tempDirectionalSignalLines.add(tempRightTurn);
	
				for(int j=1;j<tempToSegmentList.size();j+=2){				

					for(int k = 0; k < toSegmentList.length;k++){
					
						
						if(i == 0)
						{
			
							if(k == 0){
								//No U-turn
								continue;
							}
							else{

								if(tempToSegmentList.get(j) == toSegmentList[k]){
									
									int direction = i - k;
									TrafficSignalLine tempSignal = trafficSignalLines.get(tempToSegmentList.get(j-1));							
									
									if(direction == -3) {							
										tempLeftTurn.add(tempSignal);
						
									}else if(direction == -2){
										tempStraightTurn.add(tempSignal);
									}else if(direction == -1){
										tempRightTurn.add(tempSignal);	
									}
									
								}
									
							}
						
						} else if(i == 1){
							
							if(k == 1){
								//No U-turn
								continue;
							}
							else{
								if(tempToSegmentList.get(j) == toSegmentList[k]){
									
									int direction = i - k;
									TrafficSignalLine tempSignal = trafficSignalLines.get(tempToSegmentList.get(j-1));
									if(direction == 1) {						
										tempLeftTurn.add(tempSignal);
									}else if(direction == -2){
										tempStraightTurn.add(tempSignal);
									}else if(direction == -1){
										tempRightTurn.add(tempSignal);
									}
									
								}
									
							}
							
						} else if(i == 2){
							if(k == 2){
								//No U-turn
								continue;
							}
							else{
								if(tempToSegmentList.get(j) == toSegmentList[k]){
									
									int direction = i - k;
									TrafficSignalLine tempSignal = trafficSignalLines.get(tempToSegmentList.get(j-1));
									
									if(direction == 1) {	
										tempLeftTurn.add(tempSignal);
									}else if(direction == 2){
										tempStraightTurn.add(tempSignal);
									}else if(direction == -1){
										tempRightTurn.add(tempSignal);
									}
									
								}
									
							}
						} else if(i == 3){
							if(k == 3){
								//No U-turn
								continue;
							}
							else{
								if(tempToSegmentList.get(j) == toSegmentList[k]){
									
									int direction = i - k;
									TrafficSignalLine tempSignal = trafficSignalLines.get(tempToSegmentList.get(j-1));		
									if(direction == 1) {			
										tempLeftTurn.add(tempSignal);
									}else if(direction == 2){
										tempStraightTurn.add(tempSignal);
									}else if(direction == 3){
										tempRightTurn.add(tempSignal);
									}
									
								}
											
							}
				
						} 
						
					} // End k loop
				} // End j loop
			
			}
			list.put(i, tempDirectionalSignalLines);
		} // End i loop
	
		return list;
	}
	
	//Coordinate conversion 
	/*private void convertToLocal(int width, int height){
		double width5Percent = 0.05 * (cornerLR.x - cornerTL.x);
		double height5Percent = 0.05 * (cornerLR.y - cornerTL.y);
		
		DPoint newTL = new DPoint(cornerTL.x-width5Percent, cornerTL.y-height5Percent);
		DPoint newLR = new DPoint(cornerLR.x+width5Percent, cornerLR.y+height5Percent);
		
		for(int i=0;i<localPoints.size();i++){			
			localPoints.get(i).getLocalPos().scaleVia(newTL, newLR, width, height);
		}
	}*/
}






