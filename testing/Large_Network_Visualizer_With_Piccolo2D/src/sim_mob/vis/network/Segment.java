package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PBounds;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Segment extends PPath {
	private static final long serialVersionUID = 1L;
	
	//Constants/Resources
	private static Color roadColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke roadStroke = new BasicStroke(2.0F);
	
	private Node from;
	private Node to;
	private int parentLinkID;

	
	public Segment(Link parent, Node from, Node to, int parentLinkID) {
		this.from = from;
		this.to = to;
		this.parentLinkID = parentLinkID;
		
		this.setBounds(new Rectangle2D.Double(from.getX(), from.getY(), to.getX()-from.getX(), to.getY()-from.getY()));
		//this.setPathTo(new Line2D.Double(from.getX(), from.getY(), to.getX(), to.getY()));
	}
	
	public Node getFrom() { return from; }
	public Node getTo() { return to; }
	//public Link getParent() { return parent; }
	public int getparentLinkID(){ return parentLinkID;}

	protected void paint(PPaintContext paintContext){
		//line = new Line2D.Double(from.getX(), from.getY(), to.getX(), to.getY());	
		//this.setPathTo(line);
	
		Graphics2D g = paintContext.getGraphics();
		
		Stroke str = roadStroke;
		if (str instanceof BasicStroke) {
			//We can scale the stroke too
			BasicStroke bs = (BasicStroke)(str);
			str = new BasicStroke((float)(bs.getLineWidth()/paintContext.getScale()), bs.getEndCap(), bs.getLineJoin(), bs.getMiterLimit());
		}
		
		PBounds bnd = getBounds();
		Line2D line = new Line2D.Double(bnd.getX(), bnd.getY(), bnd.getX()+bnd.getWidth(), bnd.getY()+bnd.getHeight());
		g.setColor(roadColor);
		g.setStroke(str);
		g.draw(line);
	}



}
