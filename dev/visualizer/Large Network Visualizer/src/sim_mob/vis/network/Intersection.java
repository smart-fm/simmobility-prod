package sim_mob.vis.network;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;

//import sim_mob.vis.network.TrafficSignal.Crossing;
//import sim_mob.vis.network.TrafficSignal.Link;
//import sim_mob.vis.network.TrafficSignal.SignalHelper;
//import sim_mob.vis.network.TrafficSignal.Phase;
import sim_mob.vis.network.SignalHelper;


/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */

public class Intersection {

	private int intersectionID;
	private int intersectNodeID;
	private ArrayList <Integer> signalLinkIDs; 
	private ArrayList <Integer> signalCrossingIDs;
	private ArrayList <TrafficSignalCrossing> signalCrossings;
//	my solution:
	private SignalHelper signalHelper;
	
	private ArrayList<ArrayList<TrafficSignalLine>> vaSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vbSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vcSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vdSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
//	my solution:  key(String) will be the phase name.
	private Hashtable<String, ArrayList<TrafficSignalLine>> trafficSignalLines;
	private Hashtable<String, ArrayList <Integer>> trafficSignalCrossings;//will replace signalCrossingIDs
	
//	public int getIntersectNodeID (){return intersectNodeID;}
	public ArrayList <Integer> getSigalLinkIDs(){return signalLinkIDs;}//probabely useless coz it is used only in the old populate intersection
	public ArrayList <Integer> getSigalCrossingIDs(){return signalCrossingIDs;}
	public ArrayList <TrafficSignalCrossing> getSignalCrossings(){return signalCrossings;}
//	my solution:
	public SignalHelper getSignalHelper() {return signalHelper;}
	public void setSignalHelper(SignalHelper signalHelper_) { 
		signalHelper = signalHelper_; 
		}
	
	public int getIntersectNodeID (){return intersectNodeID;}
	public int getIntersectID (){return intersectionID;}
	
	public ArrayList<ArrayList<TrafficSignalLine>> getVaTrafficSignal(){return vaSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVbTrafficSignal(){return vbSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVcTrafficSignal(){return vcSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVdTrafficSignal(){return vdSignalLine;}
	//my solution
	public ArrayList<TrafficSignalLine> getPhaseTrafficSignalLines(String phase)
	{
		return trafficSignalLines.get(phase);
	}
	//since there can be 2 phases each having a trafficsignalline 
	//with identical combination of lanefrom-laneto, we need to 
	//locate the traficsignalline based on the phase also(usually current phase is our target search space)
	public TrafficSignalLine getTrafficSignalLine(String phase, Lane laneFrom, Lane laneTo)
	{
		ArrayList<TrafficSignalLine> TSLs = trafficSignalLines.get(phase);
		for(TrafficSignalLine tsl : TSLs)
		{
			if((laneFrom == tsl.getFromLane())&&(laneTo == tsl.getToLane()))
				return tsl;
		}
		return null;
	}
	public Hashtable<String, ArrayList<TrafficSignalLine>> getAllTrafficSignalLines()
	{
		return trafficSignalLines;
	}
	
	public Intersection(int intersectNodeID, ArrayList <Integer> signalLinkIDs, ArrayList <Integer> signalCrossingIDs){
		this.intersectNodeID = intersectNodeID;
		this.signalLinkIDs = signalLinkIDs;
		this.signalCrossingIDs = signalCrossingIDs;

	}
	public Intersection(SignalHelper signalHelper_) { 
		signalHelper = signalHelper_; 
		intersectNodeID = signalHelper.node; 
		intersectionID = signalHelper.hex_id;
	}

	public void setVaTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vaSignalLine){ this.vaSignalLine = vaSignalLine;}
	public void setVbTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vbSignalLine){this.vbSignalLine = vbSignalLine;}
	public void setVcTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vcSignalLine){this.vcSignalLine = vcSignalLine;}
	public void setVdTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vdSignalLine){this.vdSignalLine = vdSignalLine;}
	//my solution: phases replace approaches
	public void populateTrafficSignal(RoadNetwork rn)
	{
		Set<Integer> tempSet = new HashSet<Integer>(); //for temporary use only
		this.trafficSignalLines = new Hashtable<String, ArrayList<TrafficSignalLine>>();
		this.trafficSignalCrossings = new   Hashtable<String, ArrayList <Integer>>();
		
		for (SignalHelper.Phase ph : signalHelper.phases) {
			//step 1: populate trafficSignalLines
			ArrayList<TrafficSignalLine> tempPhaseTrafficSignalLine = new ArrayList<TrafficSignalLine>();
			// ph.trafficSignalLines = new ArrayList<TrafficSignalLine>(); I
			// guess here it is better to stick to the old design and keep the
			// trafficsignallines be hooked to intersection not signalhelper's
			// phase
			for (SignalHelper.Segment rs : ph.segments) {
				/*
				 * in redrawFrame->addAllLaneSignals->addTrafficLines
				 * only trafficsignallines pertaining to the first lane is drawn
				 * this is after a lengthy and still ambiguous way of populating a hash of a two dimensional araylist 
				 * of trafficsignallines objects(so much generate, so less used) 
				 * so, why not limit generating of trafficsignalline object right here at the beginning?
				 * this is only because I am willing to reuse trafficsignalline class
				 */
				Lane fromLane = rn.getLanes().get(rs.segment_from).get(0);
				Lane toLane = rn.getLanes().get(rs.segment_to).get(0);
//				System.out.println("Creatinf a trafficSignalLine with lanes " + fromLane.getStartMiddleNode().getPos().getX() + "  TO  " + toLane.getLaneNumber() );

				/*
				 * there is this container "trafficSignalLines" in thetrafficSignalLines
				 * roadnetwork which is used in helperAllocateDirection
				 * function. it could be a good source for referencing here but
				 * since 
				 * 1)its key is borrowed from lane-connector(which I don't
				 * have it here) 
				 * and 
				 * 2)...I have alzheimer, so can't remember
				 * what was the second reason :)..oh just remembered...forgot
				 * again...ok: helperAllocateDirection() is likely to be removed later...
				 * therfore I am not able to (re)use that container here, therefore I stick
				 * to phase based design(still reusing the concept of
				 * trafficSignalLine) and create
				 */
				TrafficSignalLine tempSignalLine = new TrafficSignalLine(fromLane, toLane);
				tempPhaseTrafficSignalLine.add(tempSignalLine);
				//and an additional book keeping for trafficsignalupdate
				rs.generatedTrafficSignalLine = tempSignalLine;
			}
			this.trafficSignalLines.put(ph.name, tempPhaseTrafficSignalLine);
			
			//step 2: populate signalCrossingIDs
			/*
			 * you may wonder why we create and keep trafficsignalline object in this class
			 * but when it comes to crossings, we just keep a record of signalCrossingIDs
			 * the reason is: 1) crossings trafficsignalcrossings in the roadnetwork class are generated 
			 * directly from output file, while trafficsignallines are indirectly generated as needed
			 * 2)the way trafficsignallines were created and used was very inefficient in terms of memory and search time.
			 * I am not saying trafficsignalcrossings are efficient. but since they are created based on the data which we dont have here,
			 * it is better to let its main container in the roadnetwork class remain as it is and we just book keep the crossing ids
			 * relative to this intersection.
			 */
			ArrayList <Integer> tempCrossingIds = new ArrayList <Integer>();
			for (SignalHelper.Crossing cr : ph.crossings) 
			{
//				if(!tempSet.contains(cr.id))//to avoid duplicates
//				{
					tempSet.add(cr.id);//to avoid duplicates. This container may be used to reuse signalCrossingIDs. this container is used in other part.so, just in case :)
					tempCrossingIds.add(cr.id);
//				}
			}
			this.trafficSignalCrossings.put(ph.name, tempCrossingIds);
		}
		//TODO: change the containers later so that we dont run into such a loop
		this.signalCrossingIDs = new ArrayList <Integer>();
		for(Integer s : tempSet) this.signalCrossingIDs.add(s);

	}
			
	public void setSignalCrossing (ArrayList <TrafficSignalCrossing> signalCrossings){
		this.signalCrossings = signalCrossings;
	}
	
}
