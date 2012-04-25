package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Ellipse2D;

import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.util.PPaintContext;
import sim_mob.vis.MainFrame;

public class Node extends PNode {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private static final int NODE_SIZE = 12;	
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;

	public String toString() {
		return "(" + getX() + "," + getY() + ")";
	}
        
	public Node(double x, double y, boolean isUni, Integer id) {
		
//		this.localPos = new LocalPoint(x,y);		
		this.isUni = isUni;
		this.id = id;

		this.setBounds(x-NODE_SIZE/2, y-NODE_SIZE/2, NODE_SIZE, NODE_SIZE);
		//this.ellipse = new Ellipse2D.Double(x-NODE_SIZE/2, y-NODE_SIZE/2, NODE_SIZE, NODE_SIZE);
	}
	
	//Keep the display in line with the bounds (todo: enforce that it remains square)
	public boolean setBounds(double x, double y, double w, double h) {
		boolean res = super.setBounds(x,  y, w, h);
		if (res) {
	//		this.localPos = new LocalPoint(x,y);	
			//this.ellipse = new Ellipse2D.Double(x, y, w, h);
		}
		return res;
	}
	

	public boolean getIsUni() {
		return isUni;
	}
	public Integer getID(){
		return id;
	}

    
	protected void paint(PPaintContext paintContext) {
		//We need to scale our ellipse's coordinates so that they are 
		//  always NODE_SIZE pixels in screen coords.
		double ActNodeSize = NODE_SIZE/paintContext.getScale();
		Ellipse2D ellipse = new Ellipse2D.Double(getX()-ActNodeSize/2, getY()-ActNodeSize/2, ActNodeSize, ActNodeSize);
		
		Graphics2D g = paintContext.getGraphics();
		g.setColor(MainFrame.Config.getBackground("node"));
		g.fill(ellipse);
		
		Stroke str = null;
		if (isUni) {
			str = MainFrame.Config.getLineStroke("uninode");
			g.setColor(MainFrame.Config.getLineColor("uninode"));
		} else {
			str = MainFrame.Config.getLineStroke("multinode");
			g.setColor(MainFrame.Config.getLineColor("multinode"));
		}
		if (str instanceof BasicStroke) {
			//We can scale the stroke too
			BasicStroke bs = (BasicStroke)(str);
			str = new BasicStroke((float)(bs.getLineWidth()/paintContext.getScale()), bs.getEndCap(), bs.getLineJoin(), bs.getMiterLimit());
		}
		g.setStroke(str);
	
		g.draw(ellipse);
	}	
}
