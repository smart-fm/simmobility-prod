package sim_mob.vis.simultion;

import sim_mob.vis.network.RoadNetwork;

/**
 * Anything returned by Gson should implement this.  
 * 
 * \author Seth N. Hetu
 */
public interface GsonResObj  {
	
	///Return the time tick this object occurred at (0 for network objects). 
	///This is required for properly sorting objects.
	public abstract int getTimeTick();
	
	///Add this object to the result set. Implementations should use this method to
	/// update the Road Network and Simulation Results (assume that you have a singleton).
	public abstract void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes);
	
}
