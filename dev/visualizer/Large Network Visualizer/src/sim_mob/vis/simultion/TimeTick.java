package sim_mob.vis.simultion;

import java.util.Hashtable;

import sim_mob.vis.network.basic.ScaledPointGroup;

/**
 * One time tick of a simulation
 */
public class TimeTick {
	public Hashtable<Integer, AgentTick> agentTicks;
	public Hashtable<Integer, SignalTick> signalTicks;
	public ScaledPointGroup tickScaleGroup = new ScaledPointGroup();
}
