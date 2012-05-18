package sim_mob.vis.network.basic;
/*
import java.awt.geom.Point2D;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashSet;
*/

/**
 * This is a simple class which allows scaled points to belong to groups. 
 * 
 * \author Seth N. Hetu
 * 
 * A group of points will
 * scale on demand according to the globally expected scale value.
 */
/*public class ScaledPointGroup {
	//Saved variables.
	private static Point2D GlobalZoomLevel;
	private static double GlobalHeight;
	
	//Internal
	private Point2D localZoomLevel;
	private HashSet<WeakReference<ScaledPoint>> points = new HashSet<WeakReference<ScaledPoint>>();
	*/
	
	/**
	 * Update the scale values of all points. This will happen over time, 
	 * as points check their contexts against the global scale context.
	 */
	/*public static void SetNewScaleContext(Point2D zoomLevel, double totalHeight) {
		GlobalZoomLevel = zoomLevel;
		GlobalHeight = totalHeight;
	}*/
	
	/**
	 * Actually update all scaled points in this context. Checks first if a scale is needed.
	 */
	/*public void synchScale() {
		if (localZoomLevel!=null && GlobalZoomLevel!=null 
			&& (localZoomLevel.getX()==GlobalZoomLevel.getX())
			&& (localZoomLevel.getY()==GlobalZoomLevel.getY())) {
			return;
		}
		
		//To trim
		ArrayList<WeakReference<ScaledPoint>> toRemove = new ArrayList<WeakReference<ScaledPoint>>();
		
		//Save, update.
		localZoomLevel = GlobalZoomLevel;
		for (WeakReference<ScaledPoint> wrpt : points) {
			ScaledPoint pt = wrpt.get();
			if (pt!=null) {
				pt.scaleVia(localZoomLevel.getX(), localZoomLevel.getY());
			} else {
				toRemove.add(wrpt);
			}
		}
		
		//Remove any dangling references.
		for (WeakReference<ScaledPoint> sp : toRemove) {
			points.remove(sp);
		}
	}
	
	public void addPoint(ScaledPoint sp) {
		points.add(new WeakReference<ScaledPoint>(sp));
	}
	
	public double getLastScaledHeight() {
		return GlobalHeight * GlobalZoomLevel.getY();
	}

}
*/