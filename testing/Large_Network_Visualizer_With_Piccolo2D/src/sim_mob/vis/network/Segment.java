package sim_mob.vis.network;

import java.awt.BasicStroke;

import java.awt.*;
import java.awt.geom.*;
import java.util.ArrayList;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Segment extends PPath {
	private static final long serialVersionUID = 1L;
	
	//Constants/Resources
	private static Color roadColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke roadStroke = new BasicStroke(2.0F);
	
	private Node from;
	private Node to;
	private int parentLinkID;
	private int segmentID;
	
	//Lane lines (lane edge lines). Can be "sealed" to prevent changes.
	private ArrayList<LaneMarking> laneEdges;
	private boolean laneEdgesSealed;
	public void addLaneEdge(int lineNumber, LaneMarking edge) {
		if (laneEdgesSealed) { throw new RuntimeException("Can't modify segment lanes; structure is sealed."); }
		
		//Expand the array as needed
		while (lineNumber >= laneEdges.size()) {
			laneEdges.add(null);
		}
		
		//Add, if unique
		if (laneEdges.get(lineNumber) != null) { throw new RuntimeException("Error: multiple lane edges specified for segment: " + segmentID); }
		laneEdges.set(lineNumber, edge);
	}
	public void sealLaneEdges() {
		if (!this.laneEdgesSealed) {
			Rectangle2D bounds = getBounds();  //More accurate bounds.
			for (LaneMarking lm : laneEdges) {
				if (lm==null) { throw new RuntimeException("Can't seal Segment; not all Lanes have been specified."); }
				bounds.add(lm.getBounds());
			}
			this.setBounds(bounds);
		}
		
		this.laneEdgesSealed = true; 
	}
	public int getNumLaneEdges() {
		return laneEdges.size();
	}
	public LaneMarking getLaneEdge(int id) {
		if (id<0) { id = laneEdges.size() + id; }
		return laneEdges.get(id);
	}
	
	

	
	public Segment(Link parent, Node from, Node to, int parentLinkID, int segmentID) {
		this.from = from;
		this.to = to;
		this.parentLinkID = parentLinkID;
		this.segmentID = segmentID;
		this.laneEdges = new ArrayList<LaneMarking>();
		this.laneEdgesSealed = false;
		
		this.setBounds(new Rectangle2D.Double(from.getX(), from.getY(), to.getX()-from.getX(), to.getY()-from.getY()));
		//this.setPathTo(new Line2D.Double(from.getX(), from.getY(), to.getX(), to.getY()));
	}
	
	public Node getFrom() { return from; }
	public Node getTo() { return to; }
	public int getparentLinkID(){ return parentLinkID;}
	public int getSegmentID() { return segmentID; }

	protected void paint(PPaintContext paintContext){
		Graphics2D g = paintContext.getGraphics();
		double oneMeterActual = 1 / paintContext.getScale(); 
		if (oneMeterActual<100) {
			paintLanes(paintContext);
		} else {
			paintSegmentLine(g, paintContext.getScale());
		}
	}
	
	private void paintSegmentLine(Graphics2D g, double contextScale) {
		Stroke str = roadStroke;
		if (str instanceof BasicStroke) {
			//We can scale the stroke too
			BasicStroke bs = (BasicStroke)(str);
			str = new BasicStroke((float)(bs.getLineWidth()/contextScale), bs.getEndCap(), bs.getLineJoin(), bs.getMiterLimit());
		}
		
		Line2D line = new Line2D.Double(from.getX(), from.getY(), to.getX(), to.getY());
		g.setColor(roadColor);
		g.setStroke(str);
		g.draw(line);
	}
	
	private void paintLanes(PPaintContext pc) {
		for (LaneMarking lnEdge : laneEdges) {
			lnEdge.paint(pc);
		}
	}



}
