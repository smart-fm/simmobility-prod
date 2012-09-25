package sim_mob.vis.network;

import java.awt.Color;
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
	
	private Hashtable<Long, Node> nodes;
	private Hashtable<Long, Link> links;
	private Hashtable<Long, Segment> segments;
	private Hashtable<Long, Hashtable<Long,Lane> > lanes;
	private Hashtable<Long,Hashtable<Long,LaneMarking>> linaMarkings;
	private Hashtable<Long, LaneConnector> laneConnectors;
	private Hashtable<Long, BusStop> busstop;
	
	private Hashtable<Long, Crossing> crossings;
	private Hashtable<Long, TrafficSignalCrossing> trafficSignalCrossings;
	private Hashtable<Long, TrafficSignalLine> trafficSignalLines;
	private Hashtable<Long,ArrayList<Long>> segmentRefTable;
	private Hashtable<Long, Intersection> intersections; 
	private Hashtable<Long, CutLine> cutLines;
	
	private ArrayList<Annotation> annot_aimsun;
	private ArrayList<Annotation> annot_mitsim;

	private Hashtable<String, LinkName> linkNames;
	private Hashtable<Integer, DriverTick> drivertick;
	private Hashtable<String, Long> fromToSegmentRefTable;
	//                segID              lane#   laneID
	private Hashtable<Long,Hashtable<Integer,Long>> segmentToLanesTable;
	
	
	//Testing on intersections
	//private ArrayList<Integer> intersecSegmentID;
	
	//public DPoint getTopLeft() { return cornerTL; }
	//public DPoint getLowerRight() { return cornerLR; }
	public Hashtable<Long, Node> getNodes() { return nodes; }
	public Hashtable<Long, Link> getLinks() { return links; }
	public Hashtable<Long, Segment> getSegments() { return segments; }
	public Hashtable<Long, Hashtable<Long,Lane> > getLanes(){return lanes;}
	public Hashtable<Long, Hashtable<Long,LaneMarking>> getLaneMarkings(){ return linaMarkings; }
	public Hashtable<Long, BusStop> getBusStop() { return busstop; }
	
	public Hashtable<Long, Crossing> getCrossings() { return crossings; }
	public Hashtable<Long, TrafficSignalCrossing> getTrafficSignalCrossing() {return trafficSignalCrossings;}
	public Hashtable<Long, TrafficSignalLine> getTrafficSignalLine(){return trafficSignalLines;}
	public Hashtable<Long, Intersection> getIntersection(){return intersections;}
	public Hashtable<Long, CutLine> getCutLine(){return cutLines;}
	
	public ArrayList<Annotation> getAimsunAnnotations() { return annot_aimsun; }
	public ArrayList<Annotation> getMitsimAnnotations() { return annot_mitsim; }
	public Hashtable<String, LinkName> getLinkNames() { return linkNames; }
	public Hashtable<Integer, DriverTick> getDriverTick(){return drivertick;}
	

	/**
	 * Load the network from a filestream.
	 */
	public void loadFileAndReport(BufferedReader inFile, long fileLength, NetworkPanel progressUpdate) throws IOException {
		Main.NEW_SIGNAL = false;//default
		System.out.println("System NEW_SIGNAL reset to false");
		nodes = new Hashtable<Long, Node>();
		busstop = new Hashtable<Long, BusStop>();
		annot_aimsun = new ArrayList<Annotation>();
		annot_mitsim = new ArrayList<Annotation>();
		annot_aimsun = new ArrayList<Annotation>();
		annot_mitsim = new ArrayList<Annotation>();
	
		links = new Hashtable<Long, Link>();
		linkNames = new Hashtable<String, LinkName>();
		segments = new Hashtable<Long, Segment>();
		linaMarkings = new Hashtable<Long,Hashtable<Long,LaneMarking>>();
		lanes = new Hashtable<Long, Hashtable<Long,Lane>>();
		crossings = new Hashtable<Long, Crossing>();
		laneConnectors = new Hashtable<Long, LaneConnector>();
		trafficSignalLines = new Hashtable<Long, TrafficSignalLine>(); 
		trafficSignalCrossings = new Hashtable<Long, TrafficSignalCrossing>();
		intersections = new Hashtable<Long, Intersection>();
		cutLines =  new Hashtable<Long, CutLine>();
		drivertick =  new Hashtable<Integer, DriverTick>();

		fromToSegmentRefTable =  new Hashtable<String, Long>();
		segmentRefTable = new  Hashtable<Long , ArrayList<Long>>(); 
		segmentToLanesTable = new Hashtable<Long,Hashtable<Integer,Long>>();
		
		//temp
		FastLineParser flp = new FastLineParser();
	
		
		//Provide feedback to the user
		long totalBytesRead = 0;
		long lastKnownTotalBytesRead = 0;
		if (progressUpdate!=null) {
			SwingUtilities.invokeLater(new ProgressUpdateRunner(progressUpdate, 0.0, false, new Color(0x00, 0x00, 0xFF), ""));
		}
		
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
			    	if (!dispatchConstructionRequest(pRes)) {
			    		break;
			    	}
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
		
	
		//Populate Intersections
		if(Main.NEW_SIGNAL)
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
	    long startNodeKEY = Utility.ParseLongOptionalHex(pRes.properties.get("start-node"));
	    long endNodeKEY = Utility.ParseLongOptionalHex(pRes.properties.get("end-node"));
	    Node startNode = nodes.get(startNodeKEY);
	    Node endNode = nodes.get(endNodeKEY);
	    
	    //Ensure nodes exist
	    if (startNode==null) {
	    	throw new IOException("Unknown node id: " + Long.toHexString(startNodeKEY));
	    }
	    if (endNode==null) {
	    	throw new IOException("Unknown node id: " + Long.toHexString(endNodeKEY));
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
	    
	    
	    long parentKey = Utility.ParseLongOptionalHex(pRes.properties.get("parent-segment"));	   
	    Hashtable<Long,LaneMarking> tempLineTable = new Hashtable<Long,LaneMarking>();
	    Hashtable<Long,Lane> tempLaneTable = new Hashtable<Long,Lane>();
	    ArrayList<Integer> lineNumbers = new ArrayList<Integer>();
	    Hashtable<Integer, ArrayList<Integer>> lineMarkingPositions = new Hashtable<Integer, ArrayList<Integer>>();
	    long sideWalkLane1 = -1;
	    long sideWalkLane2 = -1;
	    for (String key : pRes.properties.keySet()) {
	    	//Get Segment
	    	if(key.contains("parent-segment")){
	    		continue;
	    	}
	    	
	    	//Check whether the lane is a sidewalk
	    	Matcher m = Utility.NUM_REGEX.matcher(key);
	    	int lineNumber = -1;
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
	    		
	    		ArrayList<Integer> pos = Utility.ParseLaneNodePos(pRes.properties.get(key));
	    		
	    		//NOTE: We need Nodes here *at least once* because Nodes flip the Y-axis.
	    		Node startNode = new Node(pos.get(0), pos.get(1), false, null);
	    		Node endNode = new Node(pos.get(2), pos.get(3), false, null);
	    		
	    		tempLineTable.put(new Long(lineNumber), new LaneMarking(startNode.getPos(),endNode.getPos(),false,lineNumber,parentKey));
	    
	    		//Add lane number to the tracking list
		    	if(lineNumber != -1){
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
	    	
    		/*System.out.println("Adding Lane from: (" + 
    			startMiddleX + "," + startMiddleY + "), to: (" +
				endMiddleX + "," + endMiddleY + ")"
			);*/
	    		
	    	Lane tempLane = new Lane(i,new Node(startMiddleX, startMiddleY,true, null),new Node(endMiddleX,endMiddleY,false,null));	    		
	    	
	    	tempLaneTable.put(new Long(i),tempLane);

	    	if (!segmentToLanesTable.containsKey(parentKey)) {
	    		segmentToLanesTable.put(parentKey, new Hashtable<Integer,Long>());
	    	}
	    	segmentToLanesTable.get(parentKey).put(i, pRes.objID);
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
	    long parentLinkID = Utility.ParseLongOptionalHex(pRes.properties.get("parent-link"));
	    Link parent = links.get(parentLinkID);
	    long fromNodeID = Utility.ParseLongOptionalHex(pRes.properties.get("from-node"));
	    long toNodeID = Utility.ParseLongOptionalHex(pRes.properties.get("to-node"));
	    Node fromNode = nodes.get(fromNodeID);
	    Node toNode = nodes.get(toNodeID);
	    
	    //Ensure nodes exist
	    if (parent==null) {
	    	throw new IOException("Unknown Link id: " + Long.toHexString(parentLinkID));
	    }
	    if (fromNode==null) {
	    	throw new IOException("Unknown node id: " + Long.toHexString(fromNodeID));
	    }
	    if (toNode==null) {
	    	throw new IOException("Unknown node id: " + Long.toHexString(toNodeID));
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
		   // @amit:Not sure why to use Annotation 
		    /*
		    if (props.containsKey("aimsunn-id")) {
		    	Annotation an = new Annotation(new Point((int)x, (int)y), pRes.properties.get("aimsunn-id"), 'A');
		    	an.setBackgroundColor(Annotations_AimsunnBgColor);
		    	an.setBorderColor(Annotations_AimsunnFgColor);
		    	an.setFontColor(Annotations_FontColor);
		    	annot_aimsunn.add(an);
		    }
		    
		    if (props.containsKey("mitsimm-id")) {
		    	Annotation an = new Annotation(new Point((int)x, (int)y), pRes.properties.get("mitsimm-id"), 'A');
		    	an.setBackgroundColor(Annotations_MitsimmBgColor);
		    	an.setBorderColor(Annotations_MitsimmFgColor);
		    	an.setFontColor(Annotations_FontColor);
		    	annot_mitsimm.add(an);
		    }
		    */
		    
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
	    long fromSegmentKEY = Utility.ParseLongOptionalHex(pRes.properties.get("from-segment"));
	    long toSegmentKEY = Utility.ParseLongOptionalHex(pRes.properties.get("to-segment"));
	    long fromLane = Utility.ParseLongOptionalHex(pRes.properties.get("from-lane"));
	    long toLane = Utility.ParseLongOptionalHex(pRes.properties.get("to-lane"));
	    Segment fromSegment = segments.get(fromSegmentKEY);
	    Segment toSegment = segments.get(toSegmentKEY);

	    //Ensure segment exist
	    if (fromSegment==null) {
	    	throw new IOException("Unknown Segment id: " + Long.toHexString(fromSegmentKEY));
	    }
	    if (toSegment==null) {
	    	throw new IOException("Unknown Segment id: " + Long.toHexString(toSegmentKEY));
	    }
	    
	    
	    LaneConnector tempLaneConnector = new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane);
	    //Put into lane connector table
	    laneConnectors.put(pRes.objID, new LaneConnector(fromSegmentKEY, toSegmentKEY, fromLane, toLane));
	    collectSignalLineInfo(pRes.objID,tempLaneConnector);
	    
	    //Use from-segment and to-segment form a reference table, to check from-segment & to-segment pair against lane connector id
	    String fromToSegmentKey = Long.toHexString(fromSegmentKEY)+"&"+Long.toHexString(toSegmentKEY);
	    fromToSegmentRefTable.put(fromToSegmentKey, pRes.objID);

	    
	    
/*		System.out.println(fromSegmentKEY +"	" + toSegmentKEY);
		System.out.println(pRes.properties.get("from-segment") + "	" + pRes.properties.get("to-segment"));
		System.out.println();
*/				
	    if(segmentRefTable.containsKey(fromSegmentKEY)){
	    	segmentRefTable.get(fromSegmentKEY).add(pRes.objID);	
	    	segmentRefTable.get(fromSegmentKEY).add(toSegmentKEY);
	    
	    } else{
	    	ArrayList<Long> toSegmentList = new ArrayList<Long>();
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
	    long intersectionNodeID = Utility.ParseLongOptionalHex(pRes.properties.get("node"));
	    long linkVaID = Utility.ParseLongOptionalHex(pRes.properties.get("va"));
	    long linkVbID = Utility.ParseLongOptionalHex(pRes.properties.get("vb"));
	    long linkVcID = Utility.ParseLongOptionalHex(pRes.properties.get("vc"));
	    long linkVdID = Utility.ParseLongOptionalHex(pRes.properties.get("vd"));
	    long linkPaID = Utility.ParseLongOptionalHex(pRes.properties.get("pa"));
	    long linkPbID = Utility.ParseLongOptionalHex(pRes.properties.get("pb"));
	    long linkPcID = Utility.ParseLongOptionalHex(pRes.properties.get("pc"));
	    long linkPdID = Utility.ParseLongOptionalHex(pRes.properties.get("pd"));

	    ArrayList <Long>  tempLinkIDs = new ArrayList<Long>(
	    			Arrays.asList(linkVaID, linkVbID, linkVcID, linkVdID)); 
	    
	    ArrayList <Long> tempCrossingIDs = new ArrayList<Long>(
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
	
	private void collectSignalLineInfo(long objID, LaneConnector laneConnector){				
		if(lanes.containsKey(laneConnector.getFromSegment()) && lanes.containsKey(laneConnector.getToSegment()) ){
	
			long fromLaneNo = laneConnector.getFromLane();
			long toLaneNo = laneConnector.getToLane();
			Lane fromLane = lanes.get(laneConnector.getFromSegment()).get(fromLaneNo);
			Lane toLane = lanes.get(laneConnector.getToSegment()).get(toLaneNo);
			TrafficSignalLine tempSignalLine;
			if(Main.NEW_SIGNAL)
				tempSignalLine = new TrafficSignalLine(fromLane, toLane,null, -1); 
			else
				tempSignalLine = new TrafficSignalLine(fromLane, toLane); 
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
	private void populateIntersections(){

		
		for(Intersection intersection : intersections.values()){		
			ArrayList <Long> tempIntersectLinkIDs = intersection.getSigalLinkIDs();
			Hashtable<Integer, Long> intersectLinkSegmentIDTable = new Hashtable<Integer, Long>();
			
			long[] fromSegmentList = new long[]{-1,-1,-1,-1};
			long[] toSegmentList = new long[]{-1,-1,-1,-1};
			
			long intersectionNodeID = intersection.getIntersectNodeID(); 
						
			//Search all the Links
			for(int i = 0; i<tempIntersectLinkIDs.size();i++ ){	
				long tempLinkID = tempIntersectLinkIDs.get(i);
				//ArrayList<Integer> tempSegmentIDs = roadNetworkItemsMapTable.findSegmentIDWithLinkID(tempLinkID);
				ArrayList<Long> tempSegmentIDs = new ArrayList<Long>();
				
				Enumeration<Long> segmentKeys = segments.keys();
				while(segmentKeys.hasMoreElements()){
					
					Long segmentID = segmentKeys.nextElement();
					Segment tempSegment = segments.get(segmentID);
					long parentLinkID = tempSegment.getParent().getId();
				
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
						
						System.out.println("Error, no such segments in segment table "+Long.toHexString(tempSegmentIDs.get(j))+" -- RoadNetwork,populateIntersection ");
						
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
			ArrayList<Long> crossingIDs = intersection.getSigalCrossingIDs();
			ArrayList<TrafficSignalCrossing> crossingSignals =  new ArrayList<TrafficSignalCrossing>();
			long linkPaID = crossingIDs.get(0);	
			long linkPbID = crossingIDs.get(1);
			long linkPcID = crossingIDs.get(2);
			long linkPdID = crossingIDs.get(3);
			
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
	private void spaceBusStopAnnotations() {
		Hashtable<Point, Integer> alreadySpaced = new Hashtable<Point, Integer>(); //int = conflicts
		spaceBusStopAnnotations(alreadySpaced, annot_aimsun);
		spaceBusStopAnnotations(alreadySpaced, annot_mitsim);
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

	private void smoothSegmentJoins(ArrayList<Long> segmentIDs){
		if(segmentIDs.size() < 2)
			return;

		Long currentSegmentID = segmentIDs.get(0);

		LaneMarking currSegSidewalkLane1 = new LaneMarking(null, null, false, 0, 0L);
		LaneMarking currSegSidewalkLane2 = new LaneMarking(null, null, false, 0, 0L);


		for(int i = 0; i<segmentIDs.size();i++){	
			Long nextSegmentID = segmentIDs.get(i);
			if(segmentToLanesTable.containsKey(currentSegmentID) && segmentToLanesTable.containsKey(nextSegmentID))
			{
				Hashtable<Integer, Long> currentSegLanes = segmentToLanesTable.get(currentSegmentID);
				Hashtable<Integer, Long> nextSegLanes = segmentToLanesTable.get(nextSegmentID);
		
				LaneMarking nextSegSidewalkLane1 = new LaneMarking(null, null, false, 0, 0L);
				LaneMarking nextSegSidewalkLane2 = new LaneMarking(null, null, false, 0, 0L);

				boolean bFound = false;

				for(Long currSegLaneID : currentSegLanes.values())
				{
					Hashtable<Long,LaneMarking> currentSegLaneMarkTable = linaMarkings.get(currSegLaneID);
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

				for(Long nextSegLaneID : nextSegLanes.values())
				{
					Hashtable<Long,LaneMarking> nextSegLaneMarkTable = linaMarkings.get(nextSegLaneID);
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
					DPoint currSegStart1 = new DPoint(currSegSidewalkLane1.getStart().getUnscaledX(), currSegSidewalkLane1.getStart().getUnscaledY());
					DPoint currSegEnd1 = new DPoint(currSegSidewalkLane1.getEnd().getUnscaledX(), currSegSidewalkLane1.getEnd().getUnscaledY());
					DPoint currSegStart2 = new DPoint(currSegSidewalkLane2.getStart().getUnscaledX(), currSegSidewalkLane2.getStart().getUnscaledY());
					DPoint currSegEnd2 = new DPoint(currSegSidewalkLane2.getEnd().getUnscaledX(), currSegSidewalkLane2.getEnd().getUnscaledY());
			
					Vect currSidewalk1 = new Vect(currSegStart1.x, currSegStart1.y, currSegEnd1.x, currSegEnd1.y);
					Vect currSidewalk2 = new Vect(currSegStart2.x, currSegStart2.y, currSegEnd2.x, currSegEnd2.y);
			
					DPoint nextSegStart1 = new DPoint(nextSegSidewalkLane1.getStart().getUnscaledX(), nextSegSidewalkLane1.getStart().getUnscaledY());
					DPoint nextSegEnd1 = new DPoint(nextSegSidewalkLane1.getEnd().getUnscaledX(), nextSegSidewalkLane1.getEnd().getUnscaledY());
					DPoint nextSegStart2 = new DPoint(nextSegSidewalkLane2.getStart().getUnscaledX(), nextSegSidewalkLane2.getStart().getUnscaledY());
					DPoint nextSegEnd2 = new DPoint(nextSegSidewalkLane2.getEnd().getUnscaledX(), nextSegSidewalkLane2.getEnd().getUnscaledY());
					
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
	
	private Hashtable<Integer, ArrayList<ArrayList<TrafficSignalLine>>> helperAllocateDirection(long[] fromSegmentList, long [] toSegmentList){
		
		Hashtable<Integer, ArrayList<ArrayList<TrafficSignalLine>>> list = new Hashtable<Integer,ArrayList<ArrayList<TrafficSignalLine>>>();
		
		for(int i = 0;i<fromSegmentList.length;i++){
			
			
			ArrayList<ArrayList<TrafficSignalLine>> tempDirectionalSignalLines = new ArrayList<ArrayList<TrafficSignalLine>>();
			
			long fromSegmentKey = fromSegmentList[i]; 	
			if(segmentRefTable.containsKey(fromSegmentKey)){	
			
				ArrayList<Long> tempToSegmentList = segmentRefTable.get(fromSegmentKey);

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






