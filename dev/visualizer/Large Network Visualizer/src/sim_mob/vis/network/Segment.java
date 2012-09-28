package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.util.Utility;

/**
 * Segments join Nodes together
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class Segment implements DrawableItem {
	//Constants/Resources
	private static Color roadColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke roadStroke = new BasicStroke(3.0F);
	
	private Link parent;
	//private int parentLinkID;
	private Node from;
	private Node to;
	public Segment(Link parent, Node from, Node to/*, int parentLinkID*/) {
		this.parent = parent;
		this.from = from;
		this.to = to;
		//this.parentLinkID = parentLinkID;
	}
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_SEGMENT;
	}
	
	
	//We define a Segment from its start node to its end node, with some buffer space (we don't 
	//  consider individual lanes right now).
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(from.getPos().getUnscaledX(), from.getPos().getUnscaledY(), 0, 0);
		res.add(to.getPos().getUnscaledX(), to.getPos().getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	
	
	public Node getFrom() { return from; }
	public Node getTo() { return to; }
	public Link getParent() { return parent; }
	//public int getparentLinkID(){ return parentLinkID;}
	
	public void draw(Graphics2D g, DrawParams params) {
		if (params.PastCriticalZoom) { return; }
		
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.drawLine((int)from.getPos().getX(), (int)from.getPos().getY(), (int)to.getPos().getX(), (int)to.getPos().getY());
	
		
	}
}
