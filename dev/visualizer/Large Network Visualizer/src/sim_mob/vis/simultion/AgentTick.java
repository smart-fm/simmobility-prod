package sim_mob.vis.simultion;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;


/**
 * One agent's tick within the simulation
 */
public abstract class AgentTick implements DrawableItem {
	protected ScaledPoint pos;
	public ScaledPoint getPos() { return pos; }
}
