package sim_mob.vis.simultion;

import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.IOException;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Driver "Agent Tick"
 */
public class PedestrianTick extends AgentTick {
	private static BufferedImage PedImg;
	static {
		try {
			PedImg = Utility.LoadImgResource("res/entities/person.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	} 
	
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */
	public PedestrianTick(double posX, double posY) {
		this.pos = new ScaledPoint(posX, posY);
	}
	
	public void draw(Graphics2D g) {
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//Translate to top-left corner
		at.translate(-PedImg.getWidth()/2, -PedImg.getHeight()/2);
		
		//Draw
		g.setTransform(at);
		g.drawImage(PedImg, 0, 0, null);
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
	}
}



