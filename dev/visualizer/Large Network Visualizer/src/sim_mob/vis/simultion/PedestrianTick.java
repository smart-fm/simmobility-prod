package sim_mob.vis.simultion;

import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.IOException;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.network.basic.ScaledPointGroup;
import sim_mob.vis.network.basic.Vect;
import sim_mob.vis.util.Utility;


/**
 * Driver "Agent Tick"
 */
public class PedestrianTick extends AgentTick {
	private static Stroke debugStr = new BasicStroke(1.0F);
	private static Color debugClr = new Color(0x00, 0x00, 0x66);
	private static final boolean DebugOn = true;

	
	private static BufferedImage PedImg;
	private static BufferedImage FakePedImg;
	
	static {
		try {
			PedImg = Utility.LoadImgResource("res/entities/person.png");
			FakePedImg = Utility.LoadImgResource("res/entities/fake_person.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	} 
	private boolean fake;
	
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */
	public PedestrianTick(double posX, double posY, ScaledPointGroup spg) {
		this.pos = new ScaledPoint(posX, posY, spg);
		this.fake  = false;
	}
	
	public void setItFake(){
		fake = true;
	}

	public void draw(Graphics2D g, double scale) {

		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		
		//Scale
		at.scale(1/scale + 0.2, 1/scale + 0.2);
		
		//Translate to top-left corner
		at.translate(-PedImg.getWidth()/2, -PedImg.getHeight()/2);
		
		//Draw
		g.setTransform(at);

		if(fake){
			g.drawImage(FakePedImg, 0, 0, null);
		}else{
			g.drawImage(PedImg, 0, 0, null);
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
	
	public void draw(Graphics2D g, double scale, boolean drawFake){
		
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//Scale
		at.scale(1/scale + 0.2, 1/scale + 0.2);
		
		//Translate to top-left corner
		at.translate(-PedImg.getWidth()/2, -PedImg.getHeight()/2);
		
		//Draw
		g.setTransform(at);

		//Enable Draw fake agent
		if(drawFake)
		{
			if(fake){
				g.drawImage(FakePedImg, 0, 0, null);
			}else{
				g.drawImage(PedImg, 0, 0, null);
			}

		}else{
			g.drawImage(PedImg, 0, 0, null);
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



