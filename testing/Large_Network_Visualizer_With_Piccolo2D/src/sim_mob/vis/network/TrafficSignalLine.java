package sim_mob.vis.network;

import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class TrafficSignalLine {
	
	private Lane fromLane;
	private Lane toLane;
	private Node fromNode;
	private Node toNode;
	
	//private final int ARR_SIZE = 6; 
	
	public Node getFromNode (){return fromNode;}
	public Node getToNode (){return toNode;}
	
	
	public TrafficSignalLine(Lane fromLane, Lane toLane){
		this.fromLane = fromLane;
		this.toLane = toLane;

		this.findNode();
	}
	
	
	private void findNode(){
		double distStartStart = Utility.Distance(fromLane.getStartMiddleNode().getX(), 
													fromLane.getStartMiddleNode().getY(), 
													toLane.getStartMiddleNode().getX(), 
													toLane.getStartMiddleNode().getY());
	
		double distStartEnd = Utility.Distance(fromLane.getStartMiddleNode().getX(), 
				fromLane.getStartMiddleNode().getY(), 
				toLane.getEndMiddleNode().getX(), 
				toLane.getEndMiddleNode().getY());

		double distEndStart = Utility.Distance(fromLane.getEndMiddleNode().getX(), 
				fromLane.getEndMiddleNode().getY(), 
				toLane.getStartMiddleNode().getX(), 
				toLane.getStartMiddleNode().getY());

		double distEndEnd = Utility.Distance(fromLane.getEndMiddleNode().getX(), 
				fromLane.getEndMiddleNode().getY(), 
				toLane.getEndMiddleNode().getX(), 
				toLane.getEndMiddleNode().getY());

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
