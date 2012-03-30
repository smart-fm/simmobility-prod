package sim_mob.vis.network;

import java.awt.*;

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
	//private static Color nodeColor = new Color(0xFF, 0x88, 0x22);
	//private static Stroke nodeStroke = new BasicStroke(3.0F);
	//private static Stroke nodeThinStroke = new BasicStroke(1.0F);
	
	//Temp; move to config
	private static final Color aimsunBgColor = new Color(0xFF,0x66, 0x00, 0xAA);
	private static final Color aimsunFgColor = new Color(0xFF, 0xCC, 0xCC);
	private static final Color mitsimBgColor = new Color(0x00, 0xCC, 0xFF, 0xAA);
	private static final Color mitsimFgColor = new Color(0xCC, 0xCC, 0xFF);
	private static final Color fontColor = new Color(0x00, 0x00, 0x00);
	private static final Stroke annotationStroke = new BasicStroke(1.0F);
	
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;
	public String aimsunID;
	public String mitsimID;
	public Node(double x, double y, boolean isUni, Integer id) {
		pos = new ScaledPoint(x, y, null);
		this.isUni = isUni;
		this.id = id;
		this.aimsunID = "";
		this.mitsimID = "";
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
	
	
	public void drawAnnotations(Graphics2D g, boolean drawAimsunID, boolean drawMitsimID) {
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
	}
	
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}

