package sim_mob.vis.simultion;

import java.awt.Graphics2D;

import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.IOException;

import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.network.basic.ScaledPointGroup;
import sim_mob.vis.util.Utility;


/**
 * Driver "Agent Tick"
 */
public class DriverTick extends AgentTick {
	private static BufferedImage CarImg;
	static {
		try {
			CarImg = Utility.LoadImgResource("res/entities/car.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	}
	
	private double angle;
	public double getAngle() { return angle; } 
	
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */
	public DriverTick(double posX, double posY, double angle, ScaledPointGroup spg) {
		this.pos = new ScaledPoint(posX, posY, spg);
		this.angle = angle;
	}
	
	
	public void draw(Graphics2D g) {
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//Rotate
		at.rotate((Math.PI*angle)/180);
		
		//Translate to top-left corner
		at.translate(-CarImg.getWidth()/2, -CarImg.getHeight()/2);
		
		//Draw
		g.setTransform(at);
		g.drawImage(CarImg, 0, 0, null);
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
	}
}



