package sim_mob.vis.simultion;

import java.util.ArrayList;

public class SignalLineTick{
	
	private ArrayList<ArrayList<Integer>> allVehicleLights;
	private ArrayList<Integer> allPedestrianLights;	
	private Integer intersectionID;
	private boolean fake;
	
	public Integer getIntersectionID(){ return intersectionID;}
	public ArrayList<ArrayList<Integer>> getVehicleLights(){ return allVehicleLights;}
	public ArrayList<Integer> getPedestrianLights(){return allPedestrianLights;}
	public boolean getFake(){ return fake; }
	public SignalLineTick(ArrayList<ArrayList<Integer>> allVehicleLights, ArrayList<Integer> allPedestrainLights,Integer intersectionID){
		this.allVehicleLights = allVehicleLights;
		this.allPedestrianLights =  allPedestrainLights;
		this.intersectionID = intersectionID;
		this.fake = false;
	}
	
	public void setItFake(){ 
		fake = true;
	}
	
}
