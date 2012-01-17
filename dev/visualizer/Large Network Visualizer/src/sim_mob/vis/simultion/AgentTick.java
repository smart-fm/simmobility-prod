package sim_mob.vis.simultion;

import sim_mob.vis.controls.DrawableAgent;
import sim_mob.vis.network.basic.ScaledPoint;


/**
 * One agent's tick within the simulation
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public abstract class AgentTick implements DrawableAgent {
	protected ScaledPoint pos;
	public ScaledPoint getPos() { return pos; }
	
}
