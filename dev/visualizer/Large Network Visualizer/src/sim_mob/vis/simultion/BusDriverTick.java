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
 * BusDriver "Agent Tick"
 * 
 * \author Seth N. Hetu
 */
public class BusDriverTick extends DriverTick {
	private static SimpleVectorImage TempBusImg;
	private static SimpleVectorImage DebugTempBusImg;
	private static SimpleVectorImage FakeTempBusImg;
	
	
	//Later: custom colors
	private static final String[] TmpBusBkgdColorIDs = new String[] {
		"body", "body-lower", "lower-stripe", "upper-stripe", "light", "window"
	};
	private static final String[] TmpBusLineColorIDs = new String[] {
		"body-outline", "window-outline"
	};


	//The only field unique to BusDrivers (for now
	private int passengerCount;

	
	public BusDriverTick(double posX, double posY, double angle, int passengerCount, ScaledPointGroup spg) {
		super(posX, posY, angle, null, spg);
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
	

	
	public void draw(Graphics2D g,double scale, boolean drawFake,boolean debug){
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
		
		//Our bus image is significantly larger than our car image. So scale it.
		final double ScaleFact = 0.15;
		
		//Retrieve the image to draw
		SimpleVectorImage svi = (drawFake&&getFake()) ? FakeTempBusImg: debug ? DebugTempBusImg : TempBusImg;			
		BufferedImage toDraw = svi.getImage((1/scale + 0.2)*ScaleFact, angleD, true);
			
		//Translate to top-left corner
		at.translate(-toDraw.getWidth()/2, -toDraw.getHeight()/2);

		//Set new transformation matrix
		g.setTransform(at);
		
		//Draw the car
		g.drawImage(toDraw, 0, 0, null);
		
		//Draw a circle with its passenger count
		if (passengerCount>0) {
			//Background
			g.setColor(Color.BLUE);
			g.fillOval(10, 10, 20, 20);
			g.setColor(Color.CYAN);
			g.drawOval(10, 10, 20, 20);
			
			//Text
			g.setColor(Color.WHITE);
			g.setFont(DriverTick.idFont);
			g.drawString(""+passengerCount, 10, 10);
		}
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
		//Sample debug output
		if (debug) {
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



