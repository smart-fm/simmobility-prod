package sim_mob.vis.network;

import java.awt.geom.Point2D;

import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class TrafficSignalLine {
	
	private Lane fromLane;
	private Lane toLane;
	private Point2D fromPoint;
	private Point2D toPoint;
	
	//private final int ARR_SIZE = 6; 
	
	public Point2D getFromPoint (){return fromPoint;}
	public Point2D getToPoint (){return toPoint;}
	
	
	public TrafficSignalLine(Lane fromLane, Lane toLane){
		this.fromLane = fromLane;
		this.toLane = toLane;

		this.findNode();
	}
	
	
	private void findNode(){
		double distStartStart = Utility.Distance(fromLane.getStartMiddlePoint().getX(), 
													fromLane.getStartMiddlePoint().getY(), 
													toLane.getStartMiddlePoint().getX(), 
													toLane.getStartMiddlePoint().getY());
	
		double distStartEnd = Utility.Distance(fromLane.getStartMiddlePoint().getX(), 
				fromLane.getStartMiddlePoint().getY(), 
				toLane.getEndMiddlePoint().getX(), 
				toLane.getEndMiddlePoint().getY());

		double distEndStart = Utility.Distance(fromLane.getEndMiddlePoint().getX(), 
				fromLane.getEndMiddlePoint().getY(), 
				toLane.getStartMiddlePoint().getX(), 
				toLane.getStartMiddlePoint().getY());

		double distEndEnd = Utility.Distance(fromLane.getEndMiddlePoint().getX(), 
				fromLane.getEndMiddlePoint().getY(), 
				toLane.getEndMiddlePoint().getX(), 
				toLane.getEndMiddlePoint().getY());

		double miniMumDistance = findMinimum(distStartStart,distStartEnd,distEndStart,distEndEnd);
		
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
	
	private static double findMinimum(double dist1,double dist2,double dist3,double dist4){
		double minimumDistance = Math.min(Math.min(dist1, dist2), Math.min(dist3, dist4));
		return minimumDistance;
	}
	
	/*
	@Override
	public void draw(Graphics2D g) {
			
		drawArrow(g, (int)fromNode.getPos().getX(), (int)fromNode.getPos().getY(),(int)toNode.getPos().getX(),(int)toNode.getPos().getY());
		
	}
	
	
	
	public void drawPerLight(Graphics2D g, Integer light){
		
			// Do not draw out red lights			
			if(light == 2){	
				g.setColor(Color.yellow);
				draw(g);
			} else if( light == 3){
				g.setColor(Color.green);
				draw(g);
			}	
		
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
	*/
}
