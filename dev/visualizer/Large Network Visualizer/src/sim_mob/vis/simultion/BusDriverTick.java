package sim_mob.vis.simultion;

import java.awt.*;

import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Hashtable;
import sim_mob.vect.SimpleVectorImage;
import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * BusDriver "Agent Tick"
 * 
 * \author Seth N. Hetu
 */
public class BusDriverTick extends DriverTick {
	private static SimpleVectorImage TempBusImg;
	private static SimpleVectorImage TempBusImg_flip;
	private static SimpleVectorImage DebugTempBusImg;
	private static SimpleVectorImage DebugTempBusImg_flip;
	private static SimpleVectorImage FakeTempBusImg;
	private static SimpleVectorImage FakeTempBusImg_flip;
	
	
	//Later: custom colors
	private static final String[] TmpBusBkgdColorIDs = new String[] {
		"body", "body-lower", "lower-stripe", "upper-stripe", "light", "window"
	};
	private static final String[] TmpBusLineColorIDs = new String[] {
		"body-outline", "window-outline"
	};


	//The only field unique to BusDrivers (for now
	private int passengerCount;

	
	public BusDriverTick(int id, double posX, double posY, double angle, int passengerCount) {
		super(id, posX, posY, angle, null);
		this.passengerCount = passengerCount;

		//Make our images
		if(TempBusImg==null){
			MakeTempBusImg();
		}
		if (DebugTempBusImg==null) {
			MakeDebugTempBusImg();
		}
		if (FakeTempBusImg==null) {
			MakeFakeTempBusImg();
		}

	}
	
	private static void MakeTempBusImg() {
		//Load it.
		try {
			TempBusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus.json.txt"));
			TempBusImg_flip = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus_flip.json.txt"));
			
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("tmp-bus", TmpBusBkgdColorIDs, TmpBusLineColorIDs);
		TempBusImg.buildColorIndex(overrides);
	}
	
	private static void MakeDebugTempBusImg() {
		//Load it.
		try {
			DebugTempBusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus.json.txt"));
			DebugTempBusImg_flip = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus_flip.json.txt"));
			
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("tmp-bus-debug", TmpBusBkgdColorIDs, TmpBusLineColorIDs);
		DebugTempBusImg.buildColorIndex(overrides);
	}
	
	private static void MakeFakeTempBusImg() {
		//Load it.
		try {
			FakeTempBusImg = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus.json.txt"));
			FakeTempBusImg_flip = SimpleVectorImage.LoadFromFile(Utility.LoadFileResource("res/entities/tmp_bus_flip.json.txt"));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		//Recolor per the user's config file.
		Hashtable<String, Color> overrides = MainFrame.GetOverrides("tmp-bus-fake", TmpBusBkgdColorIDs, TmpBusLineColorIDs);
		FakeTempBusImg.buildColorIndex(overrides);
		FakeTempBusImg.phaseColors(0xFF/2);

		FakeTempBusImg.buildColorIndex(overrides);
		FakeTempBusImg.phaseColors(0xFF/2);
		
		FakeTempBusImg.buildColorIndex(overrides);
		FakeTempBusImg.phaseColors(0xFF/2);
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		
		AffineTransform oldAT = g.getTransform();
		AffineTransform at = AffineTransform.getTranslateInstance(pos.getX(), pos.getY());
		
		//HACK: Modify the angle slightly based on the quadrant
		double angleD = getAngle();
		if (angleD<90) {
			//Moving down-right
			angleD -= 7;
		} else if (getAngle()<180) {
			//Moving down-left
			angleD += 7;
		} else if (getAngle()<270) {
			//Moving up-left
			angleD -= 7;
		} else if (getAngle()<360) {
			//Moving up-right
			angleD += 7;
		}
		
		//TEMP
		double onePixelInM = 3; //Assume pixels are 15m		
		double scaleMultiplier = (ScaledPoint.getScaleFactor().getX()*onePixelInM);
		SimpleVectorImage svi = null;
		//Retrieve the image to draw
		if(getAngle()<90 || getAngle()>270){
			svi = (params.DrawFakeOn&&getFake()) ? FakeTempBusImg: params.DebugOn ? DebugTempBusImg : TempBusImg;			
		}
		else  {
			svi = (params.DrawFakeOn&&getFake()) ? FakeTempBusImg_flip: params.DebugOn ? DebugTempBusImg_flip : TempBusImg_flip;
		}
		
		BufferedImage toDraw = svi.getImage(scaleMultiplier, angleD, true);
			
		//Translate to top-left corner
		at.translate(-toDraw.getWidth()/2, -toDraw.getHeight()/2);

		//Set new transformation matrix
		g.setTransform(at);
		
		//Draw the car
		g.drawImage(toDraw, 0, 0, null);
		
		//Draw a circle with its passenger count
		if (passengerCount>0) {
			Point center = new Point(toDraw.getWidth()/2, toDraw.getHeight()/2-20);
			final int Size = 20;
			
			//Background
			g.setColor(Color.BLUE);
			g.fillOval(center.x-Size/2, center.y-Size/2, Size, Size);
			g.setColor(Color.CYAN);
			g.drawOval(center.x-Size/2, center.y-Size/2, Size, Size);
			
			//Text
			g.setColor(Color.WHITE);
			g.setFont(DriverTick.idFont);
			int strW = g.getFontMetrics().stringWidth(""+passengerCount);
			g.drawString(""+passengerCount, center.x-strW/2+1, center.y+4);
		}
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
		//Sample debug output
		if (params.DebugOn) {
			int sz = 12;
			int x = (int)pos.getX();
			int y = (int)pos.getY();
			g.setColor(DriverTick.debugClr);
			g.setStroke(DriverTick.debugStr);
			g.drawOval(x-sz, y-sz, 2*sz, 2*sz);
			g.drawLine(x-3*sz/2, y, x+3*sz/2,y);
			g.drawLine(x, y-3*sz/2, x, y+3*sz/2);
		}
	}
	
}



