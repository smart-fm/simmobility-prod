package sim_mob.vect;

import java.awt.*;
import java.io.BufferedReader;

import com.google.gson.Gson;

public class SimpleVectorImage {
	//No-argument constructor; required for gson
	private SimpleVectorImage() {}
	
	private CoordinateSystem coordinates;
	
	private IndexedColor[] colors;
	
	private VectorItem[] drawOrder;
	
	//Helper
	private static Gson gson = new Gson();
	
	//Load from file
	public static SimpleVectorImage LoadFromFile(BufferedReader file) {
		return gson.fromJson(file, SimpleVectorImage.class);
	}
	
	//TEMP: Just draw it.
	public void draw(Graphics2D g) {
		for (VectorItem item : drawOrder) {
			//Draw the background.
			if (!item.getShape().equals("line")) {
				if (item.getBkgrd()!=null) {
					g.setColor(getColor(item.getBkgrd()));
				} else if (item.getGradient()!=null) {
					//TODO: Gradients
					g.setColor(getColor(item.getGradient()[0]));
				} else {
					throw new RuntimeException("No background or gradient for item.");
				}
				fillPoints(g, item.getShape(), item.getPoints());
			}
			
			//Draw the stroke
			g.setColor(getColor(item.getStroke()));
			g.setStroke(new BasicStroke(item.getWidth()));
			drawPoints(g, item.getShape(), item.getPoints());
		}
	}
	
	//TEMP: Draw a shape
	private void fillPoints(Graphics2D g, String shapeType, int[] points) {
		if (shapeType.equals("poly")) {
			int[] xPoints = new int[points.length/2];
			int[] yPoints = new int[points.length/2];
			splitToPlanarArrays(points, xPoints, yPoints);
			g.fillPolygon(xPoints, yPoints, xPoints.length);
		} else {
			throw new RuntimeException("Unknown shape: " + shapeType);
		}
	}
	
	//TEMP: Draw a shape
	private void drawPoints(Graphics2D g, String shapeType, int[] points) {
		if (shapeType.equals("poly")) {
			int[] xPoints = new int[points.length/2];
			int[] yPoints = new int[points.length/2];
			splitToPlanarArrays(points, xPoints, yPoints);
			g.drawPolygon(xPoints, yPoints, xPoints.length);
		} else if (shapeType.equals("line")) {
			if (points.length!=4) {
				throw new RuntimeException("Points array must contain four items for a line.");
			}
			g.drawLine(points[0], points[1], points[2], points[3]);
		} else {
			throw new RuntimeException("Unknown shape: " + shapeType);
		}
	}
	
	//TEMP
	private void splitToPlanarArrays(int[] orig, int[] plane1, int[] plane2) {
		if (orig.length%2!=0) {
			throw new RuntimeException("Can't split array into planes; its size is not even.");
		}
		for (int i=0; i<orig.length/2; i++) {
			plane1[i] = orig[i*2];
			plane2[i] = orig[i*2 +1];
		}
	}
	
	//TEMP: Inefficient.
	private Color getColor(String key) {
		for (IndexedColor clr : colors) {
			if (clr.getId().equals(key)) {
				String rgb = clr.getRGB();
				if (rgb==null) {
					throw new RuntimeException("Can't draw color, is null: " + key);
				}
				return new Color(Integer.parseInt(rgb, 0x10));
			}
		}
		throw new RuntimeException("Invalid color: " + key);
	}
}

