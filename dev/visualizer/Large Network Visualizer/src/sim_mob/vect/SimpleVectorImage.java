package sim_mob.vect;

import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.util.*;

import com.google.gson.Gson;

public class SimpleVectorImage {
	//No-argument constructor; required for gson
	private SimpleVectorImage() {
		for (int i=0; i<rotatedBuffers.length; i++) {
			rotatedBuffers[i] = new CachedBuffer();
		}
	}
	
	private CoordinateSystem coordinates;
	
	private IndexedColor[] colors;
	
	private VectorItem[] drawOrder;
	
	//Lookup:
	private Hashtable<String, Color> indexedColors;
	
	//Cached buffers
	class CachedBuffer {
		double scaleAmt;
		BufferedImage img;
	}
	private CachedBuffer[] rotatedBuffers = new CachedBuffer[360];
	
	//Helper
	private static Gson gson = new Gson();
	
	//Load from file
	public static SimpleVectorImage LoadFromFile(BufferedReader file) {
		return gson.fromJson(file, SimpleVectorImage.class);
	}
	
	//Finalize this car's color scheme, substituting user preferences as appropriate.
	public void buildColorIndex(Hashtable<String, Color> userOverrides) {
		indexedColors = new Hashtable<String, Color>();
		
		//Add every color in the list of IndexedColors; override with user-specified overrides if they exist.
		for (IndexedColor clr : colors) {
			String key = clr.getId();
			if (userOverrides.containsKey(key)) {
				indexedColors.put(key, userOverrides.get(key));
			} else {
				String rgb = clr.getRGB();
				if (rgb==null) {
					throw new RuntimeException("Can't build color index; no default color: " + key);
				}
				indexedColors.put(key, new Color(Integer.parseInt(rgb, 0x10)));
			}
		}
	}
	
	//Helper: Make all colors slightly translucent.
	public void phaseColors(int alpha) {
		String[] oldKeys = indexedColors.keySet().toArray(new String[]{});
		for (String key : oldKeys) {
			Color orig = indexedColors.get(key);
			indexedColors.put(key, new Color(orig.getRed(), orig.getGreen(), orig.getBlue(), alpha));
		}
	}
	
	//Retrieve a scaled, rotated version of this image.
	public BufferedImage getImage(double scaleFactor, int rotateAngle) {
		//Bound, retrieve
		CachedBuffer cacheEntry = rotatedBuffers[rotateAngle%360];
		
		//Do we need to refresh the image cache?
		if (cacheEntry.img==null || Math.abs(scaleFactor-cacheEntry.scaleAmt)>1E-7) { //Just hard-code an epsilon value for now.
			cacheEntry.scaleAmt = scaleFactor;
			redrawAtScale(cacheEntry, rotateAngle);
		}
		
		return cacheEntry.img;
	}
	
	//Expand a bounding box based on rotation
	private void expandBox(double[] size, double angle) {
		//We have four points to consider. Assume the rectangle is situated with (0,0) as its top-left corner.
		Point2D[] candidates = new Point2D[] {
			new Point2D.Double(0, 0),
			new Point2D.Double(size[0], 0),
			new Point2D.Double(0, size[1]),
			new Point2D.Double(size[0], size[1])
		};
		
		//From this, we need to retrieve the minimum and maximum elements
		Point2D center = new Point2D.Double(size[0]/2, size[1]/2);
		double minX = center.getX();
		double maxX = center.getX();
		double minY = center.getY();
		double maxY = center.getY();
		
		//Transform each coordinate and consider if it changes the min/max.
		for (Point2D pt : candidates) {
			double vx = pt.getX() - center.getX();
			double vy = pt.getY() - center.getY();
			Point2D trans = new Point2D.Double(center.getX() + Math.cos(angle)*vx - Math.sin(angle)*vy, center.getY() + Math.sin(angle)*vx + Math.cos(angle)*vy);
			minX = Math.min(minX, trans.getX());
			maxX = Math.max(maxX, trans.getX());
			minY = Math.min(minY, trans.getY());
			maxY = Math.max(maxY, trans.getY());
		}
		
		//Now simply compute the size of the resulting box.
		size[0] = maxX - minX;
		size[1] = maxY - minY;
	}
	
	//Redraw all 360 degree rotations of this image at this scale.
	private void redrawAtScale(CachedBuffer entry, int angle) {
		//Based on our scale factor, take a guess at the correct size of the output rectangle.
		double[] scaleSizeD = new double[]{coordinates.getWidth()*entry.scaleAmt, coordinates.getHeight()*entry.scaleAmt};
		
		//Now modify this based on the current rotation.
		expandBox(scaleSizeD, (Math.PI*angle)/180);
		
		//Make a new image to hold this.
		entry.img = new BufferedImage((int)Math.ceil(scaleSizeD[0])+1, (int)Math.ceil(scaleSizeD[1])+1, BufferedImage.TYPE_INT_ARGB);
		Graphics2D g = (Graphics2D)entry.img.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Create a transformation for this angle/scale. Translate to the center of the image.
		AffineTransform at = AffineTransform.getTranslateInstance(entry.img.getWidth()/2, entry.img.getHeight()/2);
		at.scale(entry.scaleAmt, entry.scaleAmt);
		at.rotate((Math.PI*angle)/180);
		g.setTransform(at);
		
		//Now draw it.
		draw(g);
	}
	
	//Draw a single instance.
	private void draw(Graphics2D g) {
		//Amount to subtract from each component to get a centered version.
		float[] off = new float[]{(int)Math.ceil(coordinates.getWidth()/2.0), (int)Math.ceil(coordinates.getHeight()/2.0)};
		
		//Draw each VectorItem
		for (VectorItem item : drawOrder) {
			//Draw the background.
			if (!item.getShape().equals("line")) {
				if (item.getBkgrd()!=null) {
					g.setColor(getColor(item.getBkgrd()));
				} else if (item.getGradient()!=null) {
					//TODO: For now, gradients are assumed to always have two points only.
					if (item.getGradient().length!=2) {
						throw new RuntimeException("Error: For now gradients can only have two points.");
					}
					
					GradientPaint gp = makeGP(item.getPoints(), getColor(item.getGradient()[0]), getColor(item.getGradient()[1]));
					g.setPaint(gp);
					
					//Get backup.
					//g.setColor(getColor(item.getGradient()[0]));
				} else {
					throw new RuntimeException("No background or gradient for item.");
				}
				fillPoints(g, item.getShape(), item.getPoints(), off);
			}
			
			//Draw the stroke
			g.setColor(getColor(item.getStroke()));
			g.setStroke(new BasicStroke(item.getWidth()));
			drawPoints(g, item.getShape(), item.getPoints(), off);
		}
	}
	
	private GradientPaint makeGP(float[] points, Color startColor, Color endColor) {
		//For now, all gradients start at left-middle and go to right-middle. This can be added to the json file later.
		double minX = java.lang.Double.MAX_VALUE;
		double maxX = java.lang.Double.MIN_VALUE;
		double minY = java.lang.Double.MAX_VALUE;
		double maxY = java.lang.Double.MIN_VALUE;
		for (int i=0; i<points.length/2; i++) {
			minX = Math.min(minX, points[i*2]);
			maxX = Math.max(maxX, points[i*2]);
			minY = Math.min(minY, points[i*2+1]);
			maxY = Math.max(maxY, points[i*2+1]);
		}
		
		return new GradientPaint((float)minX, (float)(minY + (maxY-minY)/2), startColor, (float)maxX, (float)(minY + (maxY-minY)/2), endColor, false);		
	}

	//TEMP: Draw a shape
	private void fillPoints(Graphics2D g, String shapeType, float[] points, float[] off) {
		if (shapeType.equals("poly")) {
			int[] xPoints = new int[points.length/2];
			int[] yPoints = new int[points.length/2];
			splitToPlanarArrays(points, off, xPoints, yPoints);
			g.fillPolygon(xPoints, yPoints, xPoints.length);
		} else {
			throw new RuntimeException("Unknown shape: " + shapeType);
		}
	}
	
	//TEMP: Draw a shape
	private void drawPoints(Graphics2D g, String shapeType, float[] points, float[] off) {
		if (shapeType.equals("poly")) {
			int[] xPoints = new int[points.length/2];
			int[] yPoints = new int[points.length/2];
			splitToPlanarArrays(points, off, xPoints, yPoints);
			g.drawPolygon(xPoints, yPoints, xPoints.length);
		} else if (shapeType.equals("line")) {
			if (points.length!=4) {
				throw new RuntimeException("Points array must contain four items for a line.");
			}
			g.drawLine((int)(points[0]-off[0]), (int)(points[1]-off[1]), (int)(points[2]-off[0]), (int)(points[3]-off[1]));
		} else {
			throw new RuntimeException("Unknown shape: " + shapeType);
		}
	}
	
	//TEMP
	private void splitToPlanarArrays(float[] orig, float[] off, int[] plane1, int[] plane2) {
		if (orig.length%2!=0) {
			throw new RuntimeException("Can't split array into planes; its size is not even.");
		}
		for (int i=0; i<orig.length/2; i++) {
			plane1[i] = (int)(orig[i*2]-off[0]);
			plane2[i] = (int)(orig[i*2 +1]-off[1]);
		}
	}
	
	//TEMP: Inefficient.
	private Color getColor(String key) {
		//Failsafe; just use the default color scheme.
		if (indexedColors==null) {
			buildColorIndex(new Hashtable<String, Color>());
		}
		
		if (!indexedColors.containsKey(key)) {
			throw new RuntimeException("Invalid color: " + key);
		}
		
		return indexedColors.get(key);
	}
}

