package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;


import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * BusStops represent locations in our network.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class BusStop implements DrawableItem {
	//Constants
	private static final int BusStop_Length= 30;
	private static final int BusStop_Width = 15;
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;
	public BusStop (double x, double y, boolean isUni, Integer id) {
		pos = new FlippedScaledPoint(x, y);
		this.isUni = isUni;
		this.id = id;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_BusStop;
	}
	
	
	public Rectangle2D getBounds() {
		final double BusStop_CM = 10*100; //10m square 
		return new Rectangle2D.Double(
			pos.getUnscaledX(),
			pos.getUnscaledY(),
			BusStop_CM, BusStop_CM);
	}
	
	
	public ScaledPoint getPos() {
		return pos;
	}
	
	public boolean getIsUni() {
		return isUni;
	}
	public Integer getID(){
		return id;
	}
	
	
	
	public void draw(Graphics2D g,DrawParams params) {
		// if (params.PastCriticalZoom && isUni) { return; }
		
		int[] coords = new int[]{(int)pos.getX(), (int)pos.getY()};
		g.setColor(MainFrame.Config.getBackground("BusStop"));
			
		g.fillRect(coords[0], coords[1], BusStop_Length, BusStop_Width);
		/*
		if (isUni) {
			g.setStroke(MainFrame.Config.getLineStroke("uniBusStop"));
			g.setColor(MainFrame.Config.getLineColor("uniBusStop"));
		} */
		
		g.drawRect(coords[0], coords[1], BusStop_Length, BusStop_Width);
	}
	
	
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}
