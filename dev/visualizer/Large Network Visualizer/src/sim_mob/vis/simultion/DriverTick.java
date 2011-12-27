package sim_mob.vis.simultion;

import java.awt.BasicStroke;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;

import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Hashtable;

import sim_mob.vect.SimpleVectorImage;
import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.network.basic.ScaledPointGroup;
import sim_mob.vis.util.Utility;


/**
 * Driver "Agent Tick"
 */
public class DriverTick extends AgentTick {
	private static SimpleVectorImage CarImg;
	private static SimpleVectorImage DebugCarImg;
	private static SimpleVectorImage FakeCarImg;
	
	private static Stroke debugStr = new BasicStroke(1.0F);
	private static Color debugClr = new Color(0x00, 0x00, 0x66);
	private static Font idFont = new Font("Arial", Font.PLAIN, 10);
	private static final String[] CarBkgdColorIDs = new String[] {
		"body1", "body2", "window1", "window2", "wheel"
	};
	private static final String[] CarLineColorIDs = new String[] {
		"body-outline", "window-outline", "wheel-outline"
	};

	/*private static BufferedImage CarImg;
	private static BufferedImage FakeCarImg;
	static {
		try {
			CarImg = Utility.LoadImgResource("res/entities/car.png");
			FakeCarImg = Utility.LoadImgResource("res/entities/fake_car.png");
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	}*/
	private int ID;
	private double angle;
	private boolean fake;
	private int length;
	private int width;
	public int getID(){return ID;}
	public int getLength(){return length;}
	public int getWidth() {return width;}
	public double getAngle() { return angle; } 
	public boolean getFake() { return fake; }
	/**
	 * NOTE: Here is where we start to see some inefficiencies with our ScaledPoint implementation.
	 *       When we re-scale, every car on every time tick has its position scaled. We should 
	 *       limit this to the current frame, and then continue to scale frames as they arrive. 
	 */


	public DriverTick(double posX, double posY, double angle, ScaledPointGroup spg) {
		this.pos = new ScaledPoint(posX, posY, spg);
		this.angle = angle;
		this.fake = false;
		
		//Init resources?
		if (CarImg==null) {
			MakeCarImage();
		}
		if (DebugCarImg==null) {
			MakeDebugCarImage();
		}
		if (FakeCarImg==null) {
			MakeFakeCarImage();
		}
	}
	
	private static void MakeCarImage() {
		//Load it.
		try {
			CarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = GetOverrides("car");
		CarImg.buildColorIndex(overrides);
	}
	
	private static void MakeDebugCarImage() {
		//Load it.
		try {
			DebugCarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = GetOverrides("car-debug");
		DebugCarImg.buildColorIndex(overrides);
	}
	
	private static void MakeFakeCarImage() {
		//Load it.
		try {
			FakeCarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = GetOverrides("car-fake");
		FakeCarImg.buildColorIndex(overrides);
		FakeCarImg.phaseColors(0xFF/2);
	}
	
	private static Hashtable<String, Color> GetOverrides(String prefix) {
		Hashtable<String, Color> overrides = new Hashtable<String, Color>();
		for (String id : CarBkgdColorIDs) {
			Color clr = MainFrame.Config.getBackground(prefix+"-"+id);
			if (clr!=null) {
				overrides.put(id, clr);
			}
		}
		for (String id : CarLineColorIDs) {
			Color clr = MainFrame.Config.getLineColor(prefix+"-"+id);
			if (clr!=null) {
				overrides.put(id, clr);
			}
		}
		return overrides;
	}
	
	public void setItFake(){
		fake = true;
	}
	public void setLenth(int length){
		this.length = length;
	}
	public void setWidth(int width){
		this.width = width;
	}
	public void setID(int id){
		this.ID = id;
	}
	
	
	public void draw(Graphics2D g,double scale, boolean drawFake,boolean debug){
		AffineTransform oldAT = g.getTransform();

		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//Retrieve the image to draw
		SimpleVectorImage svi = (drawFake&&fake) ? FakeCarImg : debug ? DebugCarImg : CarImg;
		BufferedImage toDraw = svi.getImage(1/scale + 0.2, (int)angle);
		
		//Rotate
		//at.rotate((Math.PI*angle)/180);
		
		//Scale
		//at.scale(1/scale + 0.2, 1/scale + 0.2);
			
		//Translate to top-left corner
		//at.translate(-CarImg.getWidth()/2, -CarImg.getHeight()/2);
		at.translate(-toDraw.getWidth()/2, -toDraw.getHeight()/2);

		//Set new transformation matrix
		g.setTransform(at);
		
		//Draw with fake agent enabled
		/*if(drawFake){
			if(fake){
				//g.drawImage(FakeCarImg, 0, 0, null);
			}else{
				//g.drawImage(CarImg, 0, 0, null);
			}
		} else {*/
			g.drawImage(toDraw, 0, 0, null);
			//g.drawImage(CarImg, 0, 0, null);
		//}
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
		//Sample debug output
		if (debug) {
			int sz = 12;
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
		
		g.setColor(Color.GREEN);
		g.setFont(idFont);
		g.setStroke(new BasicStroke(0.5F));
		
		String id = Integer.toString(ID);
		g.drawString(id, 0, 0);

		//Restore AffineTransform matrix.
		g.setTransform(oldTrans);
		
	}
	
	
}



