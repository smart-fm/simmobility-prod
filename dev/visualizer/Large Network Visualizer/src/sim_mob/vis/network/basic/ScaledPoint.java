package sim_mob.vis.network.basic;

import java.awt.geom.Point2D;


/**
 * A position which can be dynamically rescaled.
 * 
 * \note
 * If you're reading points in from a file, consider using FlippedScaledPoint. 
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
	//private static ScaledPointGroup GlobalGroup = new ScaledPointGroup();
	
	//Current, shared scale/translate factors
	private static Point2D scaleFactors;
	private static Point2D translateFactors;
	//private static double scaledCanvasHeight;
	public static void updateScaleAndTranslate(Point2D scaleFact, Point2D translateFact/*, double scaledHeight*/) {
		scaleFactors = scaleFact;
		translateFactors = translateFact;
		//scaledCanvasHeight = scaledHeight;
	}
	public static Point2D getScaleFactor() {
		return new Point2D.Double(scaleFactors.getX(), scaleFactors.getY());
	}
	
	
	//Original Point value.
	private DPoint orig;
	//private DPoint scaled;
	
	//The group used to perform the scaling.
	//private ScaledPointGroup group;
	
	/*public static void ClearGlobalGroup() {
		GlobalGroup = new ScaledPointGroup();
	}*/
	  
	/**
	 * Create a new ScaledPoint at the given x and y coordinates, belonging to a given ScaledPointGroup. If null, use the glboal group. 
	 */
	public ScaledPoint(double x, double y) {
		orig = new DPoint(x, y);
		//scaled = new DPoint();
		//group = scaleGroup;
		
		//Manage the global group.
		/*if (group==null) {
			group = GlobalGroup;
		}*/
		
		//Bookkeeping
		//group.addPoint(this);
		//allPoints.add(new WeakReference<ScaledPoint>(this));
	}
	
	
	///Retrieve the original, unscaled values of x and y
	public double getUnscaledX() { return orig.x; }
	public double getUnscaledY() { return orig.y; }
	

	///Retrieve the scaled and translated values of x and y. 
	///These values can be directly displayed to the screen.
	public double getX() {
//		if(ScaledPoint.scaleFactors == null) System.out.println("ScaledPoint.scaleFactors Null");
//		if(ScaledPoint.translateFactors == null) System.out.println("ScaledPoint.translateFactors Null");
		double scaledX = orig.x*ScaledPoint.scaleFactors.getX() - ScaledPoint.translateFactors.getX();
		return scaledX;
	}
	
	public double getY() {
		double scaledY = orig.y*ScaledPoint.scaleFactors.getY() - ScaledPoint.translateFactors.getY();
		//System.out.println("Scale by: " + ScaledPoint.scaleFactors.getY() + ", translate by: " + ScaledPoint.translateFactors.getY());
		//Y is slightly different, since its axis is flipped.
		return scaledY;
		//return ScaledPoint.scaledCanvasHeight - scaledY;
	}
	
	 
	/**
	 * Actually perform the scale. This method is called by ScaledPointGroup.
	 * 
	 * \param scaleFactors Amounts to scale each axis by. 
	 * Multiply each "original" component by these values.
	 * 
	 * \param translateFactors The scaled coordinates of the top-left corner of the screen. 
	 * Once scaled,  subtract this value from each component. Then, "scaled" will contain
	 * the position within the canvas. 
	 */
	/*void update(Point2D scaleFactors, Point2D translateFactors) { 
		scaled.x = orig.x * xFactor;
		scaled.y = orig.y * yFactor;
	}*/

}

