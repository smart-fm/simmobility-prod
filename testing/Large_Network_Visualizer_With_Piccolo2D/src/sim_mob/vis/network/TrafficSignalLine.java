package sim_mob.vis.network;

import static java.awt.geom.AffineTransform.getRotateInstance;
import static java.awt.geom.AffineTransform.getTranslateInstance;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.util.PPaintContext;

import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class TrafficSignalLine extends PNode {
	private static final long serialVersionUID = 1L;
	
	private Lane fromLane;
	private Lane toLane;
	private Point2D fromPoint;
	private Point2D toPoint;
	private Color currColor;
	
	private final int ARR_SIZE = 6;  
	
	public Point2D getFromPoint (){return fromPoint;}
	public Point2D getToPoint (){return toPoint;}
	
	private static final double FindMinimum(double dist1, double... distances) {
		double min = dist1;
		for (double d : distances) {
			min = d<min ? d : min;
		}
		return min;
	}
	
	
	public TrafficSignalLine(Lane fromLane, Lane toLane){
		this.fromLane = fromLane;
		this.toLane = toLane;

		this.findNode();
		
		//Start at red
		updateSignal(Color.red);
		//updateSignal(Color.red, false);
		
		setBounds(fromPoint.getX(), fromPoint.getY(), toPoint.getX()-fromPoint.getX(), toPoint.getY()-fromPoint.getY());
	}
	
	
	//Update this signal
	public void updateSignal(Color currColor, boolean visible) {
		this.currColor = currColor;
		this.setVisible(visible);
	}
	public void updateSignal(Color currColor) { 
		updateSignal(currColor, true); 
	}
	
	
	private void findNode(){
		double distStartStart = Utility.Distance(
			fromLane.getStartMiddlePoint(), toLane.getStartMiddlePoint());
	
		double distStartEnd = Utility.Distance(
			fromLane.getStartMiddlePoint(), toLane.getEndMiddlePoint());

		double distEndStart = Utility.Distance(
			fromLane.getEndMiddlePoint(), toLane.getStartMiddlePoint());

		double distEndEnd = Utility.Distance(
			fromLane.getEndMiddlePoint(), toLane.getEndMiddlePoint());

		double miniMumDistance = FindMinimum(distStartStart,distStartEnd,distEndStart,distEndEnd);
		
		if(miniMumDistance == distStartStart){
			fromPoint = fromLane.getStartMiddlePoint();
			toPoint = toLane.getStartMiddlePoint();
		} else if(miniMumDistance == distStartEnd){
			fromPoint = fromLane.getStartMiddlePoint();
			toPoint = toLane.getEndMiddlePoint();		
		} else if(miniMumDistance == distEndStart){
			fromPoint = fromLane.getEndMiddlePoint();
			toPoint = toLane.getStartMiddlePoint();
		} else if(miniMumDistance == distEndEnd){
			fromPoint = fromLane.getEndMiddlePoint();
			toPoint = toLane.getEndMiddlePoint();
		} else{
			System.out.println("Error, No minimum distance -- TrafficSignalLine, findNode()");
		}
		
	}
	
	
	protected void paint(PPaintContext paintContext) {
		//System.out.println("Painting: " + Utility.Point2Str(fromPoint) + " => " + Utility.Point2Str(toPoint));
		
		//Set graphics
		Graphics2D g = paintContext.getGraphics();
		g.setColor(currColor);
		g.setStroke(new BasicStroke((float)(2.5/paintContext.getScale())));
		
		//Retrieve location information
		double dx = toPoint.getX() - fromPoint.getX();
		double dy = toPoint.getY() - fromPoint.getY();
		double angle = Math.atan2(dy, dx);
		
		int ArrSzScale = (int)(ARR_SIZE / paintContext.getScale());
		
		//Transform this so that we're pointing right.
		AffineTransform oldAT = g.getTransform();
		AffineTransform at = g.getTransform();
		at.translate(fromPoint.getX(), fromPoint.getY());
		at.concatenate(getRotateInstance(angle));
		g.setTransform(at);
		
		//Draw line, arrow
        int len = (int) Math.sqrt(dx*dx + dy*dy);
        g.drawLine(0, 0, (int) len, 0);
        g.fillPolygon(new int[] {len, len-ArrSzScale, len-ArrSzScale, len},
                      new int[] {0, -ArrSzScale, ArrSzScale, 0}, 
                      4);
        
        //Restore transformation
        g.setTransform(oldAT);
	}	
}
