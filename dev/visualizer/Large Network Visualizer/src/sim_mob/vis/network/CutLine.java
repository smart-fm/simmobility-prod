package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;

import sim_mob.vis.controls.DrawableItem;

public class CutLine implements DrawableItem{

	
	private Node start;
	private Node end;
	

	public Node getStartNode(){return start;}
	public Node getEndNode(){return end;}
	
	public CutLine(Node startNode, Node endNode){
		this.start = startNode;
		this.end = endNode;
	}
	
	@Override
	public void draw(Graphics2D g) {
		// TODO Auto-generated method stub
		g.setColor(Color.BLUE);
		
		final float dash1[] = {10.0f};
	    final BasicStroke dashed = new BasicStroke(1.0f,
	                                          BasicStroke.CAP_BUTT,
	                                          BasicStroke.JOIN_MITER,
	                                          10.0f, dash1, 0.0f);
	    g.setStroke(dashed);
		//g.setStroke(new BasicStroke(1.0F));
		g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
	}

}
