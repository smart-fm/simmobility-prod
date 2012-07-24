package sim_mob.vis.network;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;

import java.io.*;
import java.util.*;
import java.util.regex.*;

import javax.swing.SwingUtilities;

import sim_mob.vis.controls.NetworkPanel;
import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.network.basic.Vect;
import sim_mob.vis.simultion.DriverTick;
import sim_mob.vis.simultion.GsonResObj;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.FastLineParser;
import sim_mob.vis.util.Mapping;
import sim_mob.vis.util.Utility;
import sim_mob.vis.Main;
import sim_mob.vis.ProgressUpdateRunner;
import sim_mob.vis.network.Intersection;

/**
 * The RoadNetwork is the top-level object containing all relevant details about our 
 * static RoadNetwork.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Matthew Bremer Bruchon
 */
public class RoadNetwork {
	//Temp; move to config
	private static final Color Annotations_AimsunBgColor = new Color(0xFF,0x66, 0x00, 0xAA);
	private static final Color Annotations_AimsunFgColor = new Color(0xFF, 0xCC, 0xCC);
	private static final Color Annotations_MitsimBgColor = new Color(0x00, 0xCC, 0xFF, 0xAA);
	private static final Color Annotations_MitsimFgColor = new Color(0xCC, 0xCC, 0xFF);	
	private static final Color Annotations_FontColor = new Color(0x00, 0x00, 0x00);
	
	//private DPoint cornerTL;
	//private DPoint cornerLR;
	
	private Hashtable<Integer, Node> nodes;
	private Hashtable<Integer, BusStop> busstop;
	private ArrayList<Annotation> annot_aimsun;
	private ArrayList<Annotation> annot_mitsim;

	private Hashtable<Integer, Link> links;
	private Hashtable<String, LinkName> linkNames;
	private Hashtable<Integer, Segment> segments;
	private Hashtable<Integer,Hashtable<Integer,LaneMarking>> linaMarkings;
	private Hashtable<Integer, Crossing> crossings;
	private Hashtable<Integer, LaneConnector> laneConnectors;
	private Hashtable<Integer, Hashtable<Integer,Lane> > lanes;
	private Hashtable<Integer, TrafficSignalLine> trafficSignalLines;//at present this relatively container and its get function are mainly used in addAllLaneSignals and populate intersection, it can be ommited if the mentioned functions are its primary users
	private Hashtable<Integer, TrafficSignalCrossing> trafficSignalCrossings;//but this container may need to remain here coz unlike trafficSignalLines, it is directly created from input file
	private Hashtable<Integer, Intersection> intersections; 
	private Hashtable<Integer, CutLine> cutLines;
	private Hashtable<Integer, DriverTick> drivertick;

	private Hashtable<String, Integer> fromToSegmentRefTable;
	
	private Hashtable<Integer,ArrayList<Integer>> segmentRefTable;
	
	//                segID              lane#   laneID
	private Hashtable<Integer,Hashtable<Integer,Integer>> segmentToLanesTable;
	
	
	//Testing on intersections
	//private ArrayList<Integer> intersecSegmentID;
	
	//public DPoint getTopLeft() { return cornerTL; }
	//public DPoint getLowerRight() { return cornerLR; }
	public Hashtable<Integer, Node> getNodes() { return nodes; }
	public Hashtable<Integer, BusStop> getBusStop() { return busstop; }
	public ArrayList<Annotation> getAimsunAnnotations() { return annot_aimsun; }
	public ArrayList<Annotation> getMitsimAnnotations() { return annot_mitsim; }
	
	public Hashtable<Integer, Link> getLinks() { return links; }
	public Hashtable<String, LinkName> getLinkNames() { return linkNames; }
	public Hashtable<Integer, Segment> getSegments() { return segments; }
	public Hashtable<Integer, Hashtable<Integer,LaneMarking>> getLaneMarkings(){ return linaMarkings; }
	public Hashtable<Integer, Crossing> getCrossings() { return crossings; }
	public Hashtable<Integer, Hashtable<Integer,Lane> > getLanes(){return lanes;}
	public Hashtable<Integer, TrafficSignalLine> getTrafficSignalLine(){return trafficSignalLines;}
	public Hashtable<Integer, TrafficSignalCrossing> getTrafficSignalCrossing() {return trafficSignalCrossings;}
	public Hashtable<Integer, Intersection> getIntersections(){return intersections;}
	public Hashtable<Integer, CutLine> getCutLine(){return cutLines;}
	public Hashtable<Integer, DriverTick> getDriverTick(){return drivertick;}
	

	/**
	 * Load the network from a filestream.
	 */
	public void loadFileAndReport(BufferedReader inFile, long fileLength, NetworkPanel progressUpdate) throws IOException {
		nodes = new Hashtable<Integer, Node>();
		busstop = new Hashtable<Integer, BusStop>();
		annot_aimsun = new ArrayList<Annotation>();
		annot_mitsim = new ArrayList<Annotation>();
		annot_aimsun = new ArrayList<Annotation>();
		annot_mitsim = new ArrayList<Annotation>();
	
		links = new Hashtable<Integer, Link>();
		linkNames = new Hashtable<String, LinkName>();
		segments = new Hashtable<Integer, Segment>();
		linaMarkings = new Hashtable<Integer,Hashtable<Integer,LaneMarking>>();
		lanes = new Hashtable<Integer, Hashtable<Integer,Lane>>();
		crossings = new Hashtable<Integer, Crossing>();
		laneConnectors = new Hashtable<Integer, LaneConnector>();
		trafficSignalLines = new Hashtable<Integer, TrafficSignalLine>(); 
		trafficSignalCrossings = new Hashtable<Integer, TrafficSignalCrossing>();
		intersections = new Hashtable<Integer, Intersection>();
		cutLines =  new Hashtable<Integer, CutLine>();
		drivertick =  new Hashtable<Integer, DriverTick>();

		fromToSegmentRefTable =  new Hashtable<String, Integer>();
		segmentRefTable = new  Hashtable<Integer , ArrayList<Integer>>(); 
		segmentToLanesTable = new Hashtable<Integer,Hashtable<Integer,Integer>>();
		
		//temp
		FastLineParser flp = new FastLineParser();
	
		
		//Provide feedback to the user
		long totalBytesRead = 0;
		long lastKnownTotalBytesRead = 0;
		if (progressUpdate!=null) {
			SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, 0.0, false, new Color(0x00, 0x00, 0xFF), ""));
		}
//		System.out.println("Inside rn.loadFileAndReport(br, 0, null);");
		//Read
		String line;
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
			
			//New-style json strings use {}, while old-style ones use ().
			boolean oldStyle = line.startsWith("(") && line.endsWith(")");
			boolean newStyle = line.startsWith("{") && line.endsWith("}");
			if (!oldStyle && !newStyle) {
				continue;
			}
			//Parsing depends on how the line is structured.
			if (newStyle) {
//				System.out.println("Inside rn.loadFileAndReport.newstyle");
				//Parse this line as json.
				GsonResObj gRes = Utility.ParseGsonLine(line);
				int tTick = gRes.getTimeTick();
				//Add this object to the simulation
				if(tTick < 0)//TODO find a way to get rid of this. time tick 0 i creating problem as it is common between static network and simulation data
				//Add this object to the simulation
//				System.out.println("before gRes.addSelfToSimulation");
				gRes.addSelfToSimulation(this, null);
			} else {
				//Parse this line as text and pseudo-json.
				Utility.ParseResults pRes = Utility.ParseLogLine(flp, line);
				if (pRes.isError()) {
					throw new RuntimeException("Error parsing line: \n  " + pRes.errorMsg);
				}

			    //Pass this off to a different function based on the type
			    try {
			    	dispatchConstructionRequest(pRes);
			    } catch (IOException ex) {
			    	throw new IOException(ex.getMessage() + "\n...on line: " + line);
			    }
			}
		    
		    
		    if (pushUpdate) {
			    try {
			    	Thread.sleep(1);
			    } catch (InterruptedException ex) {
			    	throw new RuntimeException(ex);
			    }
		    }
		}//while loop
		
		//Save bounds
		//cornerTL = new DPoint(xBounds[0], yBounds[0]);
		//cornerLR = new DPoint(xBounds[1], yBounds[1]);
	
		
		//Add Link n ames
		this.addLinkNames();
		if(Main.NEW_SIGNAL) //this is definitly a temporary fix (and definitly works :)) )
		//Populate Intersections
			this.populateIntersections_newStyle();
		else
			this.populateIntersections();
		
		//Fix up connections between segments to look 
		//pretty where # of lanes changes
		this.smoothSegmentJoins();
		
		//Space node annotations as necessary to avoid distortion
		this.spaceNodeAnnotations();
		// this.spaceBusStopAnnotations();
	}
	
	
	private void addLinkNames() {
		//Keep track and avoid drawing names more than once.
		Set<String> alreadyDrawn = new HashSet<String>();
		for (Link ln : getLinks().values()) {
			String key = ln.getAuthoritativeRoadName();
			if (!alreadyDrawn.contains(key)) {
				alreadyDrawn.add(key);
				this.linkNames.put(key, new LinkName(ln, ln.getName())); 
			}
		}
	}
	
			
	//Returns true if we should continue processing.
	private boolean dispatchConstructionRequest(Utility.ParseResults pRes) throws IOException {
		//Check frameID
		//TODO: Re-enable once we're sure that we will only get NETWORK results in this function 
		//      (right now we read the log file twice).
	    //if (pRes.frame!=0) {
	    //	throw new IOException("Unexpected frame ID, should be zero");
	    //}
		
		//Nodes are displayed the same
		if (pRes.type.equals("multi-node") || pRes.type.equals("uni-node")) {
			parseNode(pRes, pRes.type.equals("uni-node"));
		} else if (pRes.type.equals("link")) {
			parseLink(pRes);
		} else if (pRes.type.equals("road-segment")) {
			parseSegment(pRes);
		} else if (pRes.type.equals("lane")){
			parseLineMarking(pRes);
		} else if(pRes.type.equals("crossing")){
			parseCrossing(pRes);
		} else if(pRes.type.equals("lane-connector")){
			parseLaneConnector(pRes);
		} else if(pRes.type.equals("Signal-location")){
			parseSignalLocation(pRes);
		} else if(pRes.type.equals("CutLine")){
			parseCutLine(pRes);
		} else if(pRes.type.equals("busstop")){
			parseBusStop(pRes);
		} else if (pRes.frame>0) {
			//We've started on runtime data.
			return false;
		}
		
		return true;

		
	}
		
	private void parseLink(Utility.ParseResults pRes) throws IOException {

	    
	    //Check and parse properties.
		if (!pRes.confirmProps(new String[]{"road-name", "start-node", "end-node", "fwd-path", "rev-path"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}
	    
	    //Now save the relevant information
	    String name = pRes.properties.get("road-name");
	    int startNodeKEY = Utility.ParseIntOptionalHex(pRes.properties.get("start-node"));
	    int endNodeKEY = Utility.ParseIntOptionalHex(pRes.properties.get("end-node"));
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
	    Link toAdd = new Link(name, startNode, endNode, pRes.objID);
	    toAdd.setFwdPathSegmentIDs(Utility.ParseLinkPaths(pRes.properties.get("fwd-path")));
	    toAdd.setRevPathSegmentIDs(Utility.ParseLinkPaths(pRes.properties.get("rev-path")));
	    links.put(pRes.objID, toAdd);
	}
	
	private void parseLineMarking(Utility.ParseResults pRes) throws IOException {
	    
	    //Check and parse properties. for lanes, it checks only parent-segment only as the number of lanes is not fixed
		if (!pRes.confirmProps(new String[]{"parent-segment"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}
	    
	    
	    int parentKey = Utility.ParseIntOptionalHex(pRes.properties.get("parent-segment"));	   
	    Hashtable<Integer,LaneMarking> tempLineTable = new Hashtable<Integer,LaneMarking>();
	    Hashtable<Integer,Lane> tempLaneTable = new Hashtable<Integer,Lane>();
	    ArrayList<Integer> lineNumbers = new ArrayList<Integer>();
	    Hashtable<Integer, ArrayList<Integer>> lineMarkingPositions = new Hashtable<Integer, ArrayList<Integer>>();
	    int sideWalkLane1 = -1;
	    int sideWalkLane2 = -1;
	    for (String key : pRes.properties.keySet()) {
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
	    		pos = Utility.ParseLaneNodePos(pRes.properties.get(key));
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

	    	if(segmentToLanesTable.containsKey(parentKey)){
	    		segmentToLanesTable.get(parentKey).put(i, pRes.objID);
	    	}	
	    	else{
	    		Hashtable<Integer, Integer> lanesOnSegment = new Hashtable<Integer,Integer>();
	    		lanesOnSegment.put(i, pRes.objID);
	    		segmentToLanesTable.put(parentKey, lanesOnSegment);
	    	}
	    }
	    lanes.put(parentKey, tempLaneTable);	 
	    
	    
	    //Create a new Lane, save it
	    linaMarkings.put(pRes.objID, tempLineTable);    
	}
	
	private void parseSegment(Utility.ParseResults pRes) throws IOException {

	    
	    //Check and parse properties.
		if (!pRes.confirmProps(new String[]{"parent-link", "max-speed", "lanes", "from-node", "to-node"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}
	    
	    //Now save the relevant information
	    int parentLinkID = Utility.ParseIntOptionalHex(pRes.properties.get("parent-link"));
	    Link parent = links.get(parentLinkID);
	    int fromNodeID = Utility.ParseIntOptionalHex(pRes.properties.get("from-node"));
	    int toNodeID = Utility.ParseIntOptionalHex(pRes.properties.get("to-node"));
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
	    segments.put(pRes.objID, new Segment(parent, fromNode, toNode));

	    
	}
	
	private void parseNode(Utility.ParseResults pRes, boolean isUni) throws IOException {

	    
	    //Check and parse properties.
		if (!pRes.confirmProps(new String[]{"xPos", "yPos"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}
	    
	    //Now save the position information
	    double x = Double.parseDouble(pRes.properties.get("xPos"));
	    double y = Double.parseDouble(pRes.properties.get("yPos"));
	    
	    //Utility.CheckBounds(xBounds, x);
	    //Utility.CheckBounds(yBounds, y);
	    
	    Node res = new Node(x, y, isUni, pRes.objID);
	    if (pRes.properties.containsKey("aimsun-id")) {
	    	Annotation an = new Annotation(new Point((int)x, (int)y), pRes.properties.get("aimsun-id"), 'A');
	    	an.setBackgroundColor(Annotations_AimsunBgColor);
	    	an.setBorderColor(Annotations_AimsunFgColor);
	    	an.setFontColor(Annotations_FontColor);
	    	annot_aimsun.add(an);
	    }
	    if (pRes.properties.containsKey("mitsim-id")) {
	    	Annotation an = new Annotation(new Point((int)x, (int)y), pRes.properties.get("mitsim-id"), 'M');
	    	an.setBackgroundColor(Annotations_MitsimBgColor);
	    	an.setBorderColor(Annotations_MitsimFgColor);
	    	an.setFontColor(Annotations_FontColor);
	    	annot_mitsim.add(an);
	    }
	    
	    nodes.put(pRes.objID, res);
	    
	}
		
		private void parseBusStop(Utility.ParseResults pRes) throws IOException {
		    
		    //Check and parse properties.
			if (!pRes.confirmProps(new String[]{"near-1", "near-2", "far-1", "far-2"})) {
				throw new IOException("Missing required key in type: " + pRes.type);
			}
		    
		    //Now save the relevant information
		    ScaledPoint nearOneNode = Utility.ParseCrossingNodePos(pRes.properties.get("near-1"));
		    ScaledPoint nearTwoNode = Utility.ParseCrossingNodePos(pRes.properties.get("near-2"));
		    ScaledPoint farOneNode = Utility.ParseCrossingNodePos(pRes.properties.get("far-1"));
		    ScaledPoint farTwoNode = Utility.ParseCrossingNodePos(pRes.properties.get("far-2"));
		    
		    
		    BusStop res = new BusStop(nearOneNode, nearTwoNode, farOneNode,farTwoNode, pRes.objID);
		
		    busstop.put(pRes.objID, res);
		    
		}
	//
	private void parseCrossing(Utility.ParseResults pRes) throws IOException {

	    
	    //Check and parse properties.
		if (!pRes.confirmProps(new String[]{"near-1", "near-2", "far-1", "far-2"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}

	    
	    //Now save the relevant information
	    ScaledPoint nearOneNode = Utility.ParseCrossingNodePos(pRes.properties.get("near-1"));
	    ScaledPoint nearTwoNode = Utility.ParseCrossingNodePos(pRes.properties.get("near-2"));
	    ScaledPoint farOneNode = Utility.ParseCrossingNodePos(pRes.properties.get("far-1"));
	    ScaledPoint farTwoNode = Utility.ParseCrossingNodePos(pRes.properties.get("far-2"));
	   
	    //Create a new Crossing, save it
	    crossings.put(pRes.objID, new Crossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,pRes.objID));
	    trafficSignalCrossings.put(pRes.objID, new TrafficSignalCrossing(nearOneNode,nearTwoNode,farOneNode,farTwoNode,pRes.objID));
	}
	
	private void parseLaneConnector(Utility.ParseResults pRes) throws IOException{
	    
		//Check and parse properties.
		if (!pRes.confirmProps(new String[]{"from-segment", "from-lane","to-segment","to-lane"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}

	    
	    //Now save the relevant information
	    int fromSegmentKEY = Utility.ParseIntOptionalHex(pRes.properties.get("from-segment"));
	    int toSegmentKEY = Utility.ParseIntOptionalHex(pRes.properties.get("to-segment"));
	    int fromLane = Utility.ParseIntOptionalHex(pRes.properties.get("from-lane"));
	    int toLane = Utility.ParseIntOptionalHex(pRes.properties.get("to-lane"));
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
	    laneConnectors.put(pRes.objID, new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane));
	    collectSignalLineInfo(pRes.objID,tempLaneConnector);
	    
	    //Use from-segment and to-segment form a reference table, to check from-segment & to-segment pair against lane connector id
	    String fromToSegmentKey = Integer.toHexString(fromSegmentKEY)+"&"+Integer.toHexString(toSegmentKEY);
	    fromToSegmentRefTable.put(fromToSegmentKey, pRes.objID);

	    
	    
/*		System.out.println(fromSegmentKEY +"	" + toSegmentKEY);
		System.out.println(pRes.properties.get("from-segment") + "	" + pRes.properties.get("to-segment"));
		System.out.println();
*/				
	    if(segmentRefTable.containsKey(fromSegmentKEY)){
	    	segmentRefTable.get(fromSegmentKEY).add(pRes.objID);	
	    	segmentRefTable.get(fromSegmentKEY).add(toSegmentKEY);
	    
	    } else{
	    	ArrayList<Integer> toSegmentList = new ArrayList<Integer>();
	    	toSegmentList.add(pRes.objID);
	    	toSegmentList.add(toSegmentKEY);
	    	segmentRefTable.put(fromSegmentKEY, toSegmentList);
	    }
	    
	    
	}

	private void parseSignalLocation(Utility.ParseResults pRes) throws IOException{		

	    
		//Check and parse properties.
		if (!pRes.confirmProps(new String[]{"node","va","aa","pa","vb","ab","pb","vc","ac","pc","vd","ad","pd"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}

	    //Now save the relevant information
	    int intersectionNodeID = Utility.ParseIntOptionalHex(pRes.properties.get("node"));
	    int linkVaID = Utility.ParseIntOptionalHex(pRes.properties.get("va"));
	    int linkVbID = Utility.ParseIntOptionalHex(pRes.properties.get("vb"));
	    int linkVcID = Utility.ParseIntOptionalHex(pRes.properties.get("vc"));
	    int linkVdID = Utility.ParseIntOptionalHex(pRes.properties.get("vd"));
	    int linkPaID = Utility.ParseIntOptionalHex(pRes.properties.get("pa"));
	    int linkPbID = Utility.ParseIntOptionalHex(pRes.properties.get("pb"));
	    int linkPcID = Utility.ParseIntOptionalHex(pRes.properties.get("pc"));
	    int linkPdID = Utility.ParseIntOptionalHex(pRes.properties.get("pd"));

	    ArrayList <Integer>  tempLinkIDs = new ArrayList<Integer>(
	    			Arrays.asList(linkVaID, linkVbID, linkVcID, linkVdID)); 
	    
	    ArrayList <Integer> tempCrossingIDs = new ArrayList<Integer>(
	    			Arrays.asList(linkPaID,linkPbID,linkPcID,linkPdID));
	    
	    intersections.put(pRes.objID, new Intersection(intersectionNodeID,tempLinkIDs, tempCrossingIDs));		
	
	}
	

	private void parseCutLine(Utility.ParseResults pRes) throws IOException{

	    
	    //Check and parse properties.
		if (!pRes.confirmProps(new String[]{"startPointX", "startPointY", "endPointX", "endPointY","color"})) {
			throw new IOException("Missing required key in type: " + pRes.type);
		}
	    
	    ScaledPoint startPoint = new FlippedScaledPoint(
	    	Integer.parseInt(pRes.properties.get("startPointX")),
	    	Integer.parseInt(pRes.properties.get("endPointX")));
	    ScaledPoint endPoint = new FlippedScaledPoint(
	    	Integer.parseInt(pRes.properties.get("startPointY")),
	    	Integer.parseInt(pRes.properties.get("endPointY")));
	    String color = pRes.properties.get("color");
	    cutLines.put(pRes.objID, new CutLine(startPoint, endPoint, color));
		
	}
	
	private void collectSignalLineInfo(int objID, LaneConnector laneConnector){				
		if(lanes.containsKey(laneConnector.getFromSegment()) && lanes.containsKey(laneConnector.getToSegment()) ){
	
			int fromLaneNo = laneConnector.getFromLane();
			int toLaneNo = laneConnector.getToLane();
			Lane fromLane = lanes.get(laneConnector.getFromSegment()).get(fromLaneNo);
			Lane toLane = lanes.get(laneConnector.getToSegment()).get(toLaneNo);
			TrafficSignalLine tempSignalLine = new TrafficSignalLine(fromLane, toLane, -1); 
			trafficSignalLines.put(objID, tempSignalLine);	
			
		} else{
			System.out.println("Error, No such segment -- RoadNetwork, collectSignalInfo()");
		}
	}

	private void populateIntersections_newStyle(){

//		System.out.println("Inside this.populateIntersections_newStyle()");
		for(Intersection intersection: intersections.values()){
			intersection.populateTrafficSignal(this);
		}
	}
	private void populateIntersections() throws IOException{

		System.out.println("Inside the populate intersection with intersection size of " + intersections.size());
//		System.in.read();
		for(Intersection intersection : intersections.values()){	
			System.out.println("Inside the populate intersection loop");
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
					int parentLinkID = tempSegment.getParent().getId();
				
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
		    //     segmentFrom,2D matrix segmentToX[left,straight,right]	still ambiguous!!				
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
	
	
	//Wrapper
	private void spaceNodeAnnotations() {
		Hashtable<Point, Integer> alreadySpaced = new Hashtable<Point, Integer>(); //int = conflicts
		spaceNodeAnnotations(alreadySpaced, annot_aimsun);
		spaceNodeAnnotations(alreadySpaced, annot_mitsim);
	}
		
	//Attempt to place each annotation. Avoid overlapping any existing annotations.
	//For now, this is done based on the assumption that coordinates are in centimeters.
	// Later, we can scale it to screen size.
	private void spaceNodeAnnotations(Hashtable<Point, Integer> alreadySpaced, ArrayList<Annotation> toSpace) {
		Point amt = new Point(500, 800);
		Point[] magnitudes = new Point[] {new Point(0,amt.y), new Point(amt.x,amt.y/2), new Point(amt.x,-amt.y/2), new Point(0,-amt.y) }; 
		
		for (Annotation an : toSpace) {
			//Scale down by 10, start counting if we haven't already.
			Point location = new Point((int)(an.getPos().getUnscaledX()/10), (int)(an.getPos().getUnscaledY()/10));
			if (!alreadySpaced.containsKey(location)) {
				alreadySpaced.put(location, 0);
			}
			 
			//Pull out the count, increment
			int count = alreadySpaced.get(location);
			alreadySpaced.put(location, count+1);
			 
			//We have a handful of magnitudes to choose from. Cycle through and
			//  pick the next one.
			Point mag = magnitudes[count%magnitudes.length];
			int times = (count / magnitudes.length)+1;
			an.setOffset(new Point((int)an.getPos().getUnscaledX()+mag.x*times, (int)an.getPos().getUnscaledY()+mag.y*times));
		}
	}
	
	//
	private void spaceBusStopAnnotations(Hashtable<Point, Integer> alreadySpaced, ArrayList<Annotation> toSpace) {
		Point amt = new Point(500, 800);
		Point[] magnitudes = new Point[] {new Point(0,amt.y), new Point(amt.x,amt.y/2), new Point(amt.x,-amt.y/2), new Point(0,-amt.y) }; 
		
		for (Annotation an : toSpace) {
			//Scale down by 10, start counting if we haven't already.
			Point location = new Point((int)(an.getPos().getUnscaledX()/10), (int)(an.getPos().getUnscaledY()/10));
			if (!alreadySpaced.containsKey(location)) {
				alreadySpaced.put(location, 0);
			}
			 
			//Pull out the count, increment
			int count = alreadySpaced.get(location);
			alreadySpaced.put(location, count+1);
			 
			//We have a handful of magnitudes to choose from. Cycle through and
			//  pick the next one.
			Point mag = magnitudes[count%magnitudes.length];
			int times = (count / magnitudes.length)+1;
			an.setOffset(new Point((int)an.getPos().getUnscaledX()+mag.x*times, (int)an.getPos().getUnscaledY()+mag.y*times));
		}
	}
	//
	
	
	private void smoothSegmentJoins() {
		for(Link link : links.values()){
			smoothSegmentJoins(link.getFwdPathSegmentIDs());
			smoothSegmentJoins(link.getRevPathSegmentIDs());
		}
	}

	private void smoothSegmentJoins(ArrayList<Integer> segmentIDs){
		if(segmentIDs.size() < 2)
			return;

		Integer currentSegmentID = segmentIDs.get(0);

		LaneMarking currSegSidewalkLane1 = new LaneMarking(null, null, false, 0, 0);
		LaneMarking currSegSidewalkLane2 = new LaneMarking(null, null, false, 0, 0);


		for(int i = 0; i<segmentIDs.size();i++){	
			Integer nextSegmentID = segmentIDs.get(i);
			if(segmentToLanesTable.containsKey(currentSegmentID) && segmentToLanesTable.containsKey(nextSegmentID))
			{
				Hashtable<Integer, Integer> currentSegLanes = segmentToLanesTable.get(currentSegmentID);
				Hashtable<Integer, Integer> nextSegLanes = segmentToLanesTable.get(nextSegmentID);
		
				LaneMarking nextSegSidewalkLane1 = new LaneMarking(null, null, false, 0, 0);
				LaneMarking nextSegSidewalkLane2 = new LaneMarking(null, null, false, 0, 0);

				boolean bFound = false;

				for(Integer currSegLaneID : currentSegLanes.values())
				{
					Hashtable<Integer,LaneMarking> currentSegLaneMarkTable = linaMarkings.get(currSegLaneID);
					for(LaneMarking currSegLaneMark : currentSegLaneMarkTable.values())
					{
						if(currSegLaneMark.isSideWalk()){
							if(bFound == false){
								currSegSidewalkLane1 = currSegLaneMark;								
							}
							else{
								currSegSidewalkLane2 = currSegLaneMark;								
							}
						}						
					}
				}

				bFound = false;

				for(Integer nextSegLaneID : nextSegLanes.values())
				{
					Hashtable<Integer,LaneMarking> nextSegLaneMarkTable = linaMarkings.get(nextSegLaneID);
					for(LaneMarking nextSegLaneMark : nextSegLaneMarkTable.values())
					{
						if(nextSegLaneMark.isSideWalk()){
							if(bFound == false){
								nextSegSidewalkLane1 = nextSegLaneMark;	
								bFound = true;
							}
							else{
								nextSegSidewalkLane2 = nextSegLaneMark;								
							}
						}						
					}
				}
			
				if(currSegSidewalkLane1.getLaneNumber() > currSegSidewalkLane2.getLaneNumber())
				{
					LaneMarking temp = currSegSidewalkLane1;		
					currSegSidewalkLane1 = currSegSidewalkLane2;
					currSegSidewalkLane2 = temp;
				}
		
				if(nextSegSidewalkLane1.getLaneNumber() > nextSegSidewalkLane2.getLaneNumber())
				{
					LaneMarking temp = nextSegSidewalkLane1;		
					nextSegSidewalkLane1 = nextSegSidewalkLane2;
					nextSegSidewalkLane2 = temp;
				}

				if(i != 0 && currSegSidewalkLane1.getStart()!=null && currSegSidewalkLane1.getEnd()!=null)
				{
					DPoint currSegStart1 = new DPoint(currSegSidewalkLane1.getStart().getPos().getUnscaledX(), currSegSidewalkLane1.getStart().getPos().getUnscaledY());
					DPoint currSegEnd1 = new DPoint(currSegSidewalkLane1.getEnd().getPos().getUnscaledX(), currSegSidewalkLane1.getEnd().getPos().getUnscaledY());
					DPoint currSegStart2 = new DPoint(currSegSidewalkLane2.getStart().getPos().getUnscaledX(), currSegSidewalkLane2.getStart().getPos().getUnscaledY());
					DPoint currSegEnd2 = new DPoint(currSegSidewalkLane2.getEnd().getPos().getUnscaledX(), currSegSidewalkLane2.getEnd().getPos().getUnscaledY());
			
					Vect currSidewalk1 = new Vect(currSegStart1.x, currSegStart1.y, currSegEnd1.x, currSegEnd1.y);
					Vect currSidewalk2 = new Vect(currSegStart2.x, currSegStart2.y, currSegEnd2.x, currSegEnd2.y);
			
					DPoint nextSegStart1 = new DPoint(nextSegSidewalkLane1.getStart().getPos().getUnscaledX(), nextSegSidewalkLane1.getStart().getPos().getUnscaledY());
					DPoint nextSegEnd1 = new DPoint(nextSegSidewalkLane1.getEnd().getPos().getUnscaledX(), nextSegSidewalkLane1.getEnd().getPos().getUnscaledY());
					DPoint nextSegStart2 = new DPoint(nextSegSidewalkLane2.getStart().getPos().getUnscaledX(), nextSegSidewalkLane2.getStart().getPos().getUnscaledY());
					DPoint nextSegEnd2 = new DPoint(nextSegSidewalkLane2.getEnd().getPos().getUnscaledX(), nextSegSidewalkLane2.getEnd().getPos().getUnscaledY());
					
					Vect nextSidewalk1 = new Vect(nextSegStart1.x, nextSegStart1.y, nextSegEnd1.x, nextSegEnd1.y);
					Vect nextSidewalk2 = new Vect(nextSegStart2.x, nextSegStart2.y, nextSegEnd2.x, nextSegEnd2.y);
					
					if(currentSegLanes.size() > nextSegLanes.size()) 
					{
						//Fix up 1st sidewalk lane						
						currSidewalk1.scaleVect((currSidewalk1.getMagnitude()-750.0)/currSidewalk1.getMagnitude());
						currSidewalk2.scaleVect((currSidewalk2.getMagnitude()-750.0)/currSidewalk2.getMagnitude());
						currSegSidewalkLane1.setPenultimatePt(new ScaledPoint(currSegStart1.x + currSidewalk1.getMagX(), currSegStart1.y + currSidewalk1.getMagY()));
						currSegSidewalkLane2.setPenultimatePt(new ScaledPoint(currSegStart2.x + currSidewalk2.getMagX(), currSegStart2.y + currSidewalk2.getMagY()));
						currSegSidewalkLane1.setLastPt(new ScaledPoint(nextSegStart1.x, nextSegStart1.y));
						currSegSidewalkLane2.setLastPt(new ScaledPoint(nextSegStart2.x, nextSegStart2.y));
					}
					else if(nextSegLanes.size() > currentSegLanes.size())
					{
						//Fix up second sidewalk lane
						nextSidewalk1.scaleVect((nextSidewalk1.getMagnitude()-750.0)/nextSidewalk1.getMagnitude());
						nextSidewalk2.scaleVect((nextSidewalk2.getMagnitude()-750.0)/nextSidewalk2.getMagnitude());
						nextSegSidewalkLane1.setSecondPt(new ScaledPoint(nextSegEnd1.x - nextSidewalk1.getMagX(), nextSegEnd1.y - nextSidewalk1.getMagY()));
						nextSegSidewalkLane2.setSecondPt(new ScaledPoint(nextSegEnd2.x - nextSidewalk2.getMagX(), nextSegEnd2.y - nextSidewalk2.getMagY()));
						nextSegSidewalkLane1.setStartPt(new ScaledPoint(currSegEnd1.x, currSegEnd1.y));
						nextSegSidewalkLane2.setStartPt(new ScaledPoint(currSegEnd2.x, currSegEnd2.y));
					}
				}
					
				currSegSidewalkLane1 = nextSegSidewalkLane1;
				currSegSidewalkLane2 = nextSegSidewalkLane2;
			}
			
			currentSegmentID = nextSegmentID;
		}
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
	
				for(int j=1;j<tempToSegmentList.size();j+=2){				//why alternate?-->vahid

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






