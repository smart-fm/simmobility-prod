package sim_mob.vis.network;


import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;


import sim_mob.vect.SimpleVectorImage;
import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.FlippedScaledPoint;
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

	

	public double[] rotatebusstop (int centre_x, int centre_y,double len,double wid,double angle){

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

	// For trial let angle=140;
	// float BusStop_angle=140;
	
	private static Color BusStopColor = new Color(0xff, 0x45, 0x00);
	private static Stroke BusStopStroke = new BasicStroke(2.0F);
	
	private double BusStop_Length= 30;
	private double BusStop_Width = 15;
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;
	private double BusStop_angle;
	public BusStop (double x, double y, boolean isUni, Integer id,double angle) {

		this.pos = new FlippedScaledPoint(x,y);

		pos = new FlippedScaledPoint(x, y);

		this.isUni = isUni;
		this.id = id;
		this.BusStop_angle=angle;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_BusStop;
	}
	
	
	public Rectangle2D getBounds() {
		final double BusStop_CM = 10*10; //10m square 
		return new Rectangle2D.Double(
			pos.getUnscaledX(),
			pos.getUnscaledY(),
			BusStop_CM+15, BusStop_CM+15);
		/*
		final double BusStop_CM = 0; //10m square
		double[] corner_temp=rotatebusstop(pos.getUnscaledX(),pos.getUnscaledY(),BusStop_Length,BusStop_Width,BusStop_angle);
		Rectangle2D res=new Rectangle2D.Double(corner_temp[0],corner_temp[4],0,0);
		res.add(corner_temp[0],corner_temp[4]);
		res.add(corner_temp[1],corner_temp[5]);
		res.add(corner_temp[3],corner_temp[6]);
		res.add(corner_temp[4],corner_temp[7]);
		
		Utility.resizeRectangle(res, res.getWidth()+BusStop_CM, res.getHeight()+BusStop_CM);
		return res;*/
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
		SimpleVectorImage svi = null;
		g.setColor(BusStopColor);
		g.setStroke(BusStopStroke);    

		double onePixelInM = 50; //Assume pixels are 15m		
		
		double scaleMultiplier = (pos.getScaleFactor().getX()*onePixelInM);
		
			
	//	BusStop_Length=	BusStop_Length*(scaleMultiplier);		
//		BusStop_Width=BusStop_Width*(1+scaleMultiplier);
	
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
