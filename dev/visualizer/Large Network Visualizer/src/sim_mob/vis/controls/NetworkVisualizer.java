package sim_mob.vis.controls;
import java.awt.*;

import java.awt.image.BufferedImage;
import java.awt.geom.*;
import java.awt.geom.Rectangle2D.Double;
import java.util.*;
import java.util.List;

import javax.swing.UIDefaults.LazyValue;

import sim_mob.index.LazySpatialIndex;
import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.*;
import sim_mob.vis.network.*;
import sim_mob.vis.simultion.*;
import sim_mob.vis.util.Utility;


/**
 * Represents an actual visualizer for the network. Handles scaling, etc. 
 * TODO: When zoomed in, it still draws the entire map. May want to "tile" drawn sections.
 * 
 * \author Seth N. Hetu
 * \author Zhang Shuai
 */
public class NetworkVisualizer {
	//The zoom level beyond which we stop drawing Links and start drawing individual Lanes,
	// Crossing, and Signals.
	//TODO: This is currently compared against currPercentZoom, but in the future we should
	//      compare the effective zoom of a single, scaled meter. Currently, if you load in
	//      a very small and a very large network, they will not reach 'critical' zoom at the 
	//      same visible zoom level.  ~Seth
	private static final double  ZOOM_IN_CRITICAL = 1.6;
	
	//temp
	private static int count;
	
	//The distance (pixels) threshold used for calculating "clicks" on a given object. 
	//If the distance from the mouse to a given object is less than this value, it is 
	//considered to be "near" enough to click on. 
	private static final double NEAR_THRESHHOLD = 20;
	
	//Some saved strokes, brushes and colors. 
	private static final Stroke str1pix = new BasicStroke(1);
	private static final Color hlColor = new Color(0xFF, 0xAA, 0x55);
	
	//The current road network, the simulation results we are showing, and the buffer we are printing it on.
	private RoadNetwork network;
	private SimulationResults simRes;
	//private BufferedImage buffer;
	
	//Turns on drawing of cut lines and fake agents.
	private boolean showFakeAgent = false;
	
	//Used to turn on "debug" mode, which causes all Agents to be highlighted. 
	private boolean debugOn = false;
	
	//Render flags for annotation labels
	private boolean showAimsunLabels = false;
	private boolean showMitsimLabels = false;
	
	//The current real-coordinate bounds of the view.
	private Rectangle2D currView;
	
	//The size of the canvas at a zoom level of 1.0
	//private Dimension naturalSize;
	
	//Spatial indexes for our network items and our current agent tick.
	private LazySpatialIndex<DrawableItem> networkItemsIndex;
	private LazySpatialIndex<DrawableItem> agentTicksIndex;

	
	//The IDs of the current Agents to highlight. Any Agent whose ID is in this list
	// will be highlighted and drawn with a target sign overlaid.
	private HashSet<Integer> currHighlightIDs = new HashSet<Integer>();
	public void setHighlightID(int id) {
		currHighlightIDs.clear();
		currHighlightIDs.add(id); 
	}
	
	//Note: This is ONLY used for updating the progress bar; we can replace it with a callback later.
	private MainFrame parent;
	
	//The amount we are multiplying all Agent images by when drawing them.
	//Be careful setting this; obviously, scaling Agents will lead to a visually
	//inaccurate display (since cars are scaled down to the network).
	//However, this may be useful for visualization or debugging.
	private int scaleMult;
	public void setScaleMultiplier(int val) { scaleMult = val; }
	
	//The maximum (valid) frame tick.
	public int getMaxFrameTick() { return simRes.ticks.size()-1; }
	
	//The name of the current file being rendered. 
	private String fileName;
	public String getFileName() { return fileName; }
	
	private static final double Distance(double x1, double y1, double x2, double y2) {
		double dX = x1-x2;
		double dY = y1-y2;
		return Math.sqrt(dX*dX + dY*dY);
	}
	private static final double Distance(Point2D start, Point2D end) {
		return Distance(start.getX(), start.getY(), end.getX(), end.getY());
	}
	
	private static final Ellipse2D CircleFromPoints(Point2D min, Point2D max, double radius) {
		//Too simplistic, but it will work for now.
		Point2D center = new Point2D.Double(min.getX()+(max.getX()-min.getX())/2, min.getY()+(max.getY()-min.getY())/2);
		Ellipse2D el = new Ellipse2D.Double(center.getX()-radius, center.getY()-radius, radius*2, radius*2);
		return el;
	}
		
	public NetworkVisualizer(MainFrame parent) {
		this.parent = parent;
		this.scaleMult = 1;
	}
	
	///Retrieve a copy of the current viewport in real-world coordinates.
	public Rectangle2D getCurrentView() {
		return new Rectangle2D.Double(currView.getX(), currView.getY(), currView.getWidth(), currView.getHeight());
	}
	
	//NOTE: For now I'm disabling this; I don't want to keep a "buffer" lying around, and it won't matter too much to
	//      performance. We can enable caching the last-drawn timeTick+rectangle later if we want.
	/*public BufferedImage getImage() {

		return buffer;
	}*/
	
	public void setAnnotationLevel(boolean showAimsun, boolean showMitsim, int frameTick) {
		this.showAimsunLabels = showAimsun;
		this.showMitsimLabels = showMitsim;
		redrawFrame(frameTick);
	}

	public BufferedImage getImageAtTimeTick(int tick, Point resSize) {
		return getImageAtTimeTick(tick, resSize, BufferedImage.TYPE_INT_RGB);
	}
	public BufferedImage getImageAtTimeTick(int tick, Point resSize, int imageType) {
		BufferedImage res = new BufferedImage(resSize.x, resSize.y, imageType);
		redrawFrame(tick, (Graphics2D)res.getGraphics(), new Point(res.getWidth(), res.getHeight()));
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
	
	private static final void BuildNetworkIndex(LazySpatialIndex<DrawableItem> res, RoadNetwork net) {
		//Just add them all; we'll worry about drawing/hiding some of them later.
		for (Node n : net.getNodes().values()) {
			res.addItem(n, n.getBounds());
		}
		for (Link ln : net.getLinks().values()) {
			res.addItem(ln, ln.getBounds());
		}
		for (Segment sg : net.getSegments().values()) {
			res.addItem(sg, sg.getBounds());
		}
		for (Annotation an : net.getAimsunAnnotations()) {
			res.addItem(an, an.getBounds());
		}
		for (Annotation an : net.getMitsimAnnotations()) {
			res.addItem(an, an.getBounds());
		}
		for(CutLine ctl : net.getCutLine().values()){
			res.addItem(ctl, ctl.getBounds());
		}
		for (Hashtable<Integer,LaneMarking> lineMarkingTable : net.getLaneMarkings().values()) {
			for(LaneMarking lineMarking : lineMarkingTable.values()){
				res.addItem(lineMarking, lineMarking.getBounds());
			}
		}
		for(Crossing crossing : net.getCrossings().values()){
			res.addItem(crossing, crossing.getBounds());
		}

		//TODO:
		//  1) "Names" are not items yet.
	}
	
	
	public void setSource(RoadNetwork network, SimulationResults simRes, Point panelSize, String fileName) {
		//Save
		this.network = network;
		this.simRes = simRes;
		//this.naturalSize = new Dimension(width100Percent, height100Percent);
		this.fileName = fileName;
		
		//Rebuild the network spatial index.
		networkItemsIndex = new LazySpatialIndex<DrawableItem>();
		BuildNetworkIndex(networkItemsIndex, network);
		
		//Recalc
		Rectangle2D initialZoom = networkItemsIndex.getBounds();
		redrawFrame(0, panelSize, initialZoom);
	}
	
	
	public void translateBy(int frameTick, Point panelSize, int plusX, int plusY) {
		//Convert pixels to meters
		Point2D plusM = new Point2D.Double(
			plusX / ScaledPoint.getScaleFactor().getX(),
			plusY / ScaledPoint.getScaleFactor().getY());
		
		//Move the current view by that amount.
		Rectangle2D zoom = getCurrentView();
		zoom.setRect(
			zoom.getX() + plusM.getX(),
			zoom.getY() + plusM.getY(),
			zoom.getWidth(), zoom.getHeight());

		//Redraw at this translation
		redrawFrame(frameTick, panelSize, zoom);
	}
	
	
	//Negative numbers mean zoom out that many times.
	public void zoomIn(int number, int frameTick, Point panelSize) {
		//Each tick increases zoom by 10%
		Rectangle2D zoom = getCurrentView();
		Utility.resizeRectangle(zoom,
			zoom.getWidth()+zoom.getWidth()*number*0.10,
			zoom.getHeight()+zoom.getHeight()*number*0.10);
		
		//Redraw at this scale.
		redrawFrame(frameTick, panelSize, zoom);
	}
	
	public void squareZoom(int frameTick, Point panelSize) {
		//EASY
		if (currView.getWidth() == currView.getHeight()) { return; }
		Rectangle2D zoom = new Rectangle2D.Double(currView.getX(), currView.getY(), currView.getWidth(), currView.getHeight());
		Utility.resizeRectangle(zoom,
			Math.max(currView.getWidth(), currView.getHeight()),
			Math.max(currView.getWidth(), currView.getHeight()));
		
		//Now redraw
		redrawFrame(frameTick, panelSize, zoom);
	}
	
	public void toggleFakeAgent(boolean drawFakeAgent, int frameTick){

		this.showFakeAgent = drawFakeAgent;			
		redrawFrame(frameTick);
	}
	
	public void toggleDebugOn(boolean debugOn, int frameTick){
		this.debugOn = debugOn;
		redrawFrame(frameTick);
	}
	

	//This function is a bit bloated; we're basically saying "redraw, except maybe zoom first"
	public void redrawFrame(int frameTick) {
		redrawFrame(frameTick, null, null, null);
	}
	public void redrawFrame(int frameTick, Graphics2D g, Point size) {
		redrawFrame(frameTick, g, size, currView);
	}
	private void redrawFrame(int frameTick, Point size, Rectangle2D zoomRect) {
		redrawFrame(frameTick, null, size, zoomRect);
	}
	
	
	private void redrawFrame(int frameTick, Graphics2D g, Point size, Rectangle2D zoomRect) {
		//Check if we need to re-scale all points.
		boolean sameZoom = (currView!=null) && ((zoomRect==currView) || zoomRect.equals(currView)); //Ref and value check.
		if (!sameZoom) {
			//Determine the "scale" value needed to achieve this zoom level.
			Point2D zoomLevel = new Point2D.Double(
				size.x / zoomRect.getWidth(),
				size.y / zoomRect.getHeight());
			
			//Determine the upper-left coordinates of the view area in translated coordinates.
			Point2D upperLeftPoint = new Point2D.Double(
				zoomRect.getX() * zoomLevel.getX(),
				zoomRect.getY() * zoomLevel.getY());
			
			//Determine the height of the map in scaled coordinates.
			//double scaledHeight = size.y * zoomLevel.getY();
			
			//Update
			ScaledPoint.updateScaleAndTranslate(zoomLevel, upperLeftPoint/*, scaledHeight*/);
			
			//Save the new zoom
			currView = zoomRect;
		}

		//Old scaling code: probably not relevant anymore.
		/*if (false) {
			//Determine the width and height of our canvas.
			int width =  Math.max(1, (int)(naturalSize.width * percent));
			int height = Math.max(1, (int)(naturalSize.height * percent));
			buffer = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
			
			//Make sure our canvas is always slightly bigger than the original size...
			double width5Percent = 0.05 * (network.getLowerRight().x - network.getTopLeft().x);
			double height5Percent = 0.05 * (network.getLowerRight().y - network.getTopLeft().y);
			DPoint newTL = new DPoint(network.getTopLeft().x-width5Percent, network.getTopLeft().y-height5Percent);
			DPoint newLR = new DPoint(network.getLowerRight().x+width5Percent, network.getLowerRight().y+height5Percent);
		}*/

		//Now re-draw the rest.
		if ((g!=null) && (size!=null)) {
			redrawNow(frameTick, g, size);
		}
	}
	
	private void redrawNow(int frameTick, Graphics2D g, Point size) {
		//Save all points to be drawn into a list, grouped by z-order:
		DrawSorterAction act = new DrawSorterAction();
		networkItemsIndex.forAllItemsInRange(currView, act, null);

		//NOTE: This *might* not be the correct time to set this:
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Fill the background
		g.setBackground(MainFrame.Config.getBackground("network"));
		g.clearRect(0, 0, size.x, size.y);
		
		//Draw all items, sorted by key
		for (int key : act.getKeys()) {
			for (DrawableItem item : act.getValues(key)) {
				//TEMP: restrict
				if ((item.getZOrder()==DrawableItem.Z_ORDER_NODE) || 
					(item.getZOrder()==DrawableItem.Z_ORDER_SEGMENT)) {
					item.draw(g);
				}
			}
		}
		
		//Update our progress bar to show how many items are being culled from view.
		double percentDrawn = ((double)act.getItemCount()) / networkItemsIndex.getItemCount();
		parent.updatePercentDrawn(percentDrawn);
		
		
		//Draw nodes
		final boolean ZoomCritical = ("x".equals("Y")); //(currPercentZoom>ZOOM_IN_CRITICAL); //TODO: Re-enable.
		//drawAllNodes(g, (!ZoomCritical) || (showAimsunLabels || showMitsimLabels));
		
		//Draw segments
		//drawAllSegments(g, (!ZoomCritical) || (showAimsunLabels || showMitsimLabels));
		
		//Draw cut lines
		drawAllCutlines(g, this.showFakeAgent);
		
		//Draw all names
		drawAllNames(g);
		
		//Draw individual lanes
		drawAllLanes(g, ZoomCritical);
		
		//Draw all pedestrian crossings
		drawAllCrossings(g, ZoomCritical);

		//Draw all pedestrian crossing lights
		drawAllCrossingSignals(g, frameTick, ZoomCritical);

		//Draw all lane crossing lights
		drawAllLaneSignals(g, frameTick, ZoomCritical);

		//Draw all Agents now
		drawAllAgents(g, frameTick);
		
		//Draw all annotations
		drawAllAnnotations(g, showAimsunLabels, showMitsimLabels);
	}
	
	
	
	//Helper class: sort and save items by z-ordering
	private class DrawSorterAction implements LazySpatialIndex.Action<DrawableItem> {
		Hashtable<Integer, LinkedList<DrawableItem>> toDraw;
		int totalItems = 0;
		
		DrawSorterAction() {
			toDraw = new Hashtable<Integer, LinkedList<DrawableItem>>();
		}
		
		public void doAction(DrawableItem item) {
			int zOrder = item.getZOrder();
			if (!toDraw.containsKey(zOrder)) {
				toDraw.put(zOrder, new LinkedList<DrawableItem>());
			}
			toDraw.get(zOrder).add(item);
			totalItems++;
		}
		
		//Retrieve keys
		Integer[] getKeys() {
			Integer[] keys = toDraw.keySet().toArray(new Integer[]{});
			Arrays.sort(keys);
			return keys;
		}
		
		//Return values for a specific key
		List<DrawableItem> getValues(int key) {
			return toDraw.get(key);
		}
		
		int getItemCount() {
			return totalItems;
		}
	}

	
	
	private void drawAllNodes(Graphics2D g, boolean ShowUniNodes) {
		for (Node n : network.getNodes().values()) {
			if (ShowUniNodes || !n.getIsUni()) {
				n.draw(g);
			}
		}
	}
	
	private void drawAllAnnotations(Graphics2D g, boolean showAimsun, boolean showMitsim) {
		if (showAimsun) {
			for (Annotation an : network.getAimsunAnnotations()) {
				an.draw(g);
			}
		}
		if (showMitsim) {
			for (Annotation an : network.getMitsimAnnotations()) {
				an.draw(g);
			}
		}
	}
	
	private void drawAllSegments(Graphics2D g, boolean ShowSegments) {
		if(!ShowSegments) { return; }
		for (Segment sn : network.getSegments().values()) {
			sn.draw(g);
		}
	}
	
	private void drawAllCutlines(Graphics2D g, boolean ShowCutLines) {
		if (!ShowCutLines) { return; }
		for(CutLine ctl : network.getCutLine().values()){
			ctl.draw(g);	
		}
	}
	
	private void drawAllNames(Graphics2D g) {
		//Keep track and avoid drawing names more than once.
		Set<String> alreadyDrawn = new HashSet<String>();
		for (Link ln : network.getLinks().values()) {
			String key = ln.getAuthoritativeRoadName();
			if (!alreadyDrawn.contains(key)) {
				alreadyDrawn.add(key);
				//ln.drawName(g, currPercentZoom);  //TODO: This should use scaled points! 
			}
		}
	}
	
	private void drawAllLanes(Graphics2D g, boolean ShowLanes) {
		if (!ShowLanes) { return; }
		for (Hashtable<Integer,LaneMarking> lineMarkingTable : network.getLaneMarkings().values()) {
			for(LaneMarking lineMarking : lineMarkingTable.values()){
				lineMarking.draw(g);
			}
		}
	}
	
	
	private void drawAllCrossings(Graphics2D g, boolean ShowCrossings) {
		if (!ShowCrossings) { return; }
		for(Crossing crossing : network.getCrossings().values()){
			crossing.draw(g);
		}
	}
	
	private void drawAllCrossingSignals(Graphics2D g, int currFrame, boolean ShowCrossingSignals) {
		if (!ShowCrossingSignals) { return; }
		if (simRes.ticks.isEmpty() && currFrame==0) { return; }
		for(SignalLineTick at: simRes.ticks.get(currFrame).signalLineTicks.values()){
			//Get all lights and Crossings at this intersection (by id)
			Intersection tempIntersection = network.getIntersection().get(at.getIntersectionID());
			ArrayList<Integer> allPedestrainLights = at.getPedestrianLights();
			ArrayList<Integer> crossingIDs = tempIntersection.getSigalCrossingIDs();

			//Draw Crossing Lights
			for(int i=0; i<crossingIDs.size(); i++) {
				if(network.getTrafficSignalCrossing().containsKey(crossingIDs.get(i))) {
					network.getTrafficSignalCrossing().get(crossingIDs.get(i)).drawSignalCrossing(g, allPedestrainLights.get(i));
				} else{
					throw new RuntimeException("Unable to draw pedestrian crossing light; ID does not exist.");
				}
			}
		}
	}
	
	private void drawAllLaneSignals(Graphics2D g, int currFrame, boolean ShowLaneSignals) {
		if (!ShowLaneSignals) { return; }
		if (simRes.ticks.isEmpty() && currFrame==0) { return; }
		for(SignalLineTick at: simRes.ticks.get(currFrame).signalLineTicks.values()){
			//Get Intersection ID and color
			Intersection tempIntersection = network.getIntersection().get(at.getIntersectionID());
			ArrayList<ArrayList<Integer>> allVehicleLights =  at.getVehicleLights();

			//Draw Vehicle Lights
			for (int i=0; i<4; i++) {
				//0,1,2,3 correspond to a,b,c,d
				//TODO: The classes created are not intuitive. Some are index-based, others are
				//      name-based. Consider redoing them, adding support for both options (perhaps
				//      using iterators). ~Seth
				ArrayList<ArrayList<TrafficSignalLine>> signalLine = null;
				if (i==0) { signalLine = tempIntersection.getVaTrafficSignal(); }
				else if (i==1) { signalLine = tempIntersection.getVbTrafficSignal(); }
				else if (i==2) { signalLine = tempIntersection.getVcTrafficSignal(); }
				else if (i==3) { signalLine = tempIntersection.getVdTrafficSignal(); }
				ArrayList<Integer> lightColors = allVehicleLights.get(i);
				
				//Draw it
				drawTrafficLines(g,signalLine, lightColors);
			}
			
		}
	}
	
	
	private void drawAllAgents(Graphics2D g, int currFrame) {
		//It is possible (but unlikely) to have absolutely no agents at all.
		// The only time this makes sense is if currFrame is equal to zero.
		if (currFrame>=simRes.ticks.size()) {
			if (currFrame!=0) { throw new RuntimeException("Error: invalid non-zero frame."); }
			return;
		}
		
		//Use a scale multiplier to allow people to resize the agents as needed.
		//TODO: Do this somewhere else.
		//double adjustedZoom = currPercentZoom * scaleMult;
		
		//Draw all agent ticks
		Hashtable<Integer, AgentTick> agents = simRes.ticks.get(currFrame).agentTicks;
		Hashtable<Integer, AgentTick> trackings = simRes.ticks.get(currFrame).trackingTicks;
		for (Integer key : agents.keySet()) {
			//Retrieve the agent
			AgentTick at = agents.get(key);
			
			//Highlight?
			boolean highlight = this.debugOn || currHighlightIDs.contains(key.intValue());
			
			//Draw
			at.draw(g,scaleMult,this.showFakeAgent,highlight, new Point2D.Double(currView.getWidth(), currView.getHeight()));
			
			//Retrieve the tracking agent; also draw it
			AgentTick tr = trackings.get(key);
			drawTrackingAgent(g, at, tr, scaleMult);
		}		
	}
	
	private void drawTrackingAgent(Graphics2D g, AgentTick orig, AgentTick tracking, double scaleMult) {
		if (orig==null || tracking==null) { return; }
		tracking.draw(g, scaleMult, true, false, new Point2D.Double(currView.getWidth(), currView.getHeight()));
			
		//Draw a circle and a line
		g.setColor(hlColor);
		g.setStroke(str1pix);
		Point2D min = new Point2D.Double(Math.min(orig.getPos().getX(), tracking.getPos().getX()), Math.min(orig.getPos().getY(), tracking.getPos().getY()));
		Point2D max = new Point2D.Double(Math.max(orig.getPos().getX(), tracking.getPos().getX()), Math.max(orig.getPos().getY(), tracking.getPos().getY()));
		double dist = Distance(min, max);
		if (dist>1.0) {
			Line2D line = new Line2D.Double(min.getX(), min.getY(), max.getX(), max.getY());
			Ellipse2D el = CircleFromPoints(min, max, dist/2);
			g.draw(el);
			g.draw(line);
		}
	}
	
	
	
	private void drawTrafficLines(Graphics2D g,ArrayList<ArrayList<TrafficSignalLine>> signalLine, ArrayList<Integer> lightColors) {
		//0,1,2 = "Left", "Straight", "Right" turn lines.
		//TODO: Again, this is a bit confusing. Please clean up. ~Seth
		for (int i=0; i<signalLine.size()&&i<lightColors.size(); i++) {
			if (!signalLine.get(i).isEmpty()) {
				signalLine.get(i).get(0).drawPerLight(g, lightColors.get(i));
			}
		}	
	}
	
}

