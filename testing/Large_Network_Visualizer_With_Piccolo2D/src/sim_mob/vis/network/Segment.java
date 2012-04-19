package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Line2D;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Segment extends PPath{

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	//Constants/Resources
	private static Color roadColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke roadStroke = new BasicStroke(0.2F);

	private Link parent;
	private int parentLinkID;
	private Node from;
	private Node to;
	private Shape line;

	
	public Segment(Link parent, Node from, Node to, int parentLinkID) {
		this.parent = parent;
		this.from = from;
		this.to = to;
		this.parentLinkID = parentLinkID;
		
		this.setPathTo(new Line2D.Double(0,0,0,0));
	}
	
	public Node getFrom() { return from; }
	public Node getTo() { return to; }
	public Link getParent() { return parent; }
	public int getparentLinkID(){ return parentLinkID;}

	protected void paint(PPaintContext paintContext){
		line = new Line2D.Double(from.getLocalPos().getX(), from.getLocalPos().getY(), to.getLocalPos().getX(), to.getLocalPos().getY());	
		this.setPathTo(line);
	
		Graphics2D g = paintContext.getGraphics();
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.draw(line);
	}



}
