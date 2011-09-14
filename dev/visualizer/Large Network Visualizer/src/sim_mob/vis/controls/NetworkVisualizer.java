package sim_mob.vis.controls;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.*;

import sim_mob.vis.network.basic.*;
import sim_mob.vis.network.*;
import sim_mob.vis.simultion.AgentTick;
import sim_mob.vis.simultion.SignalTick;
import sim_mob.vis.simultion.SimulationResults;


/**
 * Represents an actual visualizer for the network. Handles scaling, etc. 
 * TODO: When zoomed in, it still draws the entire map. May want to "tile" drawn sections.
 */
public class NetworkVisualizer {
	private RoadNetwork network;
	private SimulationResults simRes;
	private int currFrameTick;
	private BufferedImage buffer;
	private int width100Percent;
	private int height100Percent;
	double currPercentZoom;
	
	public int getCurrFrameTick() { return currFrameTick; }
	public boolean incrementCurrFrameTick(int amt) {
		return setCurrFrameTick(currFrameTick+amt);
	}
	public boolean setCurrFrameTick(int newVal) {
		if (newVal<0 || simRes==null || newVal>=simRes.ticks.size()) {
			return false;
		}
		currFrameTick = newVal;
		return true;
	}
	
	//For clicking
	private static final double NEAR_THRESHHOLD = 20;
	
	
	public NetworkVisualizer() {
	}
	
	public BufferedImage getImage() {
		return buffer;
	}
	
	
	//Retrieve the node at the given screen position, or null if there's none.
	public Node getNodeAt(Point screenPos) {
		//First, convert the screen coordinates to centimeters
		//DPoint realPos = new DPoint();  //Note: Might be needed later; for now, we can just use ScaledPoints
		for (Node n : network.getNodes().values()) {
			if (screenPos.distance(n.getPos().getX(), n.getPos().getY()) <= NEAR_THRESHHOLD) {
				return n;
			}
		}
		return null;
	}
	
	
	public void setSource(RoadNetwork network, SimulationResults simRes, double initialZoom, int width100Percent, int height100Percent) {
		//Save
		this.network = network;
		this.simRes = simRes;
		this.currFrameTick = 0;
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
		double width5Percent = 0.05 * (network.getLowerRight().x - network.getTopLeft().x);
		double height5Percent = 0.05 * (network.getLowerRight().y - network.getTopLeft().y);
		DPoint newTL = new DPoint(network.getTopLeft().x-width5Percent, network.getTopLeft().y-height5Percent);
		DPoint newLR = new DPoint(network.getLowerRight().x+width5Percent, network.getLowerRight().y+height5Percent);
		
		//Scale all points
		ScaledPoint.ScaleAllPoints(newTL, newLR, width, height);
		redrawAtCurrScale();
	}
	
	public void redrawAtCurrScale() {
		//Retrieve a graphics object; ensure it'll anti-alias
		Graphics2D g = (Graphics2D)buffer.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Fill the background
		g.setBackground(Color.WHITE);
		g.clearRect(0, 0, buffer.getWidth(), buffer.getHeight());
		
		//Draw nodes
		for (Node n : network.getNodes().values()) {
			n.draw(g);
		}
		
		//Draw segments
		for (Segment sn : network.getSegments().values()) {
			sn.draw(g);
		}
		
		//Draw links
		for (Link ln : network.getLinks().values()) {
			ln.draw(g);
		}
		
		//Names go on last; make sure we don't draw them twice...
		Set<String> alreadyDrawn = new HashSet<String>();
		for (Link ln : network.getLinks().values()) {
			String key1 = ln.getName() + ln.getStart().toString() + ":" + ln.getEnd().toString();
			String key2 = ln.getName() + ln.getEnd().toString() + ":" + ln.getStart().toString();
			if (alreadyDrawn.contains(key1) || alreadyDrawn.contains(key2)) {
				continue;
			}
			alreadyDrawn.add(key1);

			ln.drawName(g);
		}
		
		//Now draw simulation data: cars, etc.
		for (AgentTick at : simRes.ticks.get(currFrameTick).agentTicks.values()) {
			at.draw(g);
		}
		
	}
	
	
	//Hackish
	void drawTrafficLights(Graphics2D g) {
		for (SignalTick sg : simRes.ticks.get(currFrameTick).signalTicks.values()) {
			sg.draw(g);
		}
	}
	

}

