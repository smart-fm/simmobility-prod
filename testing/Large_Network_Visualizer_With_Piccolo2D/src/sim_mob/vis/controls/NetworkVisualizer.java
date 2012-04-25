package sim_mob.vis.controls;

import java.awt.*;
import java.awt.geom.*;
import java.util.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.*;
import sim_mob.vis.simultion.*;
import edu.umd.cs.piccolo.*;
import edu.umd.cs.piccolo.util.PPaintContext;



public class NetworkVisualizer extends PCanvas {
	private static final long serialVersionUID = 1L;

	private Hashtable<Integer, Car> cars;
	
	//Keep track the Index of crossing light in layer
	private Hashtable<Integer, Integer> crossLightIDtoIndex;
	
	//The size of the canvas at a zoom level of 1.0
	//private Dimension naturalSize;
	private Rectangle2D naturalBounds;
	public Rectangle2D getNaturalBounds() { return naturalBounds; }
	
	private RectNode zoomBox;
	
	private static final Stroke onePtStroke = new BasicStroke(1);
	
	public NetworkVisualizer( int width, int height){
		this.naturalBounds = new Rectangle2D.Double(0, 0, 10, 10); //Doesn't matter.
		setPreferredSize(new Dimension(width, height));
		this.setBackground(MainFrame.Config.getBackground("panel", Color.black));
 
		this.zoomBox = new RectNode(100, 100, 200, 300);
		this.zoomBox.setVisible(false);
		getCamera().addChild(zoomBox);
	}
	
	
	public void setZoomBox(Rectangle2D rect) {
		this.zoomBox.setVisible(rect!=null);
		if (rect!=null) {
			this.zoomBox.setBounds(rect);
		}
	}
	
	public void zoomToBox() {
		Rectangle2D bounds = getCamera().getBounds();
		Rectangle2D viewBounds = getCamera().getViewBounds();
		Rectangle2D zoom = zoomBox.getBounds();
		
		Rectangle2D newZoom = new Rectangle2D.Double(
			zoom.getX()*viewBounds.getWidth()/bounds.getWidth() + viewBounds.getX(),
			zoom.getY()*viewBounds.getHeight()/bounds.getHeight() + viewBounds.getY(),
			zoom.getWidth()*viewBounds.getWidth()/bounds.getWidth(),
			zoom.getHeight()*viewBounds.getHeight()/bounds.getHeight()
		);
		
		getCamera().animateViewToCenterBounds(newZoom, true, 1000);
	}
	
	
	public void buildSceneGraph(RoadNetwork rn, SimulationResults simRes, HashSet<Integer> uniqueAgentIDs){
		//Set up resources 
		Integer[] uniqueCarIDs = uniqueAgentIDs.toArray(new Integer[]{});
		
		//Keep track agent's ID and its Index
		crossLightIDtoIndex = new Hashtable<Integer, Integer>();		
		
		//Used to create car image
		cars = new Hashtable<Integer, Car>();
		
		//Add static items to the scene graph
		Point2D minPt = new Point2D.Double();
		Point2D maxPt = new Point2D.Double();
		addRoadNetworkItemsToGraph(rn, getLayer(), minPt, maxPt);
		this.naturalBounds = new Rectangle2D.Double(minPt.getX(), minPt.getY(), maxPt.getX()-minPt.getX(), maxPt.getY()-minPt.getY());
		
		//Initialize agents
		addAgentsToGraph(uniqueCarIDs, cars, getLayer());
		
		//Add intersections (?)
		/*for(Intersection it : rn.getIntersection().values()){
			for(int i = 0 ; i<it.getVaTrafficSignal().size();i++){
				if(it.getVaTrafficSignal().get(i).size()>0){
				}
			}
			for(int i = 0 ; i<it.getVbTrafficSignal().size();i++){
				if(it.getVbTrafficSignal().get(i).size()>0){
				}
			}
			for(int i = 0 ; i<it.getVcTrafficSignal().size();i++){
				if(it.getVcTrafficSignal().get(i).size()>0){
				}
			}
			for(int i = 0 ; i<it.getVdTrafficSignal().size();i++){
				if(it.getVdTrafficSignal().get(i).size()>0){
				}
			}
		}*/	
	
		
	}

	
	//Create agents according to their IDs and take note its corresponding Index in layer. 
	//For now, we only have car agent available.
	private void addAgentsToGraph(Integer[] uniqueCarIDs, Hashtable<Integer, Car> carArray, PLayer parent) {		
		for(int id : uniqueCarIDs) {
			//Make a new, invisible car for this Agent
			Car tempCar = new Car(id);
			tempCar.setVisible(false);
			carArray.put(id, tempCar);
			
			//Now add it to the scene graph
			parent.addChild(tempCar);
		}
	}
	
	/*package-private*/ void redrawAtCurrFrame(TimeTick tick){
		if (tick==null) { return; }
		
		//TODO: If we can somehow "disable repaints" before this starts, then enable them after,
		//      that would really help.
		drawCars(tick);
		drawCrossingLight(tick);

		//NOTE: This is a hack to force clean painting. In actuality, specifying bounds
		//      correctly should allow the interface to repaint smoothly.
		repaint();
	}

	
	private static final void UpdateBounds(double x, double y, Point2D minPt, Point2D maxPt) {
		minPt.setLocation(Math.min(x,  minPt.getX()),  Math.min(y,  minPt.getY()));
		maxPt.setLocation(Math.max(x,  maxPt.getX()),  Math.max(y,  maxPt.getY()));
	}
	
	private void addRoadNetworkItemsToGraph(RoadNetwork rn, PLayer parent, Point2D minPt, Point2D maxPt){
		if (rn==null) { throw new RuntimeException("Can't add null road network to the scene graph."); }
		if (rn.getNodes().size()<2) { throw new RuntimeException("The road network needs at least two nodes."); }
		
		boolean firstNode = true;
		
		//Add all Nodes
		for(Node vn : rn.getNodes().values()){
			//vn.repaint();
			if(!vn.getIsUni()) {
				if (firstNode) {
					minPt.setLocation(vn.getX(), vn.getY());
					maxPt.setLocation(vn.getX(), vn.getY());
					firstNode = false;
				}
				
				parent.addChild(vn);
				UpdateBounds(vn.getX(), vn.getY(), minPt, maxPt);
			}

		}
		
		//Add all Links (labels)
		for(Link vl : rn.getLinks().values()) {			
			PNode tn = new RoadName(vl.getName(), vl.getStart(), vl.getEnd());
			parent.addChild(tn);
		}
		
		//Add all Segments
		for(Segment vs : rn.getSegments().values()){
			parent.addChild(vs);
		}
		
		
		/*for(Hashtable<Integer,LaneMarking> vlmtable :  rn.getLaneMarkings().values()){
			for(LaneMarking vlm : vlmtable.values()){
				//vlm.repaint();
				parent.addChild(vlm);
			}
		}*/
		
		for(Crossing vc : rn.getCrossing().values()){

			parent.addChild(vc);
			crossLightIDtoIndex.put(vc.getId(),parent.indexOfChild(vc));

		}

		
		
	}


	
	private void drawCrossingLight(TimeTick currFrameTick){
		//Get crossing light information in this frame tick
		Hashtable<Integer, CrossingLightTick> crossingLightTicks = currFrameTick.crossingLightTicks;
		
		//Draw out Crossing Light		
		for(CrossingLightTick clt : crossingLightTicks.values()){
			
			for(int i = 0 ; i <clt.getCrossingIDs().size();i++){
				
				if(crossLightIDtoIndex.containsKey(clt.getCrossingIDs().get(i))){
					
					int index = crossLightIDtoIndex.get(clt.getCrossingIDs().get(i));
					Crossing tempCrossing = (Crossing) getLayer().getChild(index);
					
					//Change Crossing's Light
					if(clt.getCrossingLights().get(i) == 1){
						tempCrossing.changeColor(1);
					}else if(clt.getCrossingLights().get(i) == 2){
						tempCrossing.changeColor(2);
					}else if(clt.getCrossingLights().get(i) == 3){
						tempCrossing.changeColor(3);
					}else{
						//Set back to original color
						tempCrossing.changeColor(0);					
					}
					
				}else{
					System.out.println("Error, no such crossing id, drawAgent, NetworkVisualizer");
				}
			}
		}
	}
	
	private void drawCars(TimeTick currFrameTick){		
		//Start by setting all Agents to invisible
		for (Car c : cars.values()) {
			c.setVisible(false);
		}
		
		//Now, update all car positions and set them to visible if they exist in this time tick.
		ArrayList<Integer> agentIDs = currFrameTick.agentIDs;
		for (int agID : agentIDs) {
			//Retrieve the vehicle and its associated Agent Tick
			Car c = cars.get(agID);
			AgentTick at = currFrameTick.agentTicks.get(agID);
			if (c==null || at==null) { continue; }
			c.setVisible(true);
			
			//Update it, set it visible.
			//TODO: We might be able to do this with translations; 
			//      for now, I'm just changing their positions manually.
			//double angle = (Math.PI * at.getAngle())/180;
			c.setX(at.getPos().getX());
			c.setY(at.getPos().getY());
			//c.rotateAboutPoint(angle, at.getPos().getX(), at.getPos().getY());
		}
	}
	
	
	//For zoom display
	class RectNode extends PNode {
		public RectNode(double x, double y, double w, double h) {
			this.setBounds(x, y, w, h);
		}
		protected void paint(PPaintContext pc) {
			Graphics2D g = pc.getGraphics();
			g.setStroke(onePtStroke);
			g.setColor(Color.red);
			g.draw(getBounds());
		}
	}
	
	

}
