package sim_mob.vis.network.basic;


/**
 * Simply a Scaled point where the y-axis has been flipped. Makes it more obvious when we're using 
 *   flipped coordinates, and when we're not. 
 * 
 *  \author Seth N. Hetu
 */
public class FlippedScaledPoint extends ScaledPoint {
	private static final boolean DoFlip = true;   ///Can turn off globally if desired.
	
	public FlippedScaledPoint(double x, double y) {
		super(x, DoFlip?-y:y);
	}
}
