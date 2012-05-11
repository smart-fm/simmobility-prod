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

	
	private SetOfTurnings vaSignalLine = new SetOfTurnings();
	private SetOfTurnings vbSignalLine = new SetOfTurnings();
	private SetOfTurnings vcSignalLine = new SetOfTurnings();
	private SetOfTurnings vdSignalLine = new SetOfTurnings();

	
	
	public int getIntersectNodeID (){return intersectNodeID;}
	public Hashtable<Character, Integer> getSigalLinkIDs(){return signalLinkIDs;}
	public ArrayList <Integer> getSigalCrossingIDs(){return signalCrossingIDs;}
	public ArrayList <TrafficSignalCrossing> getSignalCrossings(){return signalCrossings;}
	
	public SetOfTurnings getVaTrafficSignal(){return vaSignalLine;}
	public SetOfTurnings getVbTrafficSignal(){return vbSignalLine;}
	public SetOfTurnings getVcTrafficSignal(){return vcSignalLine;}
	public SetOfTurnings getVdTrafficSignal(){return vdSignalLine;}
	
	public Intersection(int intersectNodeID, ArrayList <Integer> signalLinkIDs, ArrayList <Integer> signalCrossingIDs){
		
		this.intersectNodeID = intersectNodeID;
		
		this.signalLinkIDs = new Hashtable<Character, Integer>();
		for (int linkID : signalLinkIDs) {
			char charKey = (char)((int)'a'+this.signalLinkIDs.size());
			this.signalLinkIDs.put(charKey, linkID);
		}
				
		
		this.signalCrossingIDs = signalCrossingIDs;

	}
	

	public void setVaTrafficSignal(SetOfTurnings vaSignalLine) {
		this.vaSignalLine = vaSignalLine;
	}
	public void setVbTrafficSignal(SetOfTurnings vbSignalLine){
		this.vbSignalLine = vbSignalLine;
	}
	public void setVcTrafficSignal(SetOfTurnings vcSignalLine){
		this.vcSignalLine = vcSignalLine;
	}
	public void setVdTrafficSignal(SetOfTurnings vdSignalLine){
		this.vdSignalLine = vdSignalLine;
	}
			
	public void setSignalCrossing (ArrayList <TrafficSignalCrossing> signalCrossings){
		this.signalCrossings = signalCrossings;
	}
	
}
