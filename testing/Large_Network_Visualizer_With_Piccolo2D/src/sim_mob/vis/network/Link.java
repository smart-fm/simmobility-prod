package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Line2D;
import java.util.ArrayList;


import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Link extends PPath{
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private static Color roadColor = new Color(0x33, 0x99, 0x22);
	private static Stroke roadStroke = new BasicStroke(0.2F);
	
	private String name;
	private Node start;
	private Node end;
	private Shape link;
	
	private ArrayList<Integer> fwdPathSegmentIDs;
	private ArrayList<Integer> revPathSegmentIDs;


	public Link(String name, Node start, Node end) {
		this.name = name;
		this.start = start;
		this.end = end;

		setPathTo(new Line2D.Double(0,0,0,0));
		
	}
	
	public String getName() { return name; }
	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	
	public ArrayList<Integer> getFwdPathSegmentIDs() { return fwdPathSegmentIDs; }
	public ArrayList<Integer> getRevPathSegmentIDs() { return revPathSegmentIDs; }
	public void setFwdPathSegmentIDs(ArrayList<Integer> segIDs) { fwdPathSegmentIDs = segIDs; }
	public void setRevPathSegmentIDs(ArrayList<Integer> segIDs) { revPathSegmentIDs = segIDs; }
	
	
	protected void paint(PPaintContext paintContext){

		//Set New Path
		link = new Line2D.Double(start.getLocalPos().getX(),start.getLocalPos().getY(),end.getLocalPos().getX(),end.getLocalPos().getY());
		setPathTo(link);
		
		Graphics2D g = paintContext.getGraphics();
		
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.draw(link); 	
		
	}
	
}
