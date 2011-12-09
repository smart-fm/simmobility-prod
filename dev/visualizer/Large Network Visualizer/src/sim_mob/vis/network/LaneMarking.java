package sim_mob.vis.network;

import java.awt.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawableItem;

public class LaneMarking implements DrawableItem{
	//Constants/Resources
	//private static Color laneColor = new Color(0x00, 0x00, 0x00);
	//private static Color sideWalkColor = new Color(0x84, 0x70, 0xff);
	//private static Stroke laneStroke = new BasicStroke(1.0F);
	
	private Integer parentSegment;
	private Node start;
	private Node end;

	
	private boolean isSideWalk;
	private int laneNumber;

	public LaneMarking(Node start, Node end, boolean isSideWalk, int lineNumber, Integer parentSegment) {
		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;

		
	}

	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Integer getParentSegment(){ return parentSegment; }
	


	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	@Override
	public void draw(Graphics2D g) {
		//Retrieve
		Color clr = MainFrame.Config.getLineColor(isSideWalk?"sidewalk":"lane");
		Stroke strk = MainFrame.Config.getLineStroke(isSideWalk?"sidewalk":"lane");
		
		//Override for lane line zero
		if (laneNumber==0) {
			clr = MainFrame.Config.getLineColor("median");
			strk = MainFrame.Config.getLineStroke("median");
		}
		
		//Draw it.
		g.setColor(clr);
		g.setStroke(strk);
		g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY());
	}

	
}
