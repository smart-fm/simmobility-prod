package sim_mob.vis.network.basic;

/** 
 * Very simply double-point class.
 * 
 * \author Seth N. Hetu
 */
public class DPoint {
	public double x;
	public double y;
	public DPoint(double x, double y) {
		this.x = x;
		this.y = y;
	}
	public DPoint() {
		this(0.0, 0.0);
	}
}
