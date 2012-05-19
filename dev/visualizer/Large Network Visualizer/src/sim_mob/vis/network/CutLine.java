package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 */
public class CutLine implements DrawableItem{

	
	private ScaledPoint start;
	private ScaledPoint end;
	private String color;

	public ScaledPoint getStartNode(){return start;}
	public ScaledPoint getEndNode(){return end;}
	
	public CutLine(ScaledPoint startNode, ScaledPoint endNode,String color){
		this.start = startNode;
		this.end = endNode;
		this.color = color;
	}
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_CUTLINE;
	}
	
	
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(start.getUnscaledX(), start.getUnscaledY(), 0, 0);
		res.add(end.getUnscaledX(), end.getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	
	@Override
	public void draw(Graphics2D g, boolean pastCriticalZoom) {
		// TODO Auto-generated method stub
		if(color.equals("red")){
			g.setColor(Color.white);
			g.drawLine((int)start.getX(), (int)start.getY(), (int)end.getX(), (int)end.getY()); 
			
		}else if(color.equals("blue")){
			g.setColor(Color.red);
		    Stroke oldStroke = g.getStroke();
		    
			final float dash1[] = {10.0f};
		    final BasicStroke dashed = new BasicStroke(3.0f,
		                                          BasicStroke.CAP_BUTT,
		                                          BasicStroke.JOIN_MITER,
		                                          10.0f, dash1, 0.0f);
			g.setStroke(dashed);
		
			g.drawLine((int)start.getX(), (int)start.getY(), (int)end.getX(), (int)end.getY()); 
			
			g.setStroke(oldStroke);
		}
		    
	}

}
