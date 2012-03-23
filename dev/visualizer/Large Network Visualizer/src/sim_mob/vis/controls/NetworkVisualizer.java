package sim_mob.vis.controls;
import java.awt.*;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Point2D.Double;
import java.awt.geom.Point2D.Float;
import java.awt.image.BufferedImage;
import java.util.*;
import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.*;
import sim_mob.vis.network.*;
import sim_mob.vis.simultion.*;


/**
 * Represents an actual visualizer for the network. Handles scaling, etc. 
 * TODO: When zoomed in, it still draws the entire map. May want to "tile" drawn sections.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class NetworkVisualizer {
	private RoadNetwork network;
	private SimulationResults simRes;
	private BufferedImage buffer;
	private int width100Percent;
	private int height100Percent;
	double currPercentZoom;
	private static final double  ZOOM_IN_CRITICAL = 1.6;
	private String fileName;
	private boolean showFakeAgent;
	private boolean debugOn = false;
	
	private Stroke str1pix = new BasicStroke(1);
	
	private int currHighlightID;
	public void setHighlightID(int id) {
		currHighlightID=id; 
	}
	
	private int scaleMult;
	public void setScaleMultiplier(int val) { scaleMult = val; }
	
	
	public int getMaxFrameTick() { return simRes.ticks.size()-1; }
	public String getFileName(){return fileName;}
	
	//For clicking
	private static final double NEAR_THRESHHOLD = 20;
	
	private static final double Distance(double x1, double y1, double x2, double y2) {
		double dX = x1-x2;
		double dY = y1-y2;
		return Math.sqrt(dX*dX + dY*dY);
	}
	private static final double Distance(Point2D start, Point2D end) {
		return Distance(start.getX(), start.getY(), end.getX(), end.getY());
	}
	
	private static final Ellipse2D CircleFromPoints(Point2D min, Point2D max, double radius) {
		Point2D center = new Point2D.Double(min.getX()+(max.getX()-min.getX())/2, min.getY()+(max.getY()-min.getY())/2);
		Ellipse2D el = new Ellipse2D.Double(center.getX()-radius, center.getY()-radius, radius*2, radius*2);
		return el;
	}
		
	public NetworkVisualizer() {
		this.currHighlightID = -1;
		this.scaleMult = 1;
	}
	
	public BufferedImage getImage() {

		return buffer;
	}

	public BufferedImage getImageAtTimeTick(int tick) {
		return getImageAtTimeTick(tick, BufferedImage.TYPE_INT_RGB);
	}
	public BufferedImage getImageAtTimeTick(int tick, int imageType) {
		BufferedImage res = new BufferedImage(buffer.getWidth(), buffer.getHeight(), imageType);
		redrawAtCurrScale(res, tick);
		return res;
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
	
	public void setSource(RoadNetwork network, SimulationResults simRes, double initialZoom, int width100Percent, int height100Percent,String fileName) {
		//Save
		this.network = network;
		this.simRes = simRes;
		this.width100Percent = width100Percent;
		this.height100Percent = height100Percent;
		this.fileName = fileName;
		this.showFakeAgent = false;
		//this.debugOn = false;
		
		//Recalc
		redrawAtScale(initialZoom, 0);
	}
	
	//Negative numbers mean zoom out that many times.
	public void zoomIn(int number, int frameTick) {
		//Each tick increases zoom by 10%
		redrawAtScale(currPercentZoom + currPercentZoom*number*0.10, frameTick);
	}
	
	public void squareZoom(int frameTick) {
		//First, square it.
		int min100Percent = Math.min(width100Percent, height100Percent);
		if (width100Percent != height100Percent) {
			width100Percent = min100Percent;
			height100Percent = min100Percent;
		}
		
		//Now redraw
		redrawAtScale(currPercentZoom, frameTick);
	}
	
	public void toggleFakeAgent(boolean drawFakeAgent, int frameTick){

		this.showFakeAgent = drawFakeAgent;			
		redrawAtCurrScale(frameTick);
	}
	
	public void toggleDebugOn(boolean debugOn, int frameTick){
		this.debugOn = debugOn;
		redrawAtCurrScale(frameTick);
	}
	
	public void redrawAtScale(double percent, int frameTick) {
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
		ScaledPointGroup.SetNewScaleContext(new ScaleContext(percent, newTL, newLR, width, height));
		//ScaledPoint.ScaleAllPoints(percent, newTL, newLR, width, height);
		redrawAtCurrScale(buffer, frameTick);
	}
	
	public void redrawAtCurrScale(int frameTick) {
		redrawAtCurrScale(buffer, frameTick);
	}
	
	private void redrawAtCurrScale(BufferedImage dest, int frameTick) {
		//System.out.println(" refresh");
		
		//Retrieve a graphics object; ensure it'll anti-alias
		Graphics2D g = (Graphics2D)dest.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Fill the background
		g.setBackground(MainFrame.Config.getBackground("network"));
		g.clearRect(0, 0, dest.getWidth(), dest.getHeight());
		
		//Draw nodes
		for (Node n : network.getNodes().values()) {
			if(currPercentZoom<=ZOOM_IN_CRITICAL){
				n.draw(g);
			}
			else{
				if(n.getIsUni()){
					continue;
				}
				else{
					n.draw(g);
				}		
			}
		}
		
		if(currPercentZoom <= ZOOM_IN_CRITICAL){
			//Draw segments
			for (Segment sn : network.getSegments().values()) {
				sn.draw(g);
			}
		}
		
		//Draw Cutline
		if(this.showFakeAgent){
			for(CutLine ctl : network.getCutLine().values()){
				ctl.draw(g);	
			}
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
			ln.drawName(g,currPercentZoom);
		}
		

		//Draw out lanes only it is zoom to certain scale
		if(currPercentZoom>ZOOM_IN_CRITICAL){
			//Draw Lanes
			for (Hashtable<Integer,LaneMarking> lineMarkingTable : network.getLaneMarkings().values()) {
				
				for(LaneMarking lineMarking : lineMarkingTable.values()){
					
					lineMarking.draw(g);
				}
			}
			//Draw Perdestrain Crossing
			for(Crossing crossing : network.getCrossings().values()){
				crossing.draw(g);
			}
			
			//Draw Crossing Light
			for(SignalLineTick at: simRes.ticks.get(frameTick).signalLineTicks.values()){
				//Get Intersection ID
				Intersection tempIntersection = network.getIntersection().get(at.getIntersectionID());
				
				ArrayList<Integer> allPedestrainLights = at.getPedestrianLights();

				//Get Crossing IDs
				ArrayList<Integer> crossingIDs = tempIntersection.getSigalCrossingIDs();

				//Draw Crossing Lights
				for(int i = 0;i<crossingIDs.size();i++){
					if(network.getTrafficSignalCrossing().containsKey(crossingIDs.get(i))){
						
						network.getTrafficSignalCrossing().get(crossingIDs.get(i)).drawSignalCrossing(g, allPedestrainLights.get(i));
					}
					else{
						System.out.println("Error");
					}
		
				}
				
			}
	
			//Now draw out signal
			for(SignalLineTick at: simRes.ticks.get(frameTick).signalLineTicks.values()){
				
				//Get Intersection ID
				Intersection tempIntersection = network.getIntersection().get(at.getIntersectionID());
				//Get Light color
				ArrayList<ArrayList<Integer>> allVehicleLights =  at.getVehicleLights();
				
				//Vehicle Light Colors
				ArrayList<Integer> vaLights = allVehicleLights.get(0);
				ArrayList<Integer> vbLights = allVehicleLights.get(1);
				ArrayList<Integer> vcLights = allVehicleLights.get(2);
				ArrayList<Integer> vdLights = allVehicleLights.get(3);

				//Vehicle Light Lines
				ArrayList<ArrayList<TrafficSignalLine>> vaSignalLine = tempIntersection.getVaTrafficSignal();
				ArrayList<ArrayList<TrafficSignalLine>> vbSignalLine = tempIntersection.getVbTrafficSignal();
				ArrayList<ArrayList<TrafficSignalLine>> vcSignalLine = tempIntersection.getVcTrafficSignal();
				ArrayList<ArrayList<TrafficSignalLine>> vdSignalLine = tempIntersection.getVdTrafficSignal();

				//Draw Vehicle Lights
				drawTrafficLines(g,vaSignalLine, vaLights);
				drawTrafficLines(g,vbSignalLine, vbLights);
				drawTrafficLines(g,vcSignalLine, vcLights);
				drawTrafficLines(g,vdSignalLine, vdLights);
				
			}
			
		}
		
		
		//Now draw simulation data: cars, etc.
		//if(currPercentZoom>ZOOM_IN_CRITICAL){
		//Use a scale multiplier to allow people to resize the agents as needed.
		double adjustedZoom = currPercentZoom * scaleMult;
		Dimension sz100Percent = new Dimension(width100Percent, height100Percent);
		
		//Draw all agent ticks
		Hashtable<Integer, AgentTick> agents = simRes.ticks.get(frameTick).agentTicks;
		Hashtable<Integer, AgentTick> trackings = simRes.ticks.get(frameTick).trackingTicks;
		for (Integer key : agents.keySet()) {
			//Draw the agent
			AgentTick at = agents.get(key);
			boolean highlight = this.debugOn || (key.intValue()==currHighlightID);
			at.draw(g,adjustedZoom,this.showFakeAgent,highlight, sz100Percent);
			
			//Draw the tracking agent, if it exists
			AgentTick tr = trackings.get(key);
			if (tr!=null) {
				//Draw it as a "fake" agent.
				tr.draw(g, adjustedZoom, true, false, sz100Percent);
				
				//Draw a circle and a line
				g.setColor(Color.green);
				g.setStroke(str1pix);
				Point2D min = new Point2D.Double(Math.min(at.getPos().getX(), tr.getPos().getX()), Math.min(at.getPos().getY(), tr.getPos().getY()));
				Point2D max = new Point2D.Double(Math.max(at.getPos().getX(), tr.getPos().getX()), Math.max(at.getPos().getY(), tr.getPos().getY()));
				double dist = Distance(min, max);
				if (dist>1.0) {
					double diam = Math.max(max.getX()-min.getX(), max.getY()-min.getY());
					Line2D line = new Line2D.Double(min.getX(), min.getY(), max.getX(), max.getY());
					Ellipse2D el = CircleFromPoints(min, max, diam/2);
					g.draw(el);
					g.draw(line);
				}
			}
		}

		//}		
		
	}
	
	private void drawTrafficLines(Graphics2D g,ArrayList<ArrayList<TrafficSignalLine>> signalLine, ArrayList<Integer> lightColors){
		
		for(int i = 0; i<signalLine.size();i++){
			
			//Left turn light
			if (signalLine.size()>0 && lightColors.size()>0 && signalLine.get(0).size()>0) {
				ArrayList<TrafficSignalLine> leftTurnLight = signalLine.get(0);
				leftTurnLight.get(0).drawPerLight(g, lightColors.get(0));
			}
			
			
			//Straight turn light
			if (signalLine.size()>1 && lightColors.size()>1 && signalLine.get(1).size()>0) {
				ArrayList<TrafficSignalLine> straightTurnLight = signalLine.get(1);				
				straightTurnLight.get(0).drawPerLight(g, lightColors.get(1));	
			}
			
			
			//Right turn light
			if (signalLine.size()>2 && lightColors.size()>2 && signalLine.get(2).size()>0) {
				ArrayList<TrafficSignalLine> rightTurnLight = signalLine.get(2);
				rightTurnLight.get(0).drawPerLight(g, lightColors.get(2));
			}
			
			
		}
				
	}
	
	public void drawTrafficPedestrainCross(Graphics2D g,ArrayList<TrafficSignalCrossing> signalPedestrainCrossing, ArrayList<Integer> lightColor){

		if(signalPedestrainCrossing != null && signalPedestrainCrossing.size() != 0 &&  lightColor!=null && lightColor.size() != 0
				 && signalPedestrainCrossing.size() == lightColor.size())
		{
			
			for(int i = 0; i<signalPedestrainCrossing.size();i++)
			{
				//Draw crossing signal
				signalPedestrainCrossing.get(i).drawSignalCrossing(g, lightColor.get(i));			
			}
			
		}
		else{
			System.out.println("Error, the signal and crossing are not corresponding to each other -- NetWorkVisualizer, drawTrafficPedestrainCross()");		
		}
		
	}

}

