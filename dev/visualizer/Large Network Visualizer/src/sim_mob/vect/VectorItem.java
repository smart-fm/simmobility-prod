package sim_mob.vect;

public class VectorItem {
	//No-argument constructor; required for gson
	private VectorItem() {}
	
	private String bkgrd;
	
	private String[] gradient;
	
	private String stroke;
	
	private float width;
	
	private String shape;
	
	private int[] points;
	
	public String getBkgrd() { return bkgrd; }
	public String[] getGradient() { return gradient; }
	public String getStroke() { return stroke; }
	public float getWidth() { return width; }
	public String getShape() { return shape; }
	public int[] getPoints() { return points; }
}
