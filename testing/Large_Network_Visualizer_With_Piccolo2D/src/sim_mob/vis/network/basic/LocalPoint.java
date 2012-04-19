package sim_mob.vis.network.basic;

public class LocalPoint {

	private DPoint orig;
	private DPoint scaled;
	
	public double getX(){
		return scaled.x;
	}
	public double getY(){
		return scaled.y;
	}
	
	public double getUnscaledX(){
		return orig.x;
	}
	public double getUnscaledY(){
		return orig.y;
	}
	
	public LocalPoint (double x, double y){
		orig = new DPoint(x, y);
		scaled = new DPoint();
	}
	
	public void scaleVia(DPoint topLeft, DPoint lowerRight, double newWidth, double newHeight) {
		scaled.x = scaleValue(orig.x, topLeft.x, lowerRight.x-topLeft.x, newWidth);
		scaled.y = newHeight - scaleValue(orig.y, topLeft.y, lowerRight.y-topLeft.y, newHeight);
	}
	
	private static double scaleValue(double value, double min, double extent, double newExtent) {
		//What percent of the original size are we taking up?
		double percent = (value-min)/extent;
		return percent * newExtent;
	}
	
	
}
