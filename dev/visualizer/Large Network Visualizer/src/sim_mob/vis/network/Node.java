package sim_mob.vis.network;

import java.awt.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Nodes represent locations in our network.
 */
public class Node implements DrawableItem {
	//Constants
	private static final int NODE_SIZE = 12;
	//private static Color nodeColor = new Color(0xFF, 0x88, 0x22);
	//private static Stroke nodeStroke = new BasicStroke(3.0F);
	//private static Stroke nodeThinStroke = new BasicStroke(1.0F);
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;
	public Node(double x, double y, boolean isUni, Integer id) {
		pos = new ScaledPoint(x, y, null);
		this.isUni = isUni;
		this.id = id;
	}
	
	public ScaledPoint getPos() {
		return pos;
	}
	
	public boolean getIsUni() {
		return isUni;
	}
	public Integer getID(){
		return id;
	}
	
	public void draw(Graphics2D g) {
		int[] coords = new int[]{(int)pos.getX()-NODE_SIZE/2, (int)pos.getY()-NODE_SIZE/2};
		g.setColor(MainFrame.Config.getBackground("node"));
			
		g.fillOval(coords[0], coords[1], NODE_SIZE, NODE_SIZE);
		
		if (isUni) {
			g.setStroke(MainFrame.Config.getLineStroke("uninode"));
			g.setColor(MainFrame.Config.getLineColor("uninode"));
		} else {
			g.setStroke(MainFrame.Config.getLineStroke("multinode"));
			g.setColor(MainFrame.Config.getLineColor("multinode"));
		}
		
		g.drawOval(coords[0], coords[1], NODE_SIZE	, NODE_SIZE);
		
	}
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}
