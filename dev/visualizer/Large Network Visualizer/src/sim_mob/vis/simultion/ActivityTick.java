package sim_mob.vis.simultion;

import java.awt.*;

import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Hashtable;
import sim_mob.vect.SimpleVectorImage;
import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * Activity "Agent Tick"
 * 
 * \author Vahid Saber
 */
public class ActivityTick extends AgentTick {
	 long myPersonId;
	 private static Font idFont = new Font("Arial", Font.PLAIN, 10);
	public ActivityTick(int id, double posX, double posY/*, int myPersonId_*/) {
		super(id);
		
		this.myPersonId = id/*myPersonId_*/;
		this.pos = new FlippedScaledPoint(posX, posY);

	}
	//Let's assume a car is 3m square?
	public Rectangle2D getBounds() {
		final double NODE_CM = 1*100; //1m square 
		double x = (pos.getUnscaledX()-NODE_CM/2);
		double y = pos.getUnscaledY()-NODE_CM/2;
//		System.out.println("Activity.Rectangle2D.Double(" + x +","+ y +","+NODE_CM+","+ NODE_CM +")");
//		System.out.println("pos.getUnscaledX() = " + pos.getUnscaledX());
//		System.out.println("pos.getUnscaledY() = " + pos.getUnscaledY());
		return new Rectangle2D.Double(
			pos.getUnscaledX()-NODE_CM/2,
			pos.getUnscaledY()-NODE_CM/2,
			NODE_CM, NODE_CM);
//		System.out.println("returning bound for activity");
//		return new Rectangle2D.Double(1,1, 0, 0);
	}
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_PEDESTIRAN;
	}
	
	public void draw(Graphics2D g, DrawParams params) {
//		System.out.println("Drawing the activity circle");
		
		AffineTransform oldAT = g.getTransform();
		//Draw a circle with its personId
		 {
			Point center = new Point((int)this.pos.getX(), (int)this.pos.getY());
			final int Size = 20;
			
			//Background
			g.setColor(Color.GREEN);
			g.fillRect(center.x-Size/2, center.y-Size/2, Size, Size);
//			g.fillOval(center.x-Size/2, center.y-Size/2, Size, Size);
			g.setColor(Color.CYAN);
			g.drawRect(center.x-Size/2, center.y-Size/2, Size, Size);
//			g.drawOval(center.x-Size/2, center.y-Size/2, Size, Size);
			
			//Text
			g.setColor(Color.WHITE);
			g.setFont(idFont);
			int strW = g.getFontMetrics().stringWidth("" + myPersonId);
			g.drawString(""+myPersonId, center.x-strW/2+1, center.y+4);
		}
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
	}
	
}



