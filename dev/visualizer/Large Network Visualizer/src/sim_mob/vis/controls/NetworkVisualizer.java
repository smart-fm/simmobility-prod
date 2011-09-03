package sim_mob.vis.controls;

import java.awt.BasicStroke;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Stroke;
import java.awt.image.BufferedImage;

import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.network.Link;
import sim_mob.vis.network.Node;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.basic.ScaledPoint;


/**
 * Represents an actual visualizer for the network. Handles scaling, etc. 
 * TODO: When zoomed in, it still draws the entire map. May want to "tile" drawn sections.
 */
public class NetworkVisualizer {
	private RoadNetwork source;
	private BufferedImage buffer;
	private int width100Percent;
	private int height100Percent;
	
	//More generic resources
	private Stroke pt1Stroke;
	private Stroke pt2Stroke;
	
	
	public NetworkVisualizer() {
		pt1Stroke = new BasicStroke(1.0F);
		pt2Stroke = new BasicStroke(2.0F);
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
	
	public void redrawAtScale(double percent) {
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
		
		//Draw links:
		for (Link ln : source.getLinks().values()) {
			ln.draw(g);
		}
		
	}
	

}

