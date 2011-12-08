package sim_mob.vis.network;

import java.io.*;
import java.util.*;
import java.util.regex.*;

import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.util.Mapping;
import sim_mob.vis.util.Utility;

/**
 * The RoadNetwork is the top-level object containing all relevant details about our 
 * static RoadNetwork.
 */
public class RoadNetwork {
	private DPoint cornerTL;
	private DPoint cornerLR;
	
	private Hashtable<Integer, Node> nodes;
	private Hashtable<Integer, Link> links;
	private Hashtable<Integer, Segment> segments;
	private Hashtable<Integer,Hashtable<Integer,LaneMarking>> linaMarkings;
	private Hashtable<Integer, Crossing> crossings;
	private Hashtable<Integer, LaneConnector> laneConnectors;
	private Hashtable<Integer, Hashtable<Integer,Lane> > lanes;
	private Hashtable<Integer, TrafficSignalLine> trafficSignalLines;
	private Hashtable<Integer, TrafficSignalCrossing> trafficSignalCrossings;
	private Hashtable<Integer, Intersection> intersections; 
	private Hashtable<Integer, CutLine> cutLines;

	private Hashtable<String, Integer> fromToSegmentRefTable;
	
	private Hashtable<Integer,ArrayList<Integer>> segmentRefTable;
	
	
	//Testing on intersections
	//private ArrayList<Integer> intersecSegmentID;
	
	public DPoint getTopLeft() { return cornerTL; }
	public DPoint getLowerRight() { return cornerLR; }
	public Hashtable<Integer, Node> getNodes() { return nodes; }
	public Hashtable<Integer, Link> getLinks() { return links; }
	public Hashtable<Integer, Segment> getSegments() { return segments; }
	public Hashtable<Integer, Hashtable<Integer,LaneMarking>> getLaneMarkings(){ return linaMarkings; }
	public Hashtable<Integer, Crossing> getCrossings() { return crossings; }
	public Hashtable<Integer, Hashtable<Integer,Lane> > getLanes(){return lanes;}
	public Hashtable<Integer, TrafficSignalLine> getTrafficSignalLine(){return trafficSignalLines;}
	public Hashtable<Integer, TrafficSignalCrossing> getTrafficSignalCrossing() {return trafficSignalCrossings;}
	public Hashtable<Integer, Intersection> getIntersection(){return intersections;}
	public Hashtable<Integer, CutLine> getCutLine(){return cutLines;}

	/**
	 * Load the network from a filestream.
	 */
	
	public RoadNetwork(BufferedReader inFile) throws IOException {

		nodes = new Hashtable<Integer, Node>();
		links = new Hashtable<Integer, Link>();
		segments = new Hashtable<Integer, Segment>();
		linaMarkings = new Hashtable<Integer,Hashtable<Integer,LaneMarking>>();
		lanes = new Hashtable<Integer, Hashtable<Integer,Lane>>();
		crossings = new Hashtable<Integer, Crossing>();
		laneConnectors = new Hashtable<Integer, LaneConnector>();
		trafficSignalLines = new Hashtable<Integer, TrafficSignalLine>(); 
		trafficSignalCrossings = new Hashtable<Integer, TrafficSignalCrossing>();
		intersections = new Hashtable<Integer, Intersection>();
		cutLines =  new Hashtable<Integer, CutLine>();

		fromToSegmentRefTable =  new Hashtable<String, Integer>();
		segmentRefTable = new  Hashtable<Integer , ArrayList<Integer>>(); 
	
		
		//Testing
		//intersecSegmentID = new ArrayList<Integer>();
	
		
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
	
		
		//Populate Intersections
		this.populateIntersections();
		
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
		} else if(objType.equals("CutLine")){
			parseCutLine(frameID, objID, rhs);
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
	    if (startNode==null) {
	    	throw new IOException("Unknown node id: " + Integer.toHexString(startNodeKEY));
	    }
	    if (endNode==null) {
	    	throw new IOException("Unknown node id: " + Integer.toHexString(endNodeKEY));
	    }
	    
	    //Create a new Link, save it
	    links.put(objID, new Link(name, startNode, endNode));
	}
	
	private void parseLineMarking(int frameID, int objID, String rhs) throws IOException {
	 
		//Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties. for lanes, it checks only parent-segment only as the number of lanes is not fixed
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"parent-segment"});
	    
	    
	    int parentKey = Utility.ParseIntOptionalHex(props.get("parent-segment"));
	    Enumeration<String> keys = props.keys();	   
	    Hashtable<Integer,LaneMarking> tempLineTable = new Hashtable<Integer,LaneMarking>();
	    Hashtable<Integer,Lane> tempLaneTable = new Hashtable<Integer,Lane>();
	    ArrayList<Integer> lineNumbers = new ArrayList<Integer>();
	    Hashtable<Integer, ArrayList<Integer>> lineMarkingPositions = new Hashtable<Integer, ArrayList<Integer>>();
	    int sideWalkLane1 = -1;
	    int sideWalkLane2 = -1;
	    while(keys.hasMoreElements()){
		    
	    	String key = keys.nextElement().toString();

	    	//Get Segment
	    	if(key.contains("parent-segment")){
	    		continue;
	    	}
	    	
	    	//Check whether the lane is a sidewalk
	    	Matcher m = Utility.NUM_REGEX.matcher(key);
	    	Integer lineNumber = null;
	    	while(m.find()){
	    		lineNumber = Integer.parseInt(m.group());		
	    	}
	    	
	    	
	    	//Extract node information
	    	if(key.contains("sidewalk")){
	    		//keep track the side walk lane number
	    		if(sideWalkLane1 == -1)
	    		{
		    		sideWalkLane1 = lineNumber;	    			
	    		}
	    		else
	    		{
	    			sideWalkLane2 = lineNumber;
	    		}
	    		
	    	}else{
	    		
	    		ArrayList<Integer> pos = new ArrayList<Integer>();
	    		pos = Utility.ParseLaneNodePos(props.get(key));
	    		Node startNode = new Node(pos.get(0), pos.get(1), false,null);
	    		Node endNode = new Node(pos.get(2), pos.get(3), false,null);
	    		tempLineTable.put(lineNumber, new LaneMarking(startNode,endNode,false,lineNumber,parentKey));
	    
	    		//Add lane number to the tracking list
		    	if(lineNumber != null){
		    		lineNumbers.add(lineNumber);
		    	}
		    		
	    		lineMarkingPositions.put(lineNumber, pos);	
	    	}
	   
	    } 
	    
	    //Find the first sidewalk line and mark it 
	    if(sideWalkLane1!=-1){
	    	tempLineTable.get(sideWalkLane1).setSideWalk(true);
	    	tempLineTable.get((sideWalkLane1+1)).setSideWalk(true);
	    }
	    
	    //Find the second sidewalk line and mark it 
	    if(sideWalkLane2!=-1){
	    	tempLineTable.get(sideWalkLane2).setSideWalk(true);
	    	tempLineTable.get((sideWalkLane2+1)).setSideWalk(true);
	    }

	    
	    //Sort array list to ascending order
	    Collections.sort(lineNumbers);
	    	    
	    for(int i = 0;i<lineMarkingPositions.size()-1;i++){
	    	int j=i+1;

	    	int startMiddleX = (lineMarkingPositions.get(i).get(0) + lineMarkingPositions.get(j).get(0))/2;
	    	int startMiddleY = (lineMarkingPositions.get(i).get(1) + lineMarkingPositions.get(j).get(1))/2;
	    		
	    	int endMiddleX = (lineMarkingPositions.get(i).get(2) + lineMarkingPositions.get(j).get(2))/2;
	    	int endMiddleY = (lineMarkingPositions.get(i).get(3) + lineMarkingPositions.get(j).get(3))/2;
	    		
	    	Lane tempLane = new Lane(i,new Node(startMiddleX, startMiddleY,true, null),new Node(endMiddleX,endMiddleY,false,null));	    		
	    	
	    	tempLaneTable.put(i,tempLane);
	    	   	
	    }
	    lanes.put(parentKey, tempLaneTable);	 
	    
	    
	    //Create a new Lane, save it
	    linaMarkings.put(objID, tempLineTable);
	    
	    
	    
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
	    if (parent==null) {
	    	throw new IOException("Unknown Link id: " + Integer.toHexString(parentLinkID));
	    }
	    if (fromNode==null) {
	    	throw new IOException("Unknown node id: " + Integer.toHexString(fromNodeID));
	    }
	    if (toNode==null) {
	    	throw new IOException("Unknown node id: " + Integer.toHexString(toNodeID));
	    }
	    
	    //Create a new Link, save it
	    segments.put(objID, new Segment(parent, fromNode, toNode, parentLinkID));

	    
	}
	
	private void parseNode(int frameID, int objID, String rhs, boolean isUni, double[] xBounds, double[] yBounds) throws IOException {
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"xPos", "yPos"});
	    
	    //Now save the position information
	    double x = Double.parseDouble(props.get("xPos"));
	    double y = Double.parseDouble(props.get("yPos"));
	    
	    Utility.CheckBounds(xBounds, x);
	    Utility.CheckBounds(yBounds, y);
	    
	    nodes.put(objID, new Node(x, y, isUni,objID));
	    
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
	   
	    //Create a new Crossing, save it
	    crossings.put(objID, new Crossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,objID));
	    trafficSignalCrossings.put(objID, new TrafficSignalCrossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,objID));
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
	
	private void parseCutLine(int frameID, int objID, String rhs) throws IOException{
	    //Check frameID
	    if (frameID!=0) {
	    	throw new IOException("Unexpected frame ID, should be zero");
	    }
	    
	    //Check and parse properties.
	    Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[]{"startPointX", "startPointY", "endPointX", "endPointY","color"});
	    
	    int startPosintX = Integer.parseInt(props.get("startPointX"));
	    int endPosintX = Integer.parseInt(props.get("endPointX"));
	    int startPosintY = Integer.parseInt(props.get("startPointY"));
	    int endPosintY = Integer.parseInt(props.get("endPointY"));
	    String color = props.get("color");
	    cutLines.put(objID, new CutLine(new Node(startPosintX,startPosintY, true, -1),new Node(endPosintX,endPosintY, true, -1),color));
		
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
	

}






