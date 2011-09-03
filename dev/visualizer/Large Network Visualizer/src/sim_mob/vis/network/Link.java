package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;

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
	
	public void draw(Graphics2D g) {
		g.setColor(roadColor);
		g.setStroke(roadStroke);
		g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY());
		
		
		
		//TODO: Where to put the name? Here, or on the individual Road Segments? 
	}
}
