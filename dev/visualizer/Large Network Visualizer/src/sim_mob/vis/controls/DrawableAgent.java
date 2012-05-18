package sim_mob.vis.controls;

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.geom.Point2D;

/**
 * 
 * \author Seth N. Hetu
 */
public interface DrawableAgent {
	public void draw(Graphics2D g, double scaleMultiplier, boolean drawFake, boolean debugOn, Point2D size100Percent);
}
