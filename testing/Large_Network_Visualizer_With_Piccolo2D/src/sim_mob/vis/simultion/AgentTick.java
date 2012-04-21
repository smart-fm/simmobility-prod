package sim_mob.vis.simultion;

import java.awt.geom.Point2D;

public class AgentTick {
	
	//protected LocalPoint localPos;
	protected Point2D pos;
	//public LocalPoint getLocalPos() { return localPos; }
	public Point2D getPos() { return pos; }
	
	protected double angle;
	protected int type;
	public double getAngle(){	return angle;	}
	public int getType(){	return type; }
	
}
