package sim_mob.vis.controls;

import java.awt.geom.Point2D;

/**
 * Simple set of parameters that various objects need while being drawn. 
 * \author Seth N. Hetu
 */
public class DrawParams {
	//Required for all DrawableItems
	public boolean PastCriticalZoom;
	public boolean ShowCutLines;
	public boolean ShowAimsunAnnotations;
	public boolean ShowMitsimAnnotations;
	
	//Required only for DrawableAgents
	public Point2D CurrentViewSize;
	public boolean DebugOn;
	public boolean DrawFakeOn;
}
