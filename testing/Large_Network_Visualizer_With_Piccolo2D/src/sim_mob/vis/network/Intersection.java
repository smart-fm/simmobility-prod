package sim_mob.vis.network;

import java.util.ArrayList;
import java.util.Hashtable;


/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class Intersection {
	private int intersectNodeID;
	private Hashtable<Character, Integer> signalLinkIDs; //e.g., 'a' => XXX 
	private ArrayList <Integer> signalCrossingIDs;
	private ArrayList <TrafficSignalCrossing> signalCrossings;

	
	private RoadNetwork.DirectionHelper vaSignalLine = new RoadNetwork.DirectionHelper();
	private RoadNetwork.DirectionHelper vbSignalLine = new RoadNetwork.DirectionHelper();
	private RoadNetwork.DirectionHelper vcSignalLine = new RoadNetwork.DirectionHelper();
	private RoadNetwork.DirectionHelper vdSignalLine = new RoadNetwork.DirectionHelper();

	
	
	public int getIntersectNodeID (){return intersectNodeID;}
	public Hashtable<Character, Integer> getSigalLinkIDs(){return signalLinkIDs;}
	public ArrayList <Integer> getSigalCrossingIDs(){return signalCrossingIDs;}
	public ArrayList <TrafficSignalCrossing> getSignalCrossings(){return signalCrossings;}
	
	public RoadNetwork.DirectionHelper getVaTrafficSignal(){return vaSignalLine;}
	public RoadNetwork.DirectionHelper getVbTrafficSignal(){return vbSignalLine;}
	public RoadNetwork.DirectionHelper getVcTrafficSignal(){return vcSignalLine;}
	public RoadNetwork.DirectionHelper getVdTrafficSignal(){return vdSignalLine;}
	
	public Intersection(int intersectNodeID, ArrayList <Integer> signalLinkIDs, ArrayList <Integer> signalCrossingIDs){
		
		this.intersectNodeID = intersectNodeID;
		
		this.signalLinkIDs = new Hashtable<Character, Integer>();
		for (int linkID : signalLinkIDs) {
			char charKey = (char)((int)'a'+this.signalLinkIDs.size());
			this.signalLinkIDs.put(charKey, linkID);
		}
				
		
		this.signalCrossingIDs = signalCrossingIDs;

	}
	

	public void setVaTrafficSignal(RoadNetwork.DirectionHelper vaSignalLine) {
		this.vaSignalLine = vaSignalLine;
	}
	public void setVbTrafficSignal(RoadNetwork.DirectionHelper vbSignalLine){
		this.vbSignalLine = vbSignalLine;
	}
	public void setVcTrafficSignal(RoadNetwork.DirectionHelper vcSignalLine){
		this.vcSignalLine = vcSignalLine;
	}
	public void setVdTrafficSignal(RoadNetwork.DirectionHelper vdSignalLine){
		this.vdSignalLine = vdSignalLine;
	}
			
	public void setSignalCrossing (ArrayList <TrafficSignalCrossing> signalCrossings){
		this.signalCrossings = signalCrossings;
	}
	
}
