package sim_mob.vis.controls;

import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;

/**
 * 
 * \author Seth N. Hetu
 */
public interface DrawableItem {
	///Draw this item.
	public void draw(Graphics2D g);
	
	///Returns the bounds (in real, unscaled x,y coordinates) of this item.
	///  These bounds are not expected to change; if they do change, then the 
	///  code that relies on this items' bounds must be checked manually.
	public Rectangle2D getBounds();
	
}
