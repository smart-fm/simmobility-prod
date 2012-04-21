package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Path2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;
import java.util.ArrayList;
import sim_mob.vis.MainFrame;
import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.util.PPaintContext;


public class LaneMarking extends PNode {
	private static final long serialVersionUID = 1L;

	private Integer parentSegment;
	private boolean isSideWalk;
	private int laneNumber;
	
	//Some overlap here...
	private Path2D line;
	private ArrayList<Point2D> all_points;
	

	/**
	 * 
	 * @param points List of [x1,y1, x2,y2, ...]
	 * @param isSideWalk
	 * @param lineNumber
	 * @param parentSegment
	 */
	public LaneMarking(ArrayList<Integer> points, boolean isSideWalk, int lineNumber, Integer parentSegment){
		if (points.size()<4) { throw new RuntimeException("Lane marking requires at least two points (four x/y values)."); }
		if (points.size()%2 != 0) { throw new RuntimeException("Lane marking points array must contain an even number of points."); }

		//Add the line.
		Rectangle2D bounds = null; 
		this.line = new Path2D.Double();
		this.all_points = new ArrayList<Point2D>();
		for (int i=0; i<points.size()/2; i++) {
			double x = points.get(i*2);
			double y = points.get(i*2+1);
			this.all_points.add(new Point2D.Double(x, y));
			if (i==0) {
				bounds = new Rectangle2D.Double(x, y, 1, 1);
				this.line.moveTo(x, y);
			} else {
				bounds.add(new Point2D.Double(x, y));
				this.line.lineTo(x, y);
			}
			this.setBounds(bounds);
		}
		
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;
		
		//Set the bounds.
		setBounds(points.get(0), points.get(1), points.get(2)-points.get(0), points.get(3)-points.get(1));
	}

	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Integer getParentSegment(){ return parentSegment; }
	
	public Point2D getPoint(int id) {
		//Negative indices are ok.
		if (id<0) {
			id = all_points.size() + id;
		}
		
		return all_points.get(id);
	}
	
	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	protected void paint(PPaintContext paintContext){
		Color clr = MainFrame.Config.getLineColor(isSideWalk?"sidewalk":"lane");
		Stroke str = MainFrame.Config.getLineStroke(isSideWalk?"sidewalk":"lane");
		
		//Override for lane line zero
		if (laneNumber==0) {
			clr = MainFrame.Config.getLineColor("median");
			str = MainFrame.Config.getLineStroke("median");
		}
		if (str instanceof BasicStroke) {
			//We can scale the stroke too
			BasicStroke bs = (BasicStroke)(str);
			str = new BasicStroke((float)(bs.getLineWidth()/paintContext.getScale()), bs.getEndCap(), bs.getLineJoin(), bs.getMiterLimit());
		}
		
		Graphics2D g = paintContext.getGraphics();
		g.setStroke(str);
		g.setColor(clr);
		g.draw(this.line);

	}	
}
