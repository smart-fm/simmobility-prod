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
		// TODO Auto-generated method stub
		startMiddleNode.draw(g);
		endMiddleNode.draw(g);
	}
}