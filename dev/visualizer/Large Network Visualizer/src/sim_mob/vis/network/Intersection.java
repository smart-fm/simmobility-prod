package sim_mob.vis.network;

import java.util.ArrayList;

public class Intersection {

	private int intersectNodeID;
	private ArrayList <Integer> signalLinkIDs; 
	private ArrayList <Integer> signalCrossingIDs;
	private ArrayList <TrafficSignalCrossing> signalCrossings;

	
	private ArrayList<ArrayList<TrafficSignalLine>> vaSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vbSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vcSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();
	private ArrayList<ArrayList<TrafficSignalLine>> vdSignalLine  = new ArrayList<ArrayList<TrafficSignalLine>>();

	
	
	public int getIntersectNodeID (){return intersectNodeID;}
	public ArrayList <Integer> getSigalLinkIDs(){return signalLinkIDs;}
	public ArrayList <Integer> getSigalCrossingIDs(){return signalCrossingIDs;}
	public ArrayList <TrafficSignalCrossing> getSignalCrossings(){return signalCrossings;}
	
	public ArrayList<ArrayList<TrafficSignalLine>> getVaTrafficSignal(){return vaSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVbTrafficSignal(){return vbSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVcTrafficSignal(){return vcSignalLine;}
	public ArrayList<ArrayList<TrafficSignalLine>> getVdTrafficSignal(){return vdSignalLine;}
	
	public Intersection(int intersectNodeID, ArrayList <Integer> signalLinkIDs, ArrayList <Integer> signalCrossingIDs){
		
		this.intersectNodeID = intersectNodeID;
		this.signalLinkIDs = signalLinkIDs;
		this.signalCrossingIDs = signalCrossingIDs;

	}
	

	public void setVaTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vaSignalLine){
		this.vaSignalLine = vaSignalLine;
	}
	public void setVbTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vbSignalLine){
		this.vbSignalLine = vbSignalLine;
	}
	public void setVcTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vcSignalLine){
		this.vcSignalLine = vcSignalLine;
	}
	public void setVdTrafficSignal(ArrayList<ArrayList<TrafficSignalLine>> vdSignalLine){
		this.vdSignalLine = vdSignalLine;
	}
			
	public void setSignalCrossing (ArrayList <TrafficSignalCrossing> signalCrossings){
		this.signalCrossings = signalCrossings;
	}
	
}
