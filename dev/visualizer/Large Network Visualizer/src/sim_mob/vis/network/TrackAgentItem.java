package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Rectangle2D.Double;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * Nodes represent locations in our network.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class TrackAgentItem implements DrawableItem {
	//Some saved strokes, brushes and colors. 
	private static final Stroke str1pix = new BasicStroke(1);
	private static final Color hlColor = new Color(0xFF, 0xAA, 0x55);
	
	private ScaledPoint orig;
	private ScaledPoint track;
	
	public TrackAgentItem(ScaledPoint orig, ScaledPoint track) {
		this.orig = orig;
		this.track = track;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TRACKINGITEM;
	}
	
	
	//NOTE: This whole class is a bit of a hack; not sure the best way to handle relationships like this.
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 11*100; //1m of buffer room 
		Rectangle2D res = new Rectangle2D.Double(orig.getUnscaledX(), orig.getUnscaledY(), 0, 0);
		res.add(track.getUnscaledX(), track.getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}

	
	public void draw(Graphics2D g, DrawParams params) {
		//For now, just always draw it.
		g.setColor(hlColor);
		g.setStroke(str1pix);
		Point2D min = new Point2D.Double(Math.min(orig.getX(), track.getX()), Math.min(orig.getY(), track.getY()));
		Point2D max = new Point2D.Double(Math.max(orig.getX(), track.getX()), Math.max(orig.getY(), track.getY()));
		double dist = Utility.Distance(min, max);
		if (dist>1.0) {
			Line2D line = new Line2D.Double(min.getX(), min.getY(), max.getX(), max.getY());
			Ellipse2D el = CircleFromPoints(min, max, dist/2);
			g.draw(el);
			g.draw(line);
		}

	}
	
	
	private static final Ellipse2D CircleFromPoints(Point2D min, Point2D max, double radius) {
		//Too simplistic, but it will work for now.
		Point2D center = new Point2D.Double(min.getX()+(max.getX()-min.getX())/2, min.getY()+(max.getY()-min.getY())/2);
		Ellipse2D el = new Ellipse2D.Double(center.getX()-radius, center.getY()-radius, radius*2, radius*2);
		return el;
	}
}

