package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.Vect;

/**
 * Segments join Nodes together
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class LinkName implements DrawableItem {
	//Constants/Resources
	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);
	//private static Color roadNameColor = new Color(0x33, 0x33, 0x33);
	
	//private Link parent;
	//private int parentLinkID;
	//private Node from;
	//private Node to;
	private Link link;
	private String name;
	
	public LinkName(Link link, String name) {
		this.link = link;
		this.name = name;
	}
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_SEGMENT_NAMES;
	}
	
	
	//Just make it the same as the link, +3m per lane on each side. It's overestimating, but that
	//  doesn't really matter.
	public Rectangle2D getBounds() {
		//For now, just return the Link's boundary. We'll need to fix this later.
		return link.getBounds();
		
		/*final double BUFFER_CM = 1*100; //1m
		final double LANE_CM   = 3.5*100; //3.5m
		
		//We'll assume 4 lanes per segment.
		ScaledPoint start = link.getStart().getPos();
		ScaledPoint end = link.getEnd().getPos();
		Vect orig = new Vect(start.getUnscaledX(), start.getUnscaledY(), end.getUnscaledX(), end.getUnscaledY());
		Vect rLane = new Vect(orig);
		rLane.flipVecNormal(true);
		rLane.scaleVectTo(LANE_CM*4);
		Vect lLane = new Vect(orig);
		lLane.translateVect();
		lLane.flipVecNormal(false);
		lLane.scaleVectTo(LANE_CM*4);
		
		
		Rectangle2D res = new Rectangle2D.Double(orig.getX(), orig.getY(), 0, 0);
		res.add(orig.getEndX(), orig.getEndY());
		res.add(rLane.getEndX(), rLane.getEndY());
		res.add(lLane.getEndX(), lLane.getEndY());
		System.out.println(res.getWidth() + " => " + res.getHeight());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;*/
	}
	
	
	
	public void draw(Graphics2D g, DrawParams params) {
		g.setColor(MainFrame.Config.getLineColor("roadname"));
		g.setFont(roadNameFont);
		float targetX = (float)(link.getStart().getPos().getX()+(link.getEnd().getPos().getX()-link.getStart().getPos().getX())/2);
		float targetY = (float)(link.getStart().getPos().getY()+(link.getEnd().getPos().getY()-link.getStart().getPos().getY())/2);
		
		//Move the center left
		float halfStrWidth = g.getFontMetrics().stringWidth(name) / 2.0F;
		//targetX -= strWidth / 2.0F;
		
		//Save the old translation matrix
		AffineTransform oldTrans = g.getTransform();
		
		//Create a new translation matrix which is located at the center of the string.
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		
		//Figure out the rotational matrix of this line, from start to end.
		Vect line = new Vect(link.getStart().getPos().getX(), link.getStart().getPos().getY(), link.getEnd().getPos().getX(), link.getEnd().getPos().getY());
		trans.rotate(line.getMagX(), line.getMagY());
		
		//Next, translate X backwards by half the string width, and move it up slightly.
		//TODO: This is not a good way of doing things! We have scaled points for a reason; just put it at the 
		//      end of the lane line! ~Seth
		if(!params.PastCriticalZoom) {
			trans.translate(-halfStrWidth, -3);
		} else { 
			trans.translate(-halfStrWidth, -3 *1.6*4);
		}

		//Apply the transformation, draw the string at the origin.
		g.setTransform(trans);
		g.drawString(name, 0, 0);

		//Restore AffineTransform matrix.
		g.setTransform(oldTrans);
	}
}
