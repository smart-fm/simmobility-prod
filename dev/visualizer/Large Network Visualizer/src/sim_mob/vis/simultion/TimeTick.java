package sim_mob.vis.simultion;

import java.util.Hashtable;


/**
 * One time tick of a simulation
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class TimeTick {
	public Hashtable<Long, AgentTick> agentTicks;
	
	/**
	 * Tracking ticks are agents of any type that "track" an agent with the same
	 * ID in agentTicks. 
	 */
	public Hashtable<Long, AgentTick> trackingTicks;
	
	//public Hashtable<Integer, SignalTick> signalTicks;
	public Hashtable<Long, SignalLineTick> signalLineTicks;
	
	
	public TimeTick() {
    	trackingTicks = new Hashtable<Long, AgentTick>();
    	agentTicks = new Hashtable<Long, AgentTick>();
    	//signalTicks = new Hashtable<Integer, SignalTick>();
    	signalLineTicks = new Hashtable<Long,SignalLineTick>();
	}
}
