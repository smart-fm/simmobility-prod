package sim_mob.vis.controls;

import java.awt.Dimension;
import java.awt.Graphics2D;

/**
 * 
 * \author Seth N. Hetu
 */
public interface DrawableAgent {
	public void draw(Graphics2D g, double scale, boolean drawFake, boolean debugOn, Dimension size100Percent);
}
