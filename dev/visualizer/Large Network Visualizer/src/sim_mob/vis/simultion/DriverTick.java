package sim_mob.vis.simultion;

import java.awt.BasicStroke;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;

import java.awt.geom.*;
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
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Anirudh Sivaraman
 */
public class DriverTick extends AgentTick {
	private static SimpleVectorImage CarImg;
	private static SimpleVectorImage BusImg;
	private static SimpleVectorImage TruckImg;
	
	private static SimpleVectorImage DebugCarImg;
	private static SimpleVectorImage DebugBusImg;
	private static SimpleVectorImage DebugTruckImg;
	
	private static SimpleVectorImage FakeCarImg;
	private static SimpleVectorImage FakeBusImg;
	private static SimpleVectorImage FakeTruckImg;
	
	protected static Stroke debugStr = new BasicStroke(1.0F);
	protected static Color debugClr = new Color(0x00, 0x00, 0x66);
	protected static Font idFont = new Font("Arial", Font.PLAIN, 10);
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
	//private static boolean isCar;
	
	public static class RxLocation {
		public double longitude;
		public double latitude;
	}
	
	
	private RxLocation msgLocation; //If null, display no message
	private int ID;
	private double angle;
	private boolean fake;
	private int length;
	private int width;
	private int pickNumber;
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
		this(posX, posY, angle, null, spg);
	}
	
	public DriverTick(double posX, double posY, double angle, RxLocation msgLocation, ScaledPointGroup spg) {
		this.pos = new ScaledPoint(posX, posY, spg);
		this.angle = angle;
		this.fake = false;
		//DriverTick.isCar = true;
		
		this.msgLocation = msgLocation;
		if(CarImg==null){
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
			
			CarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v1.json.txt"));
			BusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v2.json.txt"));
			TruckImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v3.json.txt"));
			
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("car", CarBkgdColorIDs, CarLineColorIDs);
		CarImg.buildColorIndex(overrides);
	}
	
	private static void MakeDebugCarImage() {
		//Load it.
		try {
			DebugCarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v1.json.txt"));
			DebugBusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v2.json.txt"));
			DebugTruckImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v3.json.txt"));
			
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("car-debug", CarBkgdColorIDs, CarLineColorIDs);
		DebugCarImg.buildColorIndex(overrides);
	}
	
	private static void MakeFakeCarImage() {
		//Load it.
		try {
			FakeCarImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v1.json.txt"));
			FakeBusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v2.json.txt"));
			FakeTruckImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/car_v3.json.txt"));

		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("car-fake", CarBkgdColorIDs, CarLineColorIDs);
		FakeCarImg.buildColorIndex(overrides);
		FakeCarImg.phaseColors(0xFF/2);

		FakeBusImg.buildColorIndex(overrides);
		FakeBusImg.phaseColors(0xFF/2);
		
		FakeTruckImg.buildColorIndex(overrides);
		FakeTruckImg.phaseColors(0xFF/2);
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
		
		//HACK: Modify the angle slightly based on the quadrant
		double angleD = angle;
		if (angleD<90) {
			//Moving down-right
			angleD -= 7;
		} else if (angle<180) {
			//Moving down-left
			angleD += 7;
		} else if (angle<270) {
			//Moving up-left
			angleD -= 7;
		} else if (angle<360) {
			//Moving up-right
			angleD += 7;
		}
		
		SimpleVectorImage svi = null;
		
		//Retrieve the image to draw

		
		if(this.length == 400){
			 svi = (drawFake&&fake) ? FakeCarImg : debug ? DebugCarImg : CarImg;		
		
		}else if(this.length == 1200){
			 svi = (drawFake&&fake) ? FakeBusImg : debug ? DebugBusImg : BusImg;		
			
		}else if(this.length == 1500){
			 svi = (drawFake&&fake) ? FakeTruckImg : debug ? DebugTruckImg : TruckImg;		
		}else{
			 svi = (drawFake&&fake) ? FakeCarImg : debug ? DebugCarImg : CarImg;
			 System.out.println("Error, No such length, use car image instead -- DriverTick, draw()");
		}
		
		BufferedImage toDraw = svi.getImage(scale, angleD, true);
		
		//Rotate
		//at.rotate((Math.PI*angle)/180);
		
		//Scale
		//at.scale(1/scale + 0.2, 1/scale + 0.2);
			
		//Translate to top-left corner
		//at.translate(-CarImg.getWidth()/2, -CarImg.getHeight()/2);
		at.translate(-toDraw.getWidth()/2, -toDraw.getHeight()/2);

		//Set new transformation matrix
		g.setTransform(at);
		
		//Draw the car
		g.drawImage(toDraw, 0, 0, null);
		
		//Draw its message
		if (msgLocation != null) {
			g.setBackground(Color.BLUE);
			g.clearRect(+6, -10, 170,20);
			//double roundedLong=((int)(msgLocation.longitude*100000))/100000;
			//double roundedLat=((int)(msgLocation.latitude*100000))/100000;
			String loc=String.format("%.5f,%.5f", msgLocation.longitude, msgLocation.latitude);
			g.drawString(loc, 10, 10);
		}
		
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



