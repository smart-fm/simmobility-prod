package sim_mob.vis.network;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;


/**
 * Class for representing a Graph as stored in the Street Directory
 */
public class StDirVertex implements DrawableItem {
	private static final int VERTEX_SIZE = 16;
	private static final Color vertexColor = new Color(0x999999);
	private static final Stroke vertexStroke = new BasicStroke(2.0F);
	
	public StDirVertex(long id, double x, double y) {
		this.id = id;
		this.pos = new ScaledPoint(x, -y);
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		Rectangle2D box = new Rectangle2D.Double(pos.getX()-VERTEX_SIZE/2.0, pos.getY()-VERTEX_SIZE/2.0, VERTEX_SIZE, VERTEX_SIZE);
		g.setColor(vertexColor);
		g.setStroke(vertexStroke);
		
		//Draw as a box with x marks
		g.draw(box);
		g.draw(new Line2D.Double(box.getMinX(), box.getMinY(), box.getMaxX(), box.getMaxY()));
		g.draw(new Line2D.Double(box.getMaxX(), box.getMinY(), box.getMinX(), box.getMaxY()));
	}
	
	public Rectangle2D getBounds() {
		final double VERTEX_CM = 10*100; //10m square 
		return new Rectangle2D.Double(
			pos.getUnscaledX()-VERTEX_CM/2,
			pos.getUnscaledY()-VERTEX_CM/2,
			VERTEX_CM, VERTEX_CM);
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_SD_VERTEX;
	}
	
	public long getID() { return id; }
	public ScaledPoint getPos() { return pos; }
	
	private long id;
	private ScaledPoint pos;
}
