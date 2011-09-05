package sim_mob.vis.network.basic;


import java.util.*;
import java.lang.ref.WeakReference;


/**
 * A position which can be dynamically rescaled. By default, the "scaled" position is always returned.
 * This position also flips the Y axis to be consistent with Cartesian co-ordiates.
 */
public class ScaledPoint {
	private static double LastScaledHeight;
	private static HashSet<WeakReference<ScaledPoint>> allPoints = new HashSet<WeakReference<ScaledPoint>>();
	
	private DPoint orig;
	private DPoint scaled;
	  
	public ScaledPoint(double x, double y) {
		orig = new DPoint(x, y);
		scaled = new DPoint();
		
		//Bookkeeping
		allPoints.add(new WeakReference<ScaledPoint>(this));
	}
	
	//Helper: Rescale all known points
	public static void ScaleAllPoints(DPoint origin, DPoint farthestPoint, double canvasWidth, double canvasHeight) {
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
	}
	  
	public double getX() {
		return scaled.x;
	}
	
	public double getY() {
		return LastScaledHeight - scaled.y;
	}
	
	public double getUnscaledX() {
		return orig.x;
	}
	
	public double getUnscaledY() {
		return orig.y;
	}
	 
	private void scaleVia(DPoint topLeft, DPoint lowerRight, double newWidth, double newHeight) {
		scaled.x = scaleValue(orig.x, topLeft.x, lowerRight.x-topLeft.x, newWidth);
		scaled.y = scaleValue(orig.y, topLeft.y, lowerRight.y-topLeft.y, newHeight);
	}
	
	private static double scaleValue(double value, double min, double extent, double newExtent) {
		//What percent of the original size are we taking up?
		double percent = (value-min)/extent;
		return percent * newExtent;
	}
}


