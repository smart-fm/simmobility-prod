package sim_mob.vis.network.basic;


/**
 * A position which can be dynamically rescaled.
 * 
 *  \author Seth N. Hetu
 *  \author Zhang Shuai
 * 
 * By default, the "scaled" position is always returned.
 * This position also flips the Y axis to be consistent with Cartesian co-ordiates.
 */
public class ScaledPoint {
	//private static double LastScaledHeight;
	//private static HashSet<WeakReference<ScaledPoint>> allPoints = new HashSet<WeakReference<ScaledPoint>>();
	private static ScaledPointGroup GlobalGroup = new ScaledPointGroup();
	
	private DPoint orig;
	private DPoint scaled;
	private ScaledPointGroup group;
	
	public static void ClearGlobalGroup() {
		GlobalGroup = new ScaledPointGroup();
	}
	  
	/**
	 * Create a new ScaledPoint at the given x and y coordinates, belonging to a given ScaledPointGroup. If null, use the glboal group. 
	 */
	public ScaledPoint(double x, double y, ScaledPointGroup scaleGroup) {
		orig = new DPoint(x, y);
		scaled = new DPoint();
		group = scaleGroup;
		
		//Manage the global group.
		if (group==null) {
			group = GlobalGroup;
		}
		
		//Bookkeeping
		group.addPoint(this);
		//allPoints.add(new WeakReference<ScaledPoint>(this));
	}
	
	//Helper: Rescale all known points
	/*public static void ScaleAllPoints(DPoint origin, DPoint farthestPoint, double canvasWidth, double canvasHeight) {
		LastScaledHeight = canvasHeight;
		
		ArrayList<WeakReference<ScaledPoint>> retired = new ArrayList<WeakReference<ScaledPoint>>();
	
		for (WeakReference<ScaledPoint> pt : allPoints) {
			if (pt.get()!=null) {
				pt.get().scaleVia(origin, farthestPoint, canvasWidth, canvasHeight); 
			} else {
				retired.add(pt);
			}
		}
		
		for (WeakReference<ScaledPoint> pt : retired) {
			allPoints.remove(pt);
		}
	}*/
	  
	public double getX() {
		group.synchScale();
		return scaled.x;
	}
	
	public double getY() {
		group.synchScale();
		return group.getLastScaledHeight() - scaled.y;
	}
	
	public double getUnscaledX() {
		return orig.x;
	}
	
	public double getUnscaledY() {
		return orig.y;
	}
	 
	/**
	 * Actually perform the scale. This method is called by ScaledPointGroup
	 */
	/*Package-protected*/ void scaleVia(DPoint topLeft, DPoint lowerRight, double newWidth, double newHeight) {
		scaled.x = scaleValue(orig.x, topLeft.x, lowerRight.x-topLeft.x, newWidth);
		scaled.y = scaleValue(orig.y, topLeft.y, lowerRight.y-topLeft.y, newHeight);
	}
	
	private static double scaleValue(double value, double min, double extent, double newExtent) {
		//What percent of the original size are we taking up?
		double percent = (value-min)/extent;
		return percent * newExtent;
	}
}


