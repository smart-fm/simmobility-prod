package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;

import sim_mob.vis.controls.DrawableItem;

public class CutLine implements DrawableItem{

	
	private Node start;
	private Node end;
	private String color;

	public Node getStartNode(){return start;}
	public Node getEndNode(){return end;}
	
	public CutLine(Node startNode, Node endNode,String color){
		this.start = startNode;
		this.end = endNode;
		this.color = color;
	}
	
	@Override
	public void draw(Graphics2D g) {
		// TODO Auto-generated method stub
		if(color.equals("red")){
			g.setColor(Color.white);
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
			
		}else if(color.equals("blue")){
			g.setColor(Color.red);
		    Stroke oldStroke = g.getStroke();
		    
			final float dash1[] = {10.0f};
		    final BasicStroke dashed = new BasicStroke(3.0f,
		                                          BasicStroke.CAP_BUTT,
		                                          BasicStroke.JOIN_MITER,
		                                          10.0f, dash1, 0.0f);
			g.setStroke(dashed);
		
			g.drawLine((int)start.getPos().getX(), (int)start.getPos().getY(), (int)end.getPos().getX(), (int)end.getPos().getY()); 
			
			g.setStroke(oldStroke);
		}
		    
	}

}
