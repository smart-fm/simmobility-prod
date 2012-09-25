package sim_mob.vis.network;


import java.awt.*;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

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
	// four corner joined by line for zooming. why co-ordinates are in int why not float?
	private ScaledPoint nearOne;
	private ScaledPoint nearTwo;
	private ScaledPoint farOne;
	private ScaledPoint farTwo;
	
	/*
	public double[] rotatebusstop (double centre_x, double centre_y,double len,double wid,double angle){

		double Diagonal_len = Math.hypot(len, wid);
		double angle_rad = Math.PI*angle/180;
		double theta = Math.atan(wid/len);
		
		double[] corner_pt = {centre_x+Diagonal_len/2*Math.cos(angle_rad+theta),				
				centre_x+Diagonal_len/2*Math.cos(Math.PI+angle_rad-theta),
				centre_x+Diagonal_len/2*Math.cos(Math.PI+angle_rad+theta),
				centre_x+Diagonal_len/2*Math.cos(angle_rad-theta),
				centre_y+Diagonal_len/2*Math.sin(angle_rad+theta),
				
				centre_y+Diagonal_len/2*Math.sin(Math.PI+angle_rad-theta),
				centre_y+Diagonal_len/2*Math.sin(Math.PI+angle_rad+theta),
				centre_y+Diagonal_len/2*Math.sin(angle_rad-theta)};
		
	return corner_pt;
	}
	*/
	
	// For trial let angle=140;
	// float BusStop_angle=140;
	
	
	
	private static Color BusStopColor = new Color(0xff, 0xff, 0xff);
	private static Stroke BusStopStroke = new BasicStroke(2.0F);
		
	private boolean isUni;   //Rather than having multiple classes....
	private Long id;
	public BusStop (ScaledPoint nearOne, ScaledPoint nearTwo, ScaledPoint farOne, ScaledPoint farTwo, Long id) {

	
		this.id = id;
		this.nearOne = nearOne;
		this.nearTwo = nearTwo;
		this.farOne = farOne;
		this.farTwo = farTwo;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_BusStop;
	}
	
	public ScaledPoint getNearOne() { return nearOne; }
	public ScaledPoint getNearTwo() { return nearTwo; }
	public ScaledPoint getFarOne() { return farOne; }
	public ScaledPoint getFarTwo() { return farTwo; }
	
	//Just make sure all points are included.
		public Rectangle2D getBounds() {
			final double BUFFER_CM = 10*100; //1m
			Rectangle2D res = new Rectangle2D.Double(nearOne.getUnscaledX(), nearOne.getUnscaledY(), 0, 0);
			res.add(nearTwo.getUnscaledX(), nearTwo.getUnscaledY());
			res.add(farOne.getUnscaledX(), farOne.getUnscaledY());
			res.add(farTwo.getUnscaledX(), farTwo.getUnscaledY());
			Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
			return res;
		}
	
	public boolean getIsUni() {
		return isUni;
	}
	public Long getID(){
		return id;
	}
	
	
	public void draw(Graphics2D g, DrawParams params){
	//	if (!params.PastCriticalZoom) { return; }
		
		g.setColor(BusStopColor);
		g.setStroke(BusStopStroke);    

		g.drawLine((int)nearOne.getX(), (int)nearOne.getY(), (int)nearTwo.getX(), (int)nearTwo.getY()); 
		g.drawLine((int)farOne.getX(), (int)farOne.getY(), (int)farTwo.getX(), (int)farTwo.getY()); 

		Polygon poly = new Polygon();		
		poly.addPoint((int)nearOne.getX(), (int)nearOne.getY());
		poly.addPoint((int)nearTwo.getX(), (int)nearTwo.getY());
		poly.addPoint((int)farTwo.getX(), (int)farTwo.getY());
		poly.addPoint((int)farOne.getX(), (int)farOne.getY());
        
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,0.5f));
		g.fillPolygon(poly);
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,1.0f));
		
	}
}

