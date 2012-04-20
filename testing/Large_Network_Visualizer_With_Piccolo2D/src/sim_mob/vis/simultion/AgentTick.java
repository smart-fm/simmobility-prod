package sim_mob.vis.simultion;

import java.awt.geom.Point2D;

import sim_mob.vis.network.basic.LocalPoint;
import edu.umd.cs.piccolo.nodes.PPath;

public class AgentTick{
	
	//protected LocalPoint localPos;
	protected Point2D pos;
	//public LocalPoint getLocalPos() { return localPos; }
	public Point2D getPos() { return pos; }
	
	protected double angle;
	protected int type;
	public double getAngle(){	return angle;	}
	public int getType(){	return type; }
	
}
