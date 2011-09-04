package sim_mob.vis.network;

import java.awt.*;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Nodes represent locations in our network.
 */
public class Node implements DrawableItem {
	//Constants
	private static final int NODE_SIZE = 12;
	private static Color nodeColor = new Color(0xFF, 0x88, 0x22);
	private static Stroke nodeStroke = new BasicStroke(3.0F);
	private static Stroke nodeThinStroke = new BasicStroke(1.0F);
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	
	public Node(double x, double y, boolean isUni) {
		pos = new ScaledPoint(x, y);
		this.isUni = isUni;
	}
	
	public ScaledPoint getPos() {
		return pos;
	}
	
	public boolean getIsUni() {
		return isUni;
	}
	
	public void draw(Graphics2D g) {
		int[] coords = new int[]{(int)pos.getX()-NODE_SIZE/2, (int)pos.getY()-NODE_SIZE/2};
		g.setColor(nodeColor);
		g.fillOval(coords[0], coords[1], NODE_SIZE, NODE_SIZE);
		if (isUni) {
			g.setStroke(nodeThinStroke);
			g.setColor(Color.BLUE);
		} else {
			g.setStroke(nodeStroke);
			g.setColor(Color.BLACK);
		}
		g.drawOval(coords[0], coords[1], NODE_SIZE, NODE_SIZE);
	}
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}
