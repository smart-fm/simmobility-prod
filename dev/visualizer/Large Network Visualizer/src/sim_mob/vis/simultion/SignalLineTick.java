package sim_mob.vis.simultion;

import java.awt.Color;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import sim_mob.vis.network.TrafficSignalLine;

/**
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Vahid Saber
 */
public class SignalLineTick{
	private int id;
	private ArrayList<ArrayList<Integer>> allVehicleLights;
	private ArrayList<Integer> allPedestrianLights;	
	private Integer intersectionID;
//	private Phase [] phases; 
	private boolean fake;
	
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
	
	public int getID() {
		return id;
	}
	
	public void setItFake(){ 
		fake = true;
	}
	
	//my solution:
	private Hashtable<String ,ArrayList<TrafficSignalLine>> TrafficSignalLines;	//String is phase (A,B,C,...)
	private HashMap<TrafficSignalLine, Color> TrafficSignalLines_Map;
	private HashMap<Integer, Integer> CrossingID_Map;
	public Hashtable<String ,ArrayList<TrafficSignalLine>> getAllTrafficSignalLines(){ return TrafficSignalLines;}
	public HashMap<TrafficSignalLine, Color> getAllTrafficSignalLines_Map(){ return TrafficSignalLines_Map;}
	public HashMap<Integer, Integer> getCrossingID_Map(){ return CrossingID_Map;}

	public SignalLineTick(int id,ArrayList<TrafficSignalLine> TrafficSignalLines_,HashMap<Integer, Integer> CrossingIDs_map,int tempTick_, String tempPhase_){
		this.id = id;
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
//		this.TrafficSignalLines = new Hashtable<String, ArrayList<TrafficSignalLine>>(); let's see if it work just with pointers without copying
//		this.TrafficSignalLines = TrafficSignalLines_;//memory deficiency
		TrafficSignalLines_Map = new HashMap<TrafficSignalLine, Color>();
		for(TrafficSignalLine tsl : TrafficSignalLines_)
		{
//			System.out.println("Current color " + tsl.getCurrColor());
			TrafficSignalLines_Map.put(tsl, tsl.getCurrColor());
		}
		
		CrossingID_Map = new HashMap<Integer, Integer>();
		for(Integer i:CrossingIDs_map.keySet())
		{
//			if((tempTick == 230)&& tempPhase.equals("D"))
//				System.out.println("Tick 230 Phase D Setting crossing " + i + "  color to " + CrossingIDs_map.get(i));
			CrossingID_Map.put(i, CrossingIDs_map.get(i));
		}
		
//	//debug ends
//		this.phases = phases_;
		this.fake = false;
	}
	


	
}
