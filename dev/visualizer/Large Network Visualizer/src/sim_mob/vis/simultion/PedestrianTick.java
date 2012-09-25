package sim_mob.vis.simultion;

import java.awt.BasicStroke;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Hashtable;
import java.util.Random;

import sim_mob.vect.SimpleVectorImage;
import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Driver "Agent Tick"
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class PedestrianTick extends AgentTick {
	private static Stroke debugStr = new BasicStroke(1.0F);
	private static Color debugClr = new Color(0x00, 0x00, 0x66);
	private static Font idFont = new Font("Arial", Font.PLAIN, 10);

	private static SimpleVectorImage PersonImg;
	private static SimpleVectorImage FakePersonImg;
	private static SimpleVectorImage DebugPersonImg;
	
	private static final String[] PedBkgdColorIDs = new String[] {
		"body", "body-head", "touch-up-2"
	};
	private static final String[] PedLineColorIDs = new String[] {
		"body-outline", "touch-up-1", "touch-up-3", "touch-up-4"
	};
	
	//private static BufferedImage PedImg;
	//private static BufferedImage FakePedImg;
	
	/*static {
		try {
			PedImg = Utility.LoadImgResource("res/entities/person.png");
			FakePedImg = Utility.LoadImgResource("res/entities/fake_person.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	} */
	private boolean fake;

	
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */
	public PedestrianTick(long id, double posX, double posY) {
		super(id);
		
		this.pos = new FlippedScaledPoint(posX, posY);
		this.fake  = false;
		
		if (PersonImg==null) {
			MakePersonImage();
		}
		if (FakePersonImg==null) {
			MakeFakePersonImage();
		}
		if (DebugPersonImg==null) {
			MakeDebugPersonImage();
		}
	}
	
	//Let's assume a person is 1m square?
	public Rectangle2D getBounds() {
		final double NODE_CM = 1*100; //1m square 
		return new Rectangle2D.Double(
			pos.getUnscaledX()-NODE_CM/2,
			pos.getUnscaledY()-NODE_CM/2,
			NODE_CM, NODE_CM);
	}
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_PEDESTIRAN;
	}
	
	
	private static void MakePersonImage() {
		//Load it.
		try {
			PersonImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/person.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("pedestrian", PedBkgdColorIDs, PedLineColorIDs);
		PersonImg.buildColorIndex(overrides);
	}
	
	private static void MakeFakePersonImage() {
		//Load it.
		try {
			FakePersonImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/person.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("pedestrian-fake", PedBkgdColorIDs, PedLineColorIDs);
		FakePersonImg.buildColorIndex(overrides);
		FakePersonImg.phaseColors(0xFF/2);
	}
	
	private static void MakeDebugPersonImage() {
		//Load it.
		try {
			DebugPersonImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/person.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		// Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("pedestrian-debug", PedBkgdColorIDs, PedLineColorIDs);
		DebugPersonImg.buildColorIndex(overrides);
	}
	
	
	public void setItFake(){
		fake = true;
	}

	
	//private static Random r = new Random();
	
	public void draw(Graphics2D g, DrawParams params) {
	//}
	//public void draw(Graphics2D g, double scaleMultiplier, boolean drawFake, boolean debug, Point2D size100Percent){
		
		
		
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		//AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		AffineTransform at = new AffineTransform(oldAT);
		at.translate(pos.getX(), pos.getY());
		
		//Scale
		//at.scale(1/scale + 0.2, 1/scale + 0.2);
		
		//TEMP
		// double scaleMultiplier = Math.max(ScaledPoint.getScaleFactor().getX(), ScaledPoint.getScaleFactor().getY());
		double onePixelInM = 10; //Assume pixels are 15m
		
		SimpleVectorImage svi = (params.DrawFakeOn&&fake) ? FakePersonImg : params.DebugOn ? DebugPersonImg : PersonImg;
		double scaleMultiplier = (ScaledPoint.getScaleFactor().getX()*onePixelInM);
		//BufferedImage toDraw = svi.getImage(scaleMultiplier, angleD, true);
		
		//System.out.println("Scale multiplier: " + scaleMultiplier + " => " + (1/scaleMultiplier + 0.2));
		// scaleMultiplier=2;
		//Retrieve the image to draw
		
		//BufferedImage toDraw = svi.getImage(1/scaleMultiplier + 0.2, 0);
		BufferedImage toDraw = svi.getImage(scaleMultiplier,0);
		
		//Translate to top-left corner
		at.translate(-toDraw.getWidth()/2, -toDraw.getHeight()/2);
		
		//Draw
		g.setTransform(at);

		//Enable Draw fake agent
		/* if(drawFake)
		   {
			if(fake){
				g.drawImage(FakePedImg, 0, 0, null);
			}else{
				g.drawImage(PedImg, 0, 0, null);
			}

		}else{*/
			g.drawImage(toDraw, 0, 0, null);
			//g.drawImage(toDraw, r.nextInt(20)-10, r.nextInt(20)-10, null);
		//}

		//Restore old transformation matrix
		g.setTransform(oldAT);	
		
		
		//Sample debug output
		if (params.DebugOn) {
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
		
		String id = Long.toString(getID());
		g.drawString(id, 0, 0);

		//Restore AffineTransform matrix.
		g.setTransform(oldTrans);
		
	}


}



