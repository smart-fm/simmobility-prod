package sim_mob.vis.network;

import static java.awt.geom.AffineTransform.getRotateInstance;
import static java.awt.geom.AffineTransform.getTranslateInstance;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Line2D;
import java.awt.geom.Path2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;


/**
 * Class for representing a Graph as stored in the Street Directory
 */
public class StDirEdge implements DrawableItem {
	private static final Color edgeColor = new Color(0x55CCCC);
	private static final Color arrowColor = new Color(0xCC0077);
	private static final Stroke edgeStroke = new BasicStroke(2.0F);
	
	public StDirEdge(long id, StDirVertex from, StDirVertex to) {
		this.id = id;
		this.from = from;
		this.to = to;
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		//Transform, draw
		AffineTransform oldAt = g.getTransform();
		//g.drawLine((int)from.getPos().getX(), (int)from.getPos().getY(), (int)to.getPos().getX(), (int)to.getPos().getY());
		drawEdge(g, params);
		g.setTransform(oldAt);
	}

	public void drawEdge(Graphics2D g, DrawParams params) {
		int ARR_SZ = 15;
		int CURVE_SZ = 10;
	    double dx = to.getPos().getX() - from.getPos().getX();
	    double dy = to.getPos().getY() - from.getPos().getY();
	    double angle = Math.atan2(dy, dx);
	    int len = (int) Math.sqrt(dx*dx + dy*dy);
	    AffineTransform at = getTranslateInstance(from.getPos().getX(), from.getPos().getY());
	    at.concatenate(getRotateInstance(angle));
	    g.setTransform(at);
	    
	    //Make the first part of the arrow (the line) a Bezier curve; this causes it to stand out more.
	    Path2D line = new Path2D.Double();
	    line.moveTo(0, 0);
	    line.curveTo(len/2, -CURVE_SZ, len/2, -CURVE_SZ, len, 0);
	    
	    //Draw the line
	    g.setStroke(edgeStroke);
	    g.setColor(edgeColor);
	    g.draw(line);
	    
	    //Draw the arrow.
	    //TODO: The Bezier curve causes this to appear slightly off-center to the actual line.
	    //      We can transform or otherwise correct this later; for now, it's fine as a diagnostic tool.
	    g.setColor(arrowColor);
	    g.drawLine(len, 0, len-ARR_SZ, -ARR_SZ/3);
	    g.drawLine(len, 0, len-ARR_SZ,  ARR_SZ/3);
	}
	
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 1*100; //1m
		Rectangle2D res = new Rectangle2D.Double(from.getPos().getUnscaledX(), from.getPos().getUnscaledY(), 0, 0);
		res.add(to.getPos().getUnscaledX(), to.getPos().getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_SD_EDGE;
	}
	
	public long getID() { return id; }
	
	private long id;
	private StDirVertex from;
	private StDirVertex to;
}
