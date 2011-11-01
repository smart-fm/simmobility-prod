package sim_mob.vis.network;

import java.awt.Graphics2D;
import java.util.ArrayList;
import java.util.Hashtable;

import sim_mob.vis.controls.DrawableItem;

public class Intersection implements DrawableItem{
	
	public Node intersectionNode;
	public ArrayList<Integer> intersectionSegmentsID;
	public Crossing intersectionCrossing;
	public Hashtable<Integer, ArrayList<Lane>> intersectionLanes;

	public Intersection(){
		
		
	}
	
	
	@Override
	public void draw(Graphics2D g) {
		// TODO Auto-generated method stub
		
	}

	
	

}
