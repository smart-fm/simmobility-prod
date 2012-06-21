package sim_mob.vis.network;

import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.simultion.GsonResObj;
import sim_mob.vis.simultion.SimulationResults;

public class TrafficSignal implements DrawableItem, GsonResObj {
	private class Link {
		private String link_from;
		private String link_to;
		
		//This will be "null" for TrafficSignal; it will be set properly in "TrafficSignalUpdate"
		private Integer current_color;  
	}
	
	private class Crossing {
		private String id;
		
		//This will be "null" for TrafficSignal; it will be set properly in "TrafficSignalUpdate"
		private Integer current_color;
	}
	
	public class Phase {
		private String name;
		private Link[] links;
		private Crossing[] crossings;
	}
	
	private String hex_id;
	private String simmob_id;
	private String node;
	private Phase[] phases;
	
	
	public void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes) {
		//TODO: Here is where you'd add this traffic signal to the road network.

		//Something like this....
		
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		//TODO: Here is where you'd draw the signal. 
		
	}
	
	public Rectangle2D getBounds() {
		//TODO: You need to estimate the size of the boundary for our spatial index.
		//      Check Node, Crossing, etc., to see how they do this.
		return new Rectangle2D.Double(0, 0, 1, 1);
	}

	
	public int getTimeTick() {
		return 0; //All network items have time tick 0.
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TRAFFIC_SIGNAL;
	}
}
