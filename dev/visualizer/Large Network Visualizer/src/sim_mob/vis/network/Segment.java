package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Segments join Nodes together
 */
public class Segment implements DrawableItem {
	//Constants/Resources
	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);
	private static Color roadNameColor = new Color(0x33, 0x33, 0x33);
	private static Color roadColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke roadStroke = new BasicStroke(3.0F);
	
	private Link parent;
	private Node from;
	private Node to;
	
	public Segment(Link parent, Node from, Node to) {
		this.parent = parent;
		this.from = from;
		this.to = to;
	}
	
	public Node getFrom() { return from; }
	public Node getTo() { return to; }
	public Link getParent() { return parent; }
	
	public void draw(Graphics2D g) {
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.drawLine((int)from.getPos().getX(), (int)from.getPos().getY(), (int)to.getPos().getX(), (int)to.getPos().getY());
	}
	
	public void drawName(Graphics2D g) {
		//For now, just pull the name from the parent
		String name = parent.getName();
		g.setColor(roadNameColor);
		g.setFont(roadNameFont);
		float targetX = (float)(from.getPos().getX()+(to.getPos().getX()-from.getPos().getX())/2);
		float targetY = (float)(from.getPos().getY()+(to.getPos().getY()-from.getPos().getY())/2);
		
		//Move the center left
		int strWidth = g.getFontMetrics().stringWidth(name);
		targetX -= strWidth / 2.0F;
		
		//NOTE: We might want to center the font vertically too using getAscent(). 
		
		//Draw it.
		g.drawString(name, targetX, targetY);
	}
}
