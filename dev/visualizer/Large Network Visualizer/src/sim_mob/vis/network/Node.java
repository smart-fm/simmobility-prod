package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Nodes represent locations in our network.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class Node implements DrawableItem {
	//Constants
	private static final int NODE_SIZE = 12;	
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;
	public Node(double x, double y, boolean isUni, Integer id) {
		pos = new ScaledPoint(x, y, null);
		this.isUni = isUni;
		this.id = id;
	}
	
	//NOTE: We are declaring the VISIBLE bounds of Node to be, e.g., (10x10)m in size. 
	//      This means that, at high zoom levels, it may not appear visible when partly offscreen.
	//      This is not really such an issue; for most uses it will appear correct. Note that
	//      similar "non-spatial" items make similar estimations.
	public Rectangle2D getBounds() {
		final double NODE_CM = 10*100; //10m square 
		return new Rectangle2D.Double(
			pos.getUnscaledX()-NODE_CM/2,
			pos.getUnscaledY()-NODE_CM/2,
			NODE_CM, NODE_CM);
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
	
	
	/*public void drawAnnotations(Graphics2D g, boolean drawAimsunID, boolean drawMitsimID) {
		g.setFont(new Font("Arial", Font.PLAIN, 10));
		final int extra = 4;
		final int margin = 1;
		int minWidth = g.getFontMetrics().stringWidth("XXXXXX");
		int height = g.getFontMetrics().getHeight() + margin;
		int startX = (int)pos.getX();
		int startY = (int)pos.getY() - (height + extra);
		for (int i=0; i<2; i++) {
			//Skip?
			String toDraw = (i==0) ? this.aimsunID : this.mitsimID;
			int width = Math.max(minWidth, g.getFontMetrics().stringWidth(toDraw));
			Color bgColor = (i==0) ? Node.aimsunBgColor : Node.mitsimBgColor;
			Color fgColor = (i==0) ? Node.aimsunFgColor : Node.mitsimFgColor;
			Color fontColor = Node.fontColor;
			if (toDraw.isEmpty()) { continue; }
			if (i==0 && !drawAimsunID) { continue; }
			if (i==1 && !drawMitsimID) { continue; }
			
			//Position the baseline accurately
			int fontH = height/2 + g.getFontMetrics().getAscent()/2 + 1;
			
			
			//Draw the background
			g.setColor(bgColor);
			g.fillRect(startX, startY, width, height);
			g.setColor(fgColor);
			g.setStroke(Node.annotationStroke);
			g.drawRect(startX, startY, width, height);
			g.setColor(fontColor); 
			g.drawString(toDraw, startX+extra, startY+fontH); //TODO: baseline
			
			//Increment 
			startY -= (height+extra);
		}
	}*/
	
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}

