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
	public int getparentLinkID(){ return parentLinkID;}

	protected void paint(PPaintContext paintContext){
		Graphics2D g = paintContext.getGraphics();
		double oneMeterActual = 1 / paintContext.getScale(); 
		if (oneMeterActual<100) {
			paintLanes(g, paintContext.getScale());
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
	
	private void paintLanes(Graphics2D g, double contextScale) {
		//TEMP
		g.setColor(Color.blue);
		g.setStroke(new BasicStroke(1.0F));
		Line2D line = new Line2D.Double(from.getX(), from.getY(), to.getX(), to.getY());
		g.draw(line);
	}



}
