package sim_mob.vis.simultion;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
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
public class PedestrianTick extends AgentTick {
	private static Stroke debugStr = new BasicStroke(1.0F);
	private static Color debugClr = new Color(0x00, 0x00, 0x66);
	private static Font idFont = new Font("Arial", Font.PLAIN, 10);

	
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
	private int ID;
	public int getID(){return ID;}

	
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
	public void setID(int id){
		this.ID = id;
	}

	public void draw(Graphics2D g, double scale, boolean drawFake, boolean debug){
		
		
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
		if (debug) {
			int sz = 3;
			int x = (int)pos.getX();
			int y = (int)pos.getY();
			g.setColor(debugClr);
			g.setStroke(debugStr);
			g.drawOval(x-sz, y-sz, 2*sz, 2*sz);
			g.drawLine(x-3*sz/2, y, x+3*sz/2,y);
			g.drawLine(x, y-3*sz/2, x, y+3*sz/2);
			
			//drawString(g);
		}
	}
	public void drawString(Graphics2D g)
	{
		//Save old transformation.
		AffineTransform oldTrans = g.getTransform();
		
		float targetX = (float)(pos.getX());
		float targetY = (float)(pos.getY());
		
		//Create a new translation matrix which is located at the center of the string.
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		//Apply the transformation, draw the string at the origin.
		g.setTransform(trans);
		
		g.setColor(Color.RED);
		g.setFont(idFont);
		g.setStroke(new BasicStroke(0.5F));
		
		String id = Integer.toString(ID);
		g.drawString(id, 0, 0);

		//Restore AffineTransform matrix.
		g.setTransform(oldTrans);
		
	}


}



