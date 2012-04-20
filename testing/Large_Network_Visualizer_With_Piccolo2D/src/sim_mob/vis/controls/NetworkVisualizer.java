package sim_mob.vis.controls;

import java.awt.*;
import java.awt.geom.*;
import java.util.*;

import javax.swing.JSlider;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.*;
import sim_mob.vis.network.basic.Vect;
import sim_mob.vis.simultion.*;
import edu.umd.cs.piccolo.*;
import edu.umd.cs.piccolo.nodes.PText;
import edu.umd.cs.piccolo.util.PDimension;

public class NetworkVisualizer extends PCanvas {
	private static final long serialVersionUID = 1L;

	//private RoadNetwork network;
	private SimulationResults simRes;
	//private PLayer layer;
	private int currFrameNum;

	//private static Font roadNameFont = new Font("Arial", Font.PLAIN, 5);


	//Keep track the agent in layer
	//private Integer[] uniqueCarIDs;
	//private Hashtable<Integer,Integer> carIDtoIndex;
	//private ArrayList<Integer> visibleCars;
	private Hashtable<Integer, Car> cars;
	
	//Keep track the Index of crossing light in layer
	private Hashtable<Integer, Integer> crossLightIDtoIndex;
	
	//The size of the canvas at a zoom level of 1.0
	//private Dimension naturalSize;
	private Rectangle2D naturalBounds;
	public Rectangle2D getNaturalBounds() { return naturalBounds; }
	
	//The maximum (valid) frame tick.
	public int getMaxFrameTick() { return simRes.ticks.size()-1; }	
	
	public NetworkVisualizer( int width, int height){
		this.naturalBounds = new Rectangle2D.Double(0, 0, 10, 10); //Doesn't matter.
		setPreferredSize(new Dimension(width, height));
		this.setBackground(MainFrame.Config.getBackground("panel"));
 
		//layer = new PLayer();
    	//this.getCamera().addChild(layer);
    	
	}
	
	public void buildSceneGraph(RoadNetwork rn, SimulationResults simRes, HashSet<Integer> uniqueAgentIDs){
		
		//Set up resources 
		//this.network = rn;
		this.simRes = simRes;
		Integer[] uniqueCarIDs = uniqueAgentIDs.toArray(new Integer[]{});
		this.currFrameNum = 0;
		
		//Keep track agent's ID and its Index
		crossLightIDtoIndex = new Hashtable<Integer, Integer>();		
		//carIDtoIndex = new Hashtable<Integer,Integer>();
		
		//Visible Car's Index
		//visibleCars  = new ArrayList<Integer>();
		
		//Used to create car image
		cars = new Hashtable<Integer, Car>();
		
		//Add static items to the scene graph
		Point2D minPt = new Point2D.Double();
		Point2D maxPt = new Point2D.Double();
		addRoadNetworkItemsToGraph(rn, getLayer(), minPt, maxPt);
		this.naturalBounds = new Rectangle2D.Double(minPt.getX(), minPt.getY(), maxPt.getX()-minPt.getX(), maxPt.getY()-minPt.getY());
		
		//Initialize agents
		addAgentsToGraph(uniqueCarIDs, cars, getLayer());
		
		//Draw agents in current frame
		redrawAtCurrFrame(currFrameNum);
		
		
		for(Intersection it : rn.getIntersection().values()){
			//System.out.println(it.getVaTrafficSignal().size());
			for(int i = 0 ; i<it.getVaTrafficSignal().size();i++){
				if(it.getVaTrafficSignal().get(i).size()>0){
					//System.out.println(it.getVaTrafficSignal().get(i).get(0).getFromNode().getLocalPos().getX());
				}
			}
			//System.out.println(it.getVbTrafficSignal().size());
			for(int i = 0 ; i<it.getVbTrafficSignal().size();i++){
				
				if(it.getVbTrafficSignal().get(i).size()>0){
					//System.out.println(it.getVbTrafficSignal().get(i).get(0).getFromNode().getLocalPos().getX());
				}
			}
			//System.out.println(it.getVcTrafficSignal().size());

			for(int i = 0 ; i<it.getVcTrafficSignal().size();i++){
				if(it.getVcTrafficSignal().get(i).size()>0){
					//System.out.println(it.getVcTrafficSignal().get(i).get(0).getFromNode().getLocalPos().getX());
				}
			}
			//System.out.println(it.getVdTrafficSignal().size());

			for(int i = 0 ; i<it.getVdTrafficSignal().size();i++){
				if(it.getVdTrafficSignal().get(i).size()>0){
					//System.out.println(it.getVdTrafficSignal().get(i).get(0).getFromNode().getLocalPos().getX());
			
				}
			}
			System.out.println();
		}	
	
		
	}

	
	//Create agents according to their IDs and take note its corresponding Index in layer. 
	//For now, we only have car agent available.
	private void addAgentsToGraph(Integer[] uniqueCarIDs, Hashtable<Integer, Car> carArray, PLayer parent) {		
		for(int id : uniqueCarIDs) {
			//Make a new, invisible car for this Agent
			Car tempCar = new Car();
			tempCar.setVisible(false);
			carArray.put(id, tempCar);
			
			//Now add it to the scene graph
			parent.addChild(tempCar);
			//int index = layer.indexOfChild(tempCar);
			//carIDtoIndex.put((Integer)uniqueCarIDs[i], index);
		}
	}
	
	public void redrawAtCurrFrame(int frameTick){	
		currFrameNum = frameTick;
		updateAgent(currFrameNum);
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
		
		
		for(Hashtable<Integer,LaneMarking> vlmtable :  rn.getLaneMarkings().values()){
			for(LaneMarking vlm : vlmtable.values()){
				//vlm.repaint();
				parent.addChild(vlm);
			}
		}
		
		for(Crossing vc : rn.getCrossing().values()){

			parent.addChild(vc);
			crossLightIDtoIndex.put(vc.getId(),parent.indexOfChild(vc));

		}

		
		
	}

	
	
	public void updateAgent(int frameTick){
		drawAgent(frameTick);
	}

	private void drawAgent(int currFrame){
		//It is possible (but unlikely) to have absolutely no agents at all.
		// The only time this makes sense is if currFrame is equal to zero.
		if (currFrame>=simRes.ticks.size()) {
			if (currFrame!=0) { throw new RuntimeException("Error: invalid non-zero frame."); }
			return;
		}
		
		//TODO: If we can somehow "disable repaints" before this starts, then enable them after,
		//      that would really help.
		drawCars(currFrame);
		drawCrossingLight(currFrame);
		
		//repaint();
	}
	
	private void drawCrossingLight(int currFrame){
		//Get crossing light information in this frame tick
		Hashtable<Integer, CrossingLightTick> crossingLightTicks = simRes.ticks.get(currFrame).crossingLightTicks;
		
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
	
	private void drawCars(int currFrame){		
		//Start by setting all Agents to invisible
		for (Car c : cars.values()) {
			c.setVisible(false);
		}
		
		//Now, update all car positions and set them to visible if they exist in this time tick.
		ArrayList<Integer> agentIDs = simRes.ticks.get(currFrame).agentIDs;
		for (int agID : agentIDs) {
			//Retrieve the vehicle and its associated Agent Tick
			Car c = cars.get(agID);
			AgentTick at = simRes.ticks.get(currFrame).agentTicks.get(agID);
			if (c==null || at==null) { continue; }
			
			//Update it, set it visible.
			//TODO: We might be able to do this with translations; 
			//      for now, I'm just changing their positions manually.
			c.setX(at.getPos().getX());
			c.setY(at.getPos().getY());
			c.setRotation((Math.PI * at.getAngle())/180);
			c.setVisible(true);
		}
	}
	
	
	/////////////////////////////////////////////////////////////
	//TODO: Figure out where to put these functions later
	//      The Simulation should really be saved in an "Animator" class which
	//      drives the animation and tracks how close we are to the end.
	/////////////////////////////////////////////////////////////
	public boolean jumpAnim(int toTick, JSlider slider){
		//Set
		if (simRes==null || !setCurrFrameTick(toTick)) {
			return false;
		}
		//Update the slider, if it exists
		if (slider!=null) {
			slider.setEnabled(false);
			slider.setValue(getCurrFrameTick());
			slider.setEnabled(true);
		}

		redrawAtCurrFrame(getCurrFrameTick());
		return true;
	}
	
	public boolean setCurrFrameTick(int newVal) {
		if (newVal<0 || simRes==null || newVal>=getMaxFrameTick()) {
			return false;
		}
		currFrameNum = newVal;
		return true;
	}
	public int getCurrFrameTick() {
		return currFrameNum;
	}
	
	
	public boolean advanceAnim(int ticks, JSlider slider) {
		//Increment
		if (simRes==null || !incrementCurrFrameTick(1)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public boolean advanceAnimbyStep(int ticks, JSlider slider) {
		//Increment
		if (simRes==null || !incrementCurrFrameTick(ticks)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}

	public boolean incrementCurrFrameTick(int amt) {
		return setCurrFrameTick(getCurrFrameTick()+amt);
	}
	
}
