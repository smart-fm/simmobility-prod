package sim_mob.vis.simultion;

import sim_mob.vis.network.basic.LocalPoint;
import edu.umd.cs.piccolo.nodes.PPath;

public class AgentTick{
	
	protected LocalPoint localPos;
	protected double angle;
	protected int type;
	public LocalPoint getLocalPos() { return localPos; }
	public double getAngle(){	return angle;	}
	public int getType(){	return type; }
	
}
