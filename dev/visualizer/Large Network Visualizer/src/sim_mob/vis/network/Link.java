package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.AffineTransform;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.Vect;

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
		float halfStrWidth = g.getFontMetrics().stringWidth(name) / 2.0F;
		//targetX -= strWidth / 2.0F;
		
		//Save the old translation matrix
		AffineTransform oldTrans = g.getTransform();
		
		//Create a new translation matrix which is located at the center of the string.
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		
		//Figure out the rotational matrix of this line, from start to end.
		Vect line = new Vect(start.getPos().getX(), start.getPos().getY(), end.getPos().getX(), end.getPos().getY());
		trans.rotate(line.getMagX(), line.getMagY());
		
		//Next, translate X backwards by half the string width, and move it up slightly.
		trans.translate(-halfStrWidth, -3);
		
		//Apply the transformation, draw the string at the origin.
		g.setTransform(trans);
		g.drawString(name, 0, 0);

		//Restore AffineTransform matrix.
		g.setTransform(oldTrans);
	}
}


