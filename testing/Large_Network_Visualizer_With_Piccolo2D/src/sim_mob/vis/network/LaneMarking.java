package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Line2D;

import sim_mob.vis.MainFrame;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class LaneMarking extends PPath{


	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	private Shape line;
	
	private Integer parentSegment;
	private Node start;
	private Node end;

	
	private boolean isSideWalk;
	private int laneNumber;

	public LaneMarking(Node start, Node end, boolean isSideWalk, int lineNumber, Integer parentSegment){	
		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;
		
		this.setPathTo(new Line2D.Double(0,0,0,0));
		//System.out.println(this.getBounds().toString());
		
		//repaint(); //Shouldn't need a repaint immediately.

	}
	
	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Integer getParentSegment(){ return parentSegment; }
	
	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	protected void paint(PPaintContext paintContext){
		line = new Line2D.Double(start.getLocalPos().getX(),start.getLocalPos().getY(),end.getLocalPos().getX(),end.getLocalPos().getY());
		this.setPathTo(line);
		
		Color clr = MainFrame.Config.getLineColor(isSideWalk?"sidewalk":"lane");
		Stroke strk = MainFrame.Config.getLineStroke(isSideWalk?"sidewalk":"lane");
		
		//Override for lane line zero
		if (laneNumber==0) {
			clr = MainFrame.Config.getLineColor("median");
			strk = MainFrame.Config.getLineStroke("median");
		}
	
        Graphics2D g = paintContext.getGraphics();
		g.setStroke(new BasicStroke(0.2F));
		
		g.setColor(clr);
		g.setStroke(strk);
		/*
		if(isSideWalk)
	        g.setColor(Color.red);
		else
	        g.setColor(Color.black);
		*/
		g.draw(line);

	}	
}
