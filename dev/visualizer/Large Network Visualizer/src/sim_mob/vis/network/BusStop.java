package sim_mob.vis.network;


import java.awt.AlphaComposite;
import java.awt.*;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.Stroke;
import java.awt.geom.Rectangle2D;


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

	// @amit: Defining a new function to have four corners of bus-stop to be rotated 
	// about its centre coordinate (x,y) by angle phi. It will help to have 
	// four corner joined by line for zooming.? why co-ordinates are in int why not float

	
	public double[] rotatebusstop (int centre_x, int centre_y,float len,float wid,float angle){
		double Diagonal_len = Math.hypot(len, wid);
		double angle_rad = 3.14159265*angle/180;
		double theta = Math.atan(wid/len);
		
		double[] corner_pt = {centre_x+Diagonal_len/2*Math.cos(angle_rad+theta),				
				centre_x+Diagonal_len/2*Math.cos(3.14159265+angle_rad-theta),
				centre_x+Diagonal_len/2*Math.cos(3.14159265+angle_rad+theta),
				centre_x+Diagonal_len/2*Math.cos(angle_rad-theta),
				centre_y+Diagonal_len/2*Math.sin(angle_rad+theta),
				
				centre_y+Diagonal_len/2*Math.sin(3.14159265+angle_rad-theta),
				centre_y+Diagonal_len/2*Math.sin(3.14159265+angle_rad+theta),
				centre_y+Diagonal_len/2*Math.sin(angle_rad-theta)};
		
	return corner_pt;
	}
	
	// For trial let angle=60;
	float BusStop_angle=140;
	
	private static Color BusStopColor = new Color(0xff, 0xff, 0xff);
	private static Stroke BusStopStroke = new BasicStroke(1.0F);
	
	private int BusStop_Length= 40;
	private int BusStop_Width = 20;
	
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
		
		//int[] coords = new int[]{(int)pos.getX(), (int)pos.getY()};
		// g.setColor(MainFrame.Config.getBackground("busstop"));
		
		g.setColor(BusStopColor);
		g.setStroke(BusStopStroke);    
		double[] corner=rotatebusstop( (int)pos.getX(), (int)pos.getY(),BusStop_Length,BusStop_Width,BusStop_angle);
		
		
		
		g.drawLine((int)corner[0],(int)corner[4],(int)corner[1],(int)corner[5]);
		g.drawLine((int)corner[1],(int)corner[5],(int)corner[2],(int)corner[6]);
		g.drawLine((int)corner[2],(int)corner[6],(int)corner[3],(int)corner[7]);
		g.drawLine((int)corner[3],(int)corner[7],(int)corner[0],(int)corner[4]);
		
		
		Polygon poly = new Polygon();		
		poly.addPoint((int)corner[0], (int)corner[4]);
		poly.addPoint((int)corner[1], (int)corner[5]);
		poly.addPoint((int)corner[2], (int)corner[6]);
		poly.addPoint((int)corner[3], (int)corner[7]);
		
        
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,0.5f));
		g.fillPolygon(poly);
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,1.0f));
			
		// g.fillRect(coords[0], coords[1], BusStop_Length, BusStop_Width);
		/*
		if (isUni) {
			g.setStroke(MainFrame.Config.getLineStroke("uniBusStop"));
			g.setColor(MainFrame.Config.getLineColor("uniBusStop"));
		} */
		
		// g.drawRect(coords[0], coords[1], BusStop_Length, BusStop_Width);
	}
	
	
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}
