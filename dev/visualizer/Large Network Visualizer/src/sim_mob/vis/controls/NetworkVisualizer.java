package sim_mob.vis.controls;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.*;

import sim_mob.vis.network.basic.*;
import sim_mob.vis.network.*;


/**
 * Represents an actual visualizer for the network. Handles scaling, etc. 
 * TODO: When zoomed in, it still draws the entire map. May want to "tile" drawn sections.
 */
public class NetworkVisualizer {
	private RoadNetwork source;
	private BufferedImage buffer;
	private int width100Percent;
	private int height100Percent;
	double currPercentZoom;
	
	//More generic resources
	//private Stroke pt1Stroke;
	//private Stroke pt2Stroke;
	
	
	public NetworkVisualizer() {
		//pt1Stroke = new BasicStroke(1.0F);
		//pt2Stroke = new BasicStroke(2.0F);
	}
	
	public BufferedImage getImage() {
		return buffer;
	}
	
	public void setSource(RoadNetwork source, double initialZoom, int width100Percent, int height100Percent) {
		//Save
		this.source = source;
		this.width100Percent = width100Percent;
		this.height100Percent = height100Percent;
		
		//Recalc
		redrawAtScale(initialZoom);
	}
	
	//Negative numbers mean zoom out that many times.
	public void zoomIn(int number) {
		//Each tick increases zoom by 10%
		redrawAtScale(currPercentZoom + currPercentZoom*number*0.10);
	}
	
	public void redrawAtScale(double percent) {
		//Save
		currPercentZoom = percent;
		
		//Determine the width and height of our canvas.
		int width = (int)(width100Percent * percent);
		int height = (int)(height100Percent * percent);
		buffer = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		
		//Make sure our canvas is always slightly bigger than the original size...
		double width5Percent = 0.05 * (source.getLowerRight().x - source.getTopLeft().x);
		double height5Percent = 0.05 * (source.getLowerRight().y - source.getTopLeft().y);
		DPoint newTL = new DPoint(source.getTopLeft().x-width5Percent, source.getTopLeft().y-height5Percent);
		DPoint newLR = new DPoint(source.getLowerRight().x+width5Percent, source.getLowerRight().y+height5Percent);
		
		//Scale all points
		ScaledPoint.ScaleAllPoints(newTL, newLR, width, height);
		
		//Retrieve a graphics object; ensure it'll anti-alias
		Graphics2D g = (Graphics2D)buffer.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Fill the background
		g.setBackground(Color.WHITE);
		g.clearRect(0, 0, width, height);
		
		//Draw nodes
		for (Node n : source.getNodes().values()) {
			n.draw(g);
		}
		
		//Draw segments
		for (Segment sn : source.getSegments().values()) {
			sn.draw(g);
		}
		
		//Draw links
		for (Link ln : source.getLinks().values()) {
			ln.draw(g);
		}
		
		//Names go on last; make sure we don't draw them twice...
		Set<String> alreadyDrawn = new HashSet<String>();
		for (Link ln : source.getLinks().values()) {
			String key1 = ln.getName() + ln.getStart().toString() + ":" + ln.getEnd().toString();
			String key2 = ln.getName() + ln.getEnd().toString() + ":" + ln.getStart().toString();
			if (alreadyDrawn.contains(key1) || alreadyDrawn.contains(key2)) {
				continue;
			}
			alreadyDrawn.add(key1);

			ln.drawName(g);
		}
		
	}
	

}

