package sim_mob.vis.simultion;

import java.awt.Color;
import java.util.ArrayList;
import java.util.Hashtable;

import sim_mob.vis.network.TrafficSignal.Phase;
import sim_mob.vis.network.TrafficSignalLine;

/**
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class SignalLineTick{
	private int id;
	private ArrayList<ArrayList<Integer>> allVehicleLights;
	private ArrayList<Integer> allPedestrianLights;	
	private Integer intersectionID;
//	private Phase [] phases; 
	private boolean fake;
	private int tempTick;
	private String tempPhase;
	//my solution:
	private Hashtable<String ,ArrayList<TrafficSignalLine>> TrafficSignalLines;	//String is phase (A,B,C,...)
	public Hashtable<String ,ArrayList<TrafficSignalLine>> getAllTrafficSignalLines(){ return TrafficSignalLines;}
	public Integer getIntersectionID(){ return intersectionID;}
	public ArrayList<ArrayList<Integer>> getVehicleLights(){ return allVehicleLights;}
	public ArrayList<Integer> getPedestrianLights(){return allPedestrianLights;}
	public boolean getFake(){ return fake; }
	public SignalLineTick(int id, ArrayList<ArrayList<Integer>> allVehicleLights, ArrayList<Integer> allPedestrainLights,Integer intersectionID){
		this.id = id;
		this.allVehicleLights = allVehicleLights;
		this.allPedestrianLights =  allPedestrainLights;
		this.intersectionID = intersectionID;
		this.fake = false;
	}
	
//	public SignalLineTick(int id,Phase[] phases_){
	public SignalLineTick(int id,Hashtable<String, ArrayList<TrafficSignalLine>> TrafficSignalLines_,int tempTick_, String tempPhase_){
		this.id = id;
		tempTick = tempTick_;
		tempPhase = tempPhase_;
		//at present we leverage the similarity of signal id and intersectionId.
		//In case one day they decided them to be different,this method should 
		//change along with the following items:
		//SimResLineParser class should add another member for road network,
		//this member should be initialized through its constructor 
		//this constructor is idally called from within simulationResult.LoadFileAndReport(..., RoadNetwork rn,...) method
		//which, as you can see, has the rn that can be supplied to SimResLineParser,
		//then SimResLineParser finds the corrsponding intersection,
		//intersection has all the necessary signalline objects necessary
		this.intersectionID = id;
		TrafficSignalLines = TrafficSignalLines_;
//		//debug		
		
		for(ArrayList<TrafficSignalLine> tsls1 : TrafficSignalLines.values())
		for(TrafficSignalLine tsl:tsls1)
		{
			if(!tsl.getPhaseName().equals("C")) continue;
			if(!((tempTick ==230)||(tempTick ==240)||(tempTick ==250))) continue;
			System.out.println("creating New SignalLineTick");
			if (tsl.getCurrColor() == Color.yellow)
				System.out.println("ID: " + id + "Tick " + tempTick + " color has been set to  yellow");
			else if (tsl.getCurrColor() == Color.green)
				System.out.println("ID: " + id + "Tick " + tempTick + " color has been set to green");
			else if (tsl.getCurrColor() == Color.red)
				System.out.println("ID: " + id + "Tick " + tempTick + " color has been set to red");
			System.out.println("New SignalLineTick............");
		}
		
//	//debug ends
//		this.phases = phases_;
		this.fake = false;
	}
	
	public int getID() {
		return id;
	}
	
	public void setItFake(){ 
		fake = true;
	}
//	public Phase[] getPhases()
//	{
//		return phases;
//	}
	
}
