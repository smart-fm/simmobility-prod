package sim_mob.vis.simultion;

import java.awt.BasicStroke;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;

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
	private static Stroke debugStr = new BasicStroke(1.0F);
	private static Color debugClr = new Color(0x00, 0x00, 0x66);
	private static final boolean DebugOn = true;
	
	private static BufferedImage CarImg;
	private static BufferedImage FakeCarImg;
	static {
		try {
			CarImg = Utility.LoadImgResource("res/entities/car.png");
			FakeCarImg = Utility.LoadImgResource("res/entities/fake_car.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	}

	
	private double angle;
	private boolean fake;
	public double getAngle() { return angle; } 
	public boolean getFake() { return fake; }
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */

/*	public DriverTick(double posX, double posY, double angle) {
		//this.pos = new ScaledPoint(posX, posY);

*/	
	public DriverTick(double posX, double posY, double angle, ScaledPointGroup spg) {
		this.pos = new ScaledPoint(posX, posY, spg);
		this.angle = angle;
		this.fake = false;
	}
	
	public void setItFake(){
		fake = true;
	}
	
	public void draw(Graphics2D g,double scale) {
	
//	public void draw(Graphics2D g) {
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();

		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//Rotate
		at.rotate((Math.PI*angle)/180);
		
		//Scale
		at.scale(1/scale + 0.2, 1/scale + 0.2);
		
		//Translate to top-left corner
		at.translate(-CarImg.getWidth()/2, -CarImg.getHeight()/2);

		//Set new transformation matrix
		g.setTransform(at);
		
		//Draw
		if(fake){
			g.drawImage(FakeCarImg, 0, 0, null);		
		}else{
			g.drawImage(CarImg, 0, 0, null);				
		}
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
		//Sample debug output
		if (DebugOn) {
			int sz = 10;
			int x = (int)pos.getX();
			int y = (int)pos.getY();
			g.setColor(debugClr);
			g.setStroke(debugStr);
			g.drawOval(x-sz, y-sz, 2*sz, 2*sz);
			g.drawLine(x-3*sz/2, y, x+3*sz/2,y);
			g.drawLine(x, y-3*sz/2, x, y+3*sz/2);
		}
		
	}
	

	
}



