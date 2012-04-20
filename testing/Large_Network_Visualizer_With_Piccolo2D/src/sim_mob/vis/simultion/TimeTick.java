package sim_mob.vis.simultion;

import java.util.ArrayList;
import java.util.Hashtable;

import edu.umd.cs.piccolo.nodes.PPath;

/**
 * One time tick of a simulation
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class TimeTick {
	public Hashtable<Integer, AgentTick> agentTicks;
	public ArrayList<Integer> agentIDs;
	public Hashtable<Integer, CrossingLightTick> crossingLightTicks;
	/**
	 * Tracking ticks are agents of any type that "track" an agent with the same
	 * ID in agentTicks. 
	 */
	public Hashtable<Integer, AgentTick> trackingTicks;
	
	//public Hashtable<Integer, CrossingLightTick> getCrossingLightTick(){return this.crossingLightTicks;}
	//public Hashtable<Integer, SignalTick> signalTicks;
	//public Hashtable<Integer, SignalLineTick> signalLineTicks;
	
	
	public TimeTick() {
    	agentTicks = new Hashtable<Integer, AgentTick>();
    	agentIDs = new ArrayList<Integer>();
    	
		trackingTicks = new Hashtable<Integer,AgentTick>();
		
		crossingLightTicks = new Hashtable<Integer, CrossingLightTick>();
		
    	//signalTicks = new Hashtable<Integer, SignalTick>();
    	//signalLineTicks = new Hashtable<Integer,SignalLineTick>();
	}
}
