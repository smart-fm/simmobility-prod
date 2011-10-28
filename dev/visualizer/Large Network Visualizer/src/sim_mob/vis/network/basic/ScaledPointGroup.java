package sim_mob.vis.network.basic;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashSet;


/**
 * This is a simple class which allows scaled points to belong to groups. A group of points will
 * scale on demand according to the globally expected scale value.
 */
public class ScaledPointGroup {
	//Saved variables.
	private static ScaleContext GlobalScaleContext;
	
	//Internal
	private ScaleContext localScaleContext;
	private HashSet<WeakReference<ScaledPoint>> points = new HashSet<WeakReference<ScaledPoint>>();
	
	
	/**
	 * Update the scale values of all points. This will happen over time, 
	 * as points check their contexts against the global scale context.
	 */
	public static void SetNewScaleContext(ScaleContext context) {
		GlobalScaleContext = context;
	}
	
	/**
	 * Actually update all scaled points in this context. Checks first if a scale is needed.
	 */
	public void synchScale() {
		if (localScaleContext!=null && localScaleContext.sameScale(GlobalScaleContext)) {
			return;
		}
		
		//To trim
		ArrayList<WeakReference<ScaledPoint>> toRemove = new ArrayList<WeakReference<ScaledPoint>>();
		
		//Save, update.
		localScaleContext = GlobalScaleContext;
		for (WeakReference<ScaledPoint> wrpt : points) {
			ScaledPoint pt = wrpt.get();
			if (pt!=null) {
				pt.scaleVia(localScaleContext.origin, localScaleContext.farthestPoint, localScaleContext.canvasWidth, localScaleContext.canvasHeight);
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
		return GlobalScaleContext.canvasHeight;
	}

}
