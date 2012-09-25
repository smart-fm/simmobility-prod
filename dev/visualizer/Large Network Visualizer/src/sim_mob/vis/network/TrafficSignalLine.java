package sim_mob.vis.network;

import static java.awt.geom.AffineTransform.getRotateInstance;

import static java.awt.geom.AffineTransform.getTranslateInstance;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.Main;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class TrafficSignalLine implements DrawableItem{
	
	private Lane fromLane;
	private Lane toLane;
	private Node fromNode;
	private Node toNode;
	private Color currColor;
	private String temPhaseName;
	
	private final int ARR_SIZE = 6; 
	
	
	public TrafficSignalLine(Lane fromLane, Lane toLane,String temPhaseName_, int startingColor) {
		if (fromLane==null || toLane==null) { 
			throw new RuntimeException("Can't create a TrafficSignalLine with a null from/to lane."); 
		}
		
		this.fromLane = fromLane;
		this.toLane = toLane;
		this.findNode();
		this.temPhaseName = temPhaseName_;
		setLightColor(startingColor);
	}
	
	public TrafficSignalLine(Lane fromLane, Lane toLane) {
		this(fromLane, toLane, "", -1);
	}
	
	public String getPhaseName() { return temPhaseName; }
	public Lane getFromLane() { return fromLane; }
	public Lane getToLane() { return toLane; }
	public Node getFromNode() { return fromNode; }
	public Node getToNode() { return toNode; }
	public Color getCurrColor() { return currColor; }
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TSL;
	}
	
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(fromNode.getPos().getUnscaledX(), fromNode.getPos().getUnscaledY(), 0, 0);
		res.add(toNode.getPos().getUnscaledX(), toNode.getPos().getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	
	private void findNode(){
		double distStartStart = Utility.Distance(fromLane.getStartMiddleNode().getPos().getUnscaledX(), 
													fromLane.getStartMiddleNode().getPos().getUnscaledY(), 
													toLane.getStartMiddleNode().getPos().getUnscaledX(), 
													toLane.getStartMiddleNode().getPos().getUnscaledY());
	
		double distStartEnd = Utility.Distance(fromLane.getStartMiddleNode().getPos().getUnscaledX(), 
				fromLane.getStartMiddleNode().getPos().getUnscaledY(), 
				toLane.getEndMiddleNode().getPos().getUnscaledX(), 
				toLane.getEndMiddleNode().getPos().getUnscaledY());

		double distEndStart = Utility.Distance(fromLane.getEndMiddleNode().getPos().getUnscaledX(), 
				fromLane.getEndMiddleNode().getPos().getUnscaledY(), 
				toLane.getStartMiddleNode().getPos().getUnscaledX(), 
				toLane.getStartMiddleNode().getPos().getUnscaledY());

		double distEndEnd = Utility.Distance(fromLane.getEndMiddleNode().getPos().getUnscaledX(), 
				fromLane.getEndMiddleNode().getPos().getUnscaledY(), 
				toLane.getEndMiddleNode().getPos().getUnscaledX(), 
				toLane.getEndMiddleNode().getPos().getUnscaledY());

		double miniMumDistance = findMinimum(distStartStart,distStartEnd,distEndStart,distEndEnd);
		
		if(miniMumDistance == distStartStart){
			fromNode = fromLane.getStartMiddleNode();
			toNode = toLane.getStartMiddleNode();
		} else if(miniMumDistance == distStartEnd){
			fromNode = fromLane.getStartMiddleNode();
			toNode = toLane.getEndMiddleNode();		
		} else if(miniMumDistance == distEndStart){
			fromNode = fromLane.getEndMiddleNode();
			toNode = toLane.getStartMiddleNode();
		} else if(miniMumDistance == distEndEnd){
			fromNode = fromLane.getEndMiddleNode();
			toNode = toLane.getEndMiddleNode();
		} else{
			System.out.println("Error, No minimum distance -- TrafficSignalLine, findNode()");
		}
		
	}

	private double findMinimum(double dist1,double dist2,double dist3,double dist4){
		double minimumDistance = Math.min(Math.min(dist1, dist2), Math.min(dist3, dist4));
		return minimumDistance;
	}
	
	@Override
	public void draw(Graphics2D g, DrawParams params) {
		if (currColor==null) { return; }
		if (!params.PastCriticalZoom) { return; }
		g.setColor(currColor);
		drawArrow(g, (int)fromNode.getPos().getX(), (int)fromNode.getPos().getY(),(int)toNode.getPos().getX(),(int)toNode.getPos().getY());
		
	}
	
	
	public void setLightColor(Integer color) {
		if(Main.NEW_SIGNAL)
		{
		switch(color)
		{
		case 1:
			currColor = Color.red;
			break;
		case 2:
			currColor = Color.yellow;
			break;
		case 3:
			currColor = Color.green;
			break;
		case 6:
			currColor = Color.cyan;
			break;
		default:
			currColor = Color.darkGray;
		}
		}
		else
		{
			if (color == 2) {
				currColor = Color.yellow;
			} else if (color==3) {
				currColor = Color.green;
			} else {
				currColor = null;
			}
		}
	}
	
	public void setLightColor(Color color) {
		currColor = color;
	}
	
    
	public void drawArrow(Graphics2D g, int x1, int y1, int x2, int y2) {
		AffineTransform oldAt = g.getTransform();

        double dx = x2 - x1, dy = y2 - y1;
        double angle = Math.atan2(dy, dx);
        int len = (int) Math.sqrt(dx*dx + dy*dy);
        AffineTransform at = getTranslateInstance(x1, y1);
        at.concatenate(getRotateInstance(angle));
        g.setTransform(at);

        // Draw horizontal arrow starting in (0, 0)
        g.drawLine(0, 0, (int) len, 0);
        g.fillPolygon(new int[] {len, len-ARR_SIZE, len-ARR_SIZE, len},
                      new int[] {0, -ARR_SIZE, ARR_SIZE, 0}, 4);
        
        //Restore
        g.setTransform(oldAt);
    }

}
