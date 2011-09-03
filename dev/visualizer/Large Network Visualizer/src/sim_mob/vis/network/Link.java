package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Links join Intersections and consist of Segments. 
 */
public class Link implements DrawableItem {
	//Constants/Resources
	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);
	private static Color roadNameColor = new Color(0x33, 0x33, 0x33);
	private static Color roadColor = new Color(0x33, 0x99, 0x22);
	private static Stroke roadStroke = new BasicStroke(1.0F);
	
	private String name;
	private Node start;
	private Node end;
	
	public Link(String name, Node start, Node end) {
		this.name = name;
		this.start = start;
		this.end = end;
	}
	
	public String getName() { return name; }
	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	
	public void draw(Graphics2D g) {
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
	}
	
	public void drawName(Graphics2D g) {
		g.setColor(roadNameColor);
		g.setFont(roadNameFont);
		float targetX = (float)(start.getPos().getX()+(end.getPos().getX()-start.getPos().getX())/2);
		float targetY = (float)(start.getPos().getY()+(end.getPos().getY()-start.getPos().getY())/2);
		
		//Move the center left
		int strWidth = g.getFontMetrics().stringWidth(name);
		targetX -= strWidth / 2.0F;
		
		//NOTE: We might want to center the font vertically too using getAscent().
		
		//TEMP: Try rotating it
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		AffineTransform oldTrans = g.getTransform();
		trans.rotate(-Math.PI/20);
		//trans.setToRotation(Math.PI/4); //45 deg.
		g.setTransform(trans);
		
		//Draw it.
		//g.drawString(name, targetX, targetY);
		g.drawString(name, 0, 0);
		
		g.setTransform(oldTrans);
	}
}


