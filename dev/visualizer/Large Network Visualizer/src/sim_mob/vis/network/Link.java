package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.util.Utility;

/**
 * Links join Intersections and consist of Segments.
 * 
 *  \author Seth N. Hetu
 *  \author Zhang Shuai
 *  \author Matthew Bremer Bruchon
 */
public class Link implements DrawableItem {
	//Constants/Resources
	//private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);
	//private static Color roadNameColor = new Color(0x33, 0x33, 0x33);
	private static Color roadColor = new Color(0x33, 0x99, 0x22);
	private static Stroke roadStroke = new BasicStroke(1.0F);
	
	private String name;
	private Node start;
	private Node end;
	private long id;
	private ArrayList<Long> fwdPathSegmentIDs;
	private ArrayList<Long> revPathSegmentIDs;
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_LINK;
	}
	
	public long getId() { return id; }
	
	
	public Link(String name, Node start, Node end, long id) {
		this.id = id;
		this.name = name;
		this.start = start;
		this.end = end;
	}
	
	//See Segment for discussion 
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(getStart().getPos().getUnscaledX(), getStart().getPos().getUnscaledY(), 0, 0);
		res.add(getEnd().getPos().getUnscaledX(), getEnd().getPos().getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	
	
	public String getName() { return name; }
	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	
	//Retrieve an "authoritative" road name. If two Links have the same authoritative road
	// name, it means their start and end points are the same (possibly reversed) and the 
	// name of the road itself is the same.
	public String getAuthoritativeRoadName() {
		return getName() + ":" + NodeNameHelper(getStart(), getEnd());
	}
	private static final String NodeNameHelper(Node n1, Node n2) {
		//A little messy, but good enough for now.
		if (n1==null && n2==null) { return "<null>:<null>"; }
		Node smaller = null;
		Node larger = null;
		if (n1==null) {
			smaller = n1; 
			larger = n2;
		} else if (n2==null) {
			smaller = n2; 
			larger = n1;
		} else {
			smaller = n1.hashCode()<n2.hashCode() ? n1 : n2;
			larger = n1.hashCode()>n2.hashCode() ? n1 : n2;
		}		
		return (smaller!=null?smaller.hashCode():"<null>") + ":" + (larger!=null?larger.hashCode():"<null>");
	}
	
	public ArrayList<Long> getFwdPathSegmentIDs() { return fwdPathSegmentIDs; }
	public ArrayList<Long> getRevPathSegmentIDs() { return revPathSegmentIDs; }
	public void setFwdPathSegmentIDs(ArrayList<Long> segIDs) { fwdPathSegmentIDs = segIDs; }
	public void setRevPathSegmentIDs(ArrayList<Long> segIDs) { revPathSegmentIDs = segIDs; }
	
	public void draw(Graphics2D g, DrawParams params) {
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
	}
	

}


