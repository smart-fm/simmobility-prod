package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 */
public class LaneMarking implements DrawableItem{
	//Constants/Resources
	//private static Color laneColor = new Color(0x00, 0x00, 0x00);
	//private static Color sideWalkColor = new Color(0x84, 0x70, 0xff);
	//private static Stroke laneStroke = new BasicStroke(1.0F);
	
	private Integer parentSegment;
	private Node start;
	private Node end;
	private ScaledPoint startPt;
	private ScaledPoint secondPt;
	private ScaledPoint penultimatePt; //second to last
	private ScaledPoint lastPt;
	
	private boolean isSideWalk;
	private int laneNumber;

	public LaneMarking(Node start, Node end, boolean isSideWalk, int lineNumber, Integer parentSegment) {
		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;

		if(start != null)
		{
			this.startPt = start.getPos();
			this.secondPt = start.getPos();			
		}
		if(end != null)
		{
			this.penultimatePt = end.getPos();
			this.lastPt = end.getPos();
		}
	}
	
	
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(start.getPos().getUnscaledX(), start.getPos().getUnscaledY(), 0, 0);
		res.add(end.getPos().getUnscaledX(), end.getPos().getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	

	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Integer getParentSegment(){ return parentSegment; }

	public void setStartPt(ScaledPoint pt){ startPt = pt; }
	public void setSecondPt(ScaledPoint pt){ secondPt = pt; }
	public void setPenultimatePt(ScaledPoint pt){ penultimatePt = pt; }
	public void setLastPt(ScaledPoint pt){ lastPt = pt; }

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
		g.drawLine((int)startPt.getX(),(int)startPt.getY(),(int)secondPt.getX(),(int)secondPt.getY());
		g.drawLine((int)secondPt.getX(),(int)secondPt.getY(),(int)penultimatePt.getX(),(int)penultimatePt.getY());
		g.drawLine((int)penultimatePt.getX(),(int)penultimatePt.getY(), (int)lastPt.getX(),(int)lastPt.getY());
	}

	
}
