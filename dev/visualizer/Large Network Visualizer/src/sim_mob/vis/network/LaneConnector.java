package sim_mob.vis.network;

import java.awt.Graphics2D;

import sim_mob.vis.controls.DrawableItem;

public class LaneConnector{

	private int fromSegment;
	private int toSegment;
	private int fromLane;
	private int toLane;
	
	private Node fromNode;
	private Node toNode;
	
	public int getFromSegment(){return fromSegment; }
	public int getToSegment(){return toSegment;}
	public int getFromLane(){return fromLane;}
	public int getToLane(){return toLane;}
	
	
	public LaneConnector(int fromSegment, int toSegment, int fromLane, int toLane){
		this.fromSegment = fromSegment;
		this.toSegment = toSegment;
		this.fromLane = fromLane;
		this.toLane = toLane;
		
	}
	
	
		
}
