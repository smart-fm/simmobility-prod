package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.AffineTransform;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.Vect;

public class Lane implements DrawableItem{
	//Constants/Resources
	private static Color laneColor = new Color(0x00, 0x00, 0x00);
	private static Color sideWalkColor = new Color(0x84, 0x70, 0xff);
	private static Stroke laneStroke = new BasicStroke(1.0F);
	
	private Integer parentSegment;
	private Node start;
	private Node end;
	
	private boolean isSideWalk;
	private int laneNumber;

	public Lane(Node start, Node end, boolean isSideWalk, int lineNumber, Integer parentSegment) {
		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;
		
	}

	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Integer getParentSegment(){ return parentSegment; }
	
	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	@Override
	public void draw(Graphics2D g) {
		g.setColor(laneColor);
		g.setStroke(laneStroke);
		if(isSideWalk){
			 
			g.setColor(sideWalkColor);
			g.setStroke(laneStroke);
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
		
		} else if(laneNumber == 0){

			g.setColor(Color.red);
			g.setStroke(laneStroke);
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
			
			
		} else if(laneNumber == 1){

			g.setColor(Color.GREEN);
			g.setStroke(laneStroke);
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 	
			
		}
		else {
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY());
				
		}
		
		
				
	}

	
}
