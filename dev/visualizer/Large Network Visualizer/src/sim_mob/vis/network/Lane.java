package sim_mob.vis.network;

import java.awt.*;
import sim_mob.vis.controls.DrawableItem;

public class Lane implements DrawableItem{
	//Constants/Resources
	private static Color laneColor = new Color(0x00, 0x00, 0x00);
	private static Color sideWalkColor = new Color(0x84, 0x70, 0xff);
	private static Stroke laneStroke = new BasicStroke(1.0F);
	
	private Node start;
	private Node end;
	private boolean isSideWalk;
	public Lane(Node start, Node end, boolean isSideWalk) {
		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
	}

	public Node getStart() { return start; }
	public Node getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }

	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	@Override
	public void draw(Graphics2D g) {
		g.setColor(laneColor);
		g.setStroke(laneStroke);
		if(!isSideWalk){
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
		}else{
			g.setColor(sideWalkColor);
			g.setStroke(laneStroke);
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
		}
			
			
		
		
	}

	
}
