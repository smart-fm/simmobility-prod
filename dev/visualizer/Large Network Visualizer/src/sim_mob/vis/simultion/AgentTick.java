package sim_mob.vis.simultion;

import sim_mob.vis.controls.DrawableItem;

import sim_mob.vis.network.basic.ScaledPoint;


/**
 * One agent's tick within the simulation
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public abstract class AgentTick implements DrawableItem {
	public AgentTick(long id) {
		this.id = id;
	}
	
	protected long id;
	protected ScaledPoint pos;
	public ScaledPoint getPos() { return pos; }
	public long getID() { return id; }
	
}
