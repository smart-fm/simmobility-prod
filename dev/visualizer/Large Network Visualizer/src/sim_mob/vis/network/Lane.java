package sim_mob.vis.network;

import java.awt.Graphics2D;
import sim_mob.vis.controls.DrawableItem;

public class Lane implements DrawableItem{
	private int laneNumber;
	private Node startMiddleNode;
	private Node endMiddleNode;

	
	public int getLaneNumber(){return laneNumber;}
	public Node getStartMiddleNode(){return startMiddleNode;}
	public Node getEndMiddleNode(){return endMiddleNode;}
	
	public Lane(int laneNumber, Node startMiddleNode, Node endMiddleNode){
		this.laneNumber = laneNumber;
		this.startMiddleNode = startMiddleNode;
		this.endMiddleNode = endMiddleNode;
		
	}
	
	@Override
	public void draw(Graphics2D g) {

/*
		g.setColor(MainFrame.Config.getLineColor("lane"));
		g.setStroke(MainFrame.Config.getLineStroke("lane"));

		if(isSideWalk){
			 
			g.setColor(MainFrame.Config.getLineColor("sidewalk"));
			g.setStroke(MainFrame.Config.getLineStroke("lane"));
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
		
		} 
		/*
		else if(laneNumber == 0){

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
		
*/		
				
	}
}
