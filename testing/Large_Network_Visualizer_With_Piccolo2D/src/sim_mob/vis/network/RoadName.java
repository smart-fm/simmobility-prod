package sim_mob.vis.network;

import java.awt.Font;
import java.awt.Graphics2D;

import sim_mob.vis.MainFrame;

import edu.umd.cs.piccolo.nodes.PText;
import edu.umd.cs.piccolo.util.PPaintContext;

public class RoadName extends PText{
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);
	private String name;
	private Node start;
	private Node end;
	
	public String getName() { return name; }

	public RoadName(String name, Node start, Node end){
		this.name = name;
		this.start = start;
		this.end = end;

		this.setText(name);
	}
	
	protected void paint(PPaintContext paintContext){
		
		Graphics2D g = paintContext.getGraphics();
		g.setColor(MainFrame.Config.getLineColor("roadname"));
		
		g.setFont(roadNameFont);
		
		float targetX = (float)(start.getLocalPos().getX()+(end.getLocalPos().getX()-start.getLocalPos().getX())/2);
		float targetY = (float)(start.getLocalPos().getY()+(end.getLocalPos().getY()-start.getLocalPos().getY())/2);
		/*
		//Move the center left
		float halfStrWidth = g.getFontMetrics().stringWidth(name) / 2.0F;
		//targetX -= strWidth / 2.0F;
		
		//Save the old translation matrix
		AffineTransform oldTrans = g.getTransform();
		
		//Create a new translation matrix which is located at the center of the string.
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		
		//Figure out the rotational matrix of this line, from start to end.
		Vect line = new Vect(start.getLocalPos().getX(), start.getLocalPos().getY(), end.getLocalPos().getX(), end.getLocalPos().getY());
		trans.rotate(line.getMagX(), line.getMagY());
		
		trans.translate(-halfStrWidth, -3);

		//Apply the transformation, draw the string at the origin.
		g.setTransform(trans);
		*/
		g.drawString(name, targetX, targetY);

		//Restore AffineTransform matrix.
		//g.setTransform(oldTrans);
	}
	
}
