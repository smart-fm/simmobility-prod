package sim_mob.vis.simultion;

import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.TrafficSignal.Phase;
import sim_mob.vis.simultion.GsonResObj;



/**
 * 
 * IMPORTANT NOTE: You might not have to subclass "DrawableItem". Instead, you might
 *                 have this class just "update" the traffic signal, which is what I 
 *                 think you were intending. Anyway, this is up to whoever develops the
 *                 visualization for this component. ~Seth
 *
 */
public class TrafficSignalUpdate implements DrawableItem, GsonResObj {
	private String hex_id;
	private Integer frame;
	private Phase[] phases;
	
	
	public void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes) {
		//TODO: Here is where you'd add this traffic signal to the road network.
		
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		//TODO: Here is where you'd draw the signal/update. 
		
	}
	
	public Rectangle2D getBounds() {
		//TODO: You need to estimate the size of the boundary for our spatial index.
		//      Check Node, Crossing, etc., to see how they do this.
		return new Rectangle2D.Double(0, 0, 1, 1);
	}

	
	public int getTimeTick() {
		return this.frame;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TRAFFIC_SIGNAL_UPDATE;
	}
	

}
