package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class Crossing implements DrawableItem{
	//Constants/Resources
	private static Color crossingColor = new Color(0x00, 0x9a, 0xcd);
	private static Stroke crossingStroke = new BasicStroke(1.0F);
	private float alpha = 0.5f;
	

	private ScaledPoint nearOne;
	private ScaledPoint nearTwo;
	private ScaledPoint farOne;
	private ScaledPoint farTwo;
	private int id;
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_CROSSING;
	}
	
	
	public Crossing(ScaledPoint nearOne, ScaledPoint nearTwo, ScaledPoint farOne, ScaledPoint farTwo,int id) {
		this.nearOne = nearOne;
		this.nearTwo = nearTwo;
		this.farOne = farOne;
		this.farTwo = farTwo;
		this.id = id;
	}
	
	
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
	

	public ScaledPoint getNearOne() { return nearOne; }
	public ScaledPoint getNearTwo() { return nearTwo; }
	public ScaledPoint getFarOne() { return farOne; }
	public ScaledPoint getFarTwo() { return farTwo; }
	public int getId() { return id; }
	
	
	
	public void draw(Graphics2D g, boolean pastCriticalZoom){
		g.setColor(crossingColor);
		g.setStroke(crossingStroke);    

		g.drawLine((int)nearOne.getX(), (int)nearOne.getY(), (int)nearTwo.getX(), (int)nearTwo.getY()); 
		g.drawLine((int)farOne.getX(), (int)farOne.getY(), (int)farTwo.getX(), (int)farTwo.getY()); 

		Polygon poly = new Polygon();		
		poly.addPoint((int)nearOne.getX(), (int)nearOne.getY());
		poly.addPoint((int)nearTwo.getX(), (int)nearTwo.getY());
		poly.addPoint((int)farTwo.getX(), (int)farTwo.getY());
		poly.addPoint((int)farOne.getX(), (int)farOne.getY());
        
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,alpha));
		g.fillPolygon(poly);
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,1.0f));
		
	}
	

}
