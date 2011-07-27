package sim_mob;

import java.awt.Color;
import java.awt.geom.Ellipse2D;

///Lightweight class to help us represent traffic lights.
public class TrafficLight extends Ellipse2D.Double {
	private static final long serialVersionUID = 1L;
	
	public TrafficLight(double x, double y, double size) {
		super(x, y, size, size);
		lightColor = Color.RED;
	}
	
	public Color lightColor;

}
