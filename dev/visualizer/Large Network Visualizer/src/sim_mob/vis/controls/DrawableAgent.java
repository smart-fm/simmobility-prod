package sim_mob.vis.controls;

import java.awt.Graphics2D;

public interface DrawableAgent {
	public void draw(Graphics2D g, double scale, boolean drawFake);
}
