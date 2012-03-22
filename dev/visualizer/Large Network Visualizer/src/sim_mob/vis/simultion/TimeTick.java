package sim_mob.vis.simultion;

import java.util.Hashtable;

import sim_mob.vis.network.basic.ScaledPointGroup;

/**
 * One time tick of a simulation
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class TimeTick {
	public Hashtable<Integer, AgentTick> agentTicks;
	
	/**
	 * Tracking ticks are agents of any type that "track" an agent with the same
	 * ID in agentTicks. 
	 */
	public Hashtable<Integer, AgentTick> trackingTicks;
	
	public Hashtable<Integer, SignalTick> signalTicks;
	public Hashtable<Integer, SignalLineTick> signalLineTicks;
	public ScaledPointGroup tickScaleGroup = new ScaledPointGroup();
	
	
	public TimeTick() {
    	trackingTicks = new Hashtable<Integer, AgentTick>();
    	agentTicks = new Hashtable<Integer, AgentTick>();
    	signalTicks = new Hashtable<Integer, SignalTick>();
    	signalLineTicks = new Hashtable<Integer,SignalLineTick>();
	}
}
