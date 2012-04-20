package sim_mob.vis.simultion;

import java.util.ArrayList;
import java.util.Hashtable;

public class CrossingLightTick {
	//Signal Location
	private int location;
	private ArrayList<Integer> crossingIDs;
	private ArrayList<Integer> crossingLights;
	
	public int getSignalLocation(){return this.location;}
	public ArrayList<Integer> getCrossingIDs(){return this.crossingIDs;}
	public ArrayList<Integer> getCrossingLights(){return this.crossingLights;}
	
	
	public CrossingLightTick(int location, ArrayList<Integer> crossingIDs, ArrayList<Integer> crossingLights){
		this.location = location;
		this.crossingIDs = crossingIDs;
		this.crossingLights = crossingLights;	
	}
	
	
}
