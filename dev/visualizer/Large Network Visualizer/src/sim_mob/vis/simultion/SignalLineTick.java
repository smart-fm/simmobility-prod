package sim_mob.vis.simultion;

import java.awt.Color;
import java.awt.Graphics2D;
import java.util.ArrayList;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.Intersection;

public class SignalLineTick{
	
	private ArrayList<ArrayList<Integer>> allVehicleLights;
	private ArrayList<Integer> allPedestrianLights;	
	private Integer intersectionID;

	public Integer getIntersectionID(){ return intersectionID;}
	public ArrayList<ArrayList<Integer>> getVehicleLights(){ return allVehicleLights;}
	public ArrayList<Integer> getPedestrianLights(){return allPedestrianLights;}
	
	public SignalLineTick(ArrayList<ArrayList<Integer>> allVehicleLights, ArrayList<Integer> allPedestrainLights,Integer intersectionID){
		this.allVehicleLights = allVehicleLights;
		this.allPedestrianLights =  allPedestrainLights;
		this.intersectionID = intersectionID;
	}
	
}
