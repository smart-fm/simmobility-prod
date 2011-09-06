package sim_mob;

import java.util.*;
//Simple class for visualizing an agent's state in one time tick.
public class AgentTick {
	public int agentID;
	public int agentType;
	public double agentX;
	public double agentY;
	public double carDir;
	public Hashtable<String,ArrayList<Integer>> phaseValue; 
	public int agentPedestrianStatus;
	
	public AgentTick()
	{
		phaseValue = new Hashtable<String,ArrayList<Integer>>();
	}
	
}
