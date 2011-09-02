package sim_mob.vis.network;

import java.util.ArrayList;
import java.util.HashSet;

import sim_mob.vis.util.IntGetter;


/**
 * A position which can be dynamically rescaled. By default, the "scaled" position is always returned.
 * This position also flips the Y axis to be consistent with Cartesian co-ordiates.
 */
public class ScaledPoint {
	//private static final int BUFFER = 95; //NOTE: Do this yourself!
	public static IntGetter CanvasWidth;
	public static IntGetter CanvasHeight;
	
	private static HashSet<ScaledPoint> allPoints = new HashSet<ScaledPoint>();
	
	private DPoint orig;
	private DPoint scaled;
	  
	public ScaledPoint(double x, double y) {
		orig = new DPoint(x, y);
		scaled = new DPoint();
		
		//Bookkeeping
		allPoints.add(this);
	}
	
	//NOTE: This will never happen; we need a better way to deal with "old" ScaledPoints
	//      For now, just avoid creating scaled points that you will then throw away.
	/*protected void finalize() throws Throwable {
		//Bookkeeping
		allPoints.remove(this);
	}*/
	
	//Helper: Rescale all known points
	//NOTE: xScale.x=min, xScale.y=max. Not very intuitive, I know.
	public static void ScaleAllPoints(DPoint xScale, DPoint yScale) {
		int width = CanvasWidth.get();
		int height = CanvasHeight.get();
		for (ScaledPoint pt : allPoints) {
			pt.scaleTo(xScale, yScale, width,  height);
		}
	}
	  
	public double getX() {
		return scaled.x;
	}
	
	public double getY() {
		return CanvasHeight.get() - scaled.y;
	}
	 
	public void scaleTo(DPoint xBounds, DPoint yBounds, int width, int height) {
		scaled.x = scalePointForDisplay(orig.x, xBounds.x, xBounds.y, width);
		scaled.y = scalePointForDisplay(orig.y, yBounds.x, yBounds.y, height);
	}

	public int scalePointForDisplay(double orig, double min, double max, int scaleArea) {
		double percent = (orig - min) / (max - min);
		//int scaledMagnitude = ((int)scaleArea * ScaledPoint.BUFFER) / 100;
		int newVal = (int)(percent * scaleArea) + ((int)scaleArea-scaleArea)/2;
		return newVal;
	}
}


