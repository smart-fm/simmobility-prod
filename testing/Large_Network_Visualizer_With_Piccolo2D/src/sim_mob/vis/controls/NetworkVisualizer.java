package sim_mob.vis.controls;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.geom.AffineTransform;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;

import sim_mob.vis.network.Intersection;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.Crossing;
import sim_mob.vis.network.LaneMarking;
import sim_mob.vis.network.Link;
import sim_mob.vis.network.Node;
import sim_mob.vis.network.RoadName;
import sim_mob.vis.network.Segment;
import sim_mob.vis.network.basic.DPoint;
import sim_mob.vis.network.basic.Vect;
import sim_mob.vis.simultion.AgentTick;
import sim_mob.vis.simultion.Car;
import sim_mob.vis.simultion.CrossingLightTick;
import sim_mob.vis.simultion.DriverTick;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.simultion.TimeTick;
import edu.umd.cs.piccolo.PCanvas;
import edu.umd.cs.piccolo.PLayer;
import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.activities.PActivity;
import edu.umd.cs.piccolo.activities.PInterpolatingActivity;
import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.nodes.PText;

public class NetworkVisualizer extends PCanvas{

	private RoadNetwork network;
	private SimulationResults simRes;
	private PLayer layer;
	private int currFrameNum;

	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 5);


	//Keep track the agent in layer
	private Object[] uniqueCarIDs;
	private Hashtable<Integer,Integer> carIDtoIndex;
	private ArrayList<Integer> visibleCars;
	private Hashtable<Integer, Car> cars;
	
	//Keep track the Index of crossing light in layer
	private Hashtable<Integer, Integer> crossLightIDtoIndex;
	
	//The size of the canvas at a zoom level of 1.0
	private Dimension naturalSize;		
	
	public PLayer getMyLayer() { return this.layer; }
	
	//The maximum (valid) frame tick.
	public int getMaxFrameTick() { return simRes.ticks.size()-1; }	
	
	public NetworkVisualizer( int width, int height){
    	this.naturalSize = new Dimension(width, height);
		setPreferredSize(this.naturalSize);	
 
		layer = new PLayer();
    	this.getCamera().addChild(layer);
    	
	}
	
	public void setVis(RoadNetwork rn, SimulationResults simRes, HashSet<Integer> uniqueAgentIDs){
		
		//Set up resources 
		this.network = rn;
		this.simRes = simRes;
		this.uniqueCarIDs = uniqueAgentIDs.toArray();
		this.currFrameNum = 0;
		
		//Keep track agent's ID and its Index
		crossLightIDtoIndex = new Hashtable<Integer, Integer>();		
		carIDtoIndex = new Hashtable<Integer,Integer>();
		
		//Visible Car's Index
		visibleCars  = new ArrayList<Integer>();
		
		//Used to create car image
		cars = new Hashtable<Integer, Car>();
		
		//Draw static map
		drawRoadnetworkMap();	
		
		//Initialize agents
		prepareAgent();
		
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
	public void prepareAgent(){		
		
		for(int i = 0; i<uniqueCarIDs.length;i++){

			Car tempCar = new Car();
			
			cars.put((Integer)uniqueCarIDs[i], tempCar);
			
			tempCar.setVisible(false);
			
			layer.addChild(tempCar);
			int index = layer.indexOfChild(tempCar);
			carIDtoIndex.put((Integer)uniqueCarIDs[i], index);
		}
	}
	
	public void redrawAtCurrFrame(int frameTick){	
		currFrameNum = frameTick;
		updateAgent(currFrameNum);
	}
	
	public void drawRoadnetworkMap(){
		if (network==null) {
			return;
		}
		
		Hashtable<Integer, Node> virtualNodeList = network.getNodes();
		Hashtable<Integer, Link> virtualLinkList = network.getLinks();
		Hashtable<Integer, Hashtable<Integer, LaneMarking>> virtualLaneMarkingList = network.getLaneMarkings();
		Hashtable<Integer, Segment> virtualSegmentList = network.getSegments();
		Hashtable<Integer, Crossing> virtualCrossingList = network.getCrossing();
		
		
		
		for(Node vn : virtualNodeList.values()){
			//vn.repaint();		
			if(!vn.getIsUni())
				layer.addChild(vn);

		}
		
		for(Link vl : virtualLinkList.values()){
			//vl.repaint();
			//layer.addChild(vl);
			layer.addChild(drawText(vl.getName(),vl.getStart(),vl.getEnd()));
			
		}
		
		for(Hashtable<Integer,LaneMarking> vlmtable :  virtualLaneMarkingList.values()){
			for(LaneMarking vlm : vlmtable.values()){
				//vlm.repaint();
				layer.addChild(vlm);
			}
		}
		
		for(Segment vs : virtualSegmentList.values()){
			//vs.repaint();
			//layer.addChild(vs);
		}
		
		for(Crossing vc : virtualCrossingList.values()){

			layer.addChild(vc);
			crossLightIDtoIndex.put(vc.getId(),layer.indexOfChild(vc));

		}

		
		
	}
	
	public PText drawText(String name, Node start, Node end){
		
		PText tempText = new PText(name);
		tempText.setFont(roadNameFont);
		float targetX = (float)(start.getLocalPos().getX()+(end.getLocalPos().getX()-start.getLocalPos().getX())/2);
		float targetY = (float)(start.getLocalPos().getY()+(end.getLocalPos().getY()-start.getLocalPos().getY())/2);
		
		float halfStrWidth = 10 / 2.0F;
		//Create a new translation matrix which is located at the center of the string.
		AffineTransform trans = AffineTransform.getTranslateInstance(targetX, targetY);
		
		//Figure out the rotational matrix of this line, from start to end.
		Vect line = new Vect(start.getLocalPos().getX(), start.getLocalPos().getY(), end.getLocalPos().getX(), end.getLocalPos().getY());
		trans.rotate(line.getMagX(), line.getMagY());
		trans.translate(-16, -18);

		tempText.setPaint(Color.white);
		tempText.setTransform(trans);
		return tempText;
	}
	
	
	public void updateAgent(int frameTick){
		drawAgent(frameTick);
	}

	public void drawAgent(int currFrame){
				
		drawCars(currFrame);
		drawCrossingLight(currFrame);
				
	}
	
	public void drawCrossingLight(int currFrame){
		
		//It is possible (but unlikely) to have absolutely no agents at all.
		// The only time this makes sense is if currFrame is equal to zero.
		if (currFrame>=simRes.ticks.size()) {
			if (currFrame!=0) { throw new RuntimeException("Error: invalid non-zero frame."); }
			return;
		}
		
		//Get crossing light information in this frame tick
		Hashtable<Integer, CrossingLightTick> crossingLightTicks = simRes.ticks.get(currFrame).crossingLightTicks;
		
		//Draw out Crossing Light		
		for(CrossingLightTick clt : crossingLightTicks.values()){
			
			for(int i = 0 ; i <clt.getCrossingIDs().size();i++){
				
				if(crossLightIDtoIndex.containsKey(clt.getCrossingIDs().get(i))){
					
					int index = crossLightIDtoIndex.get(clt.getCrossingIDs().get(i));
					Crossing tempCrossing = (Crossing) layer.getChild(index);
					
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
	
	public void drawCars(int currFrame){
		//It is possible (but unlikely) to have absolutely no agents at all.
		// The only time this makes sense is if currFrame is equal to zero.
		if (currFrame>=simRes.ticks.size()) {
			if (currFrame!=0) { throw new RuntimeException("Error: invalid non-zero frame."); }
			return;
		}
		
		Hashtable<Integer, AgentTick> agents = simRes.ticks.get(currFrame).agentTicks;
		ArrayList<Integer> agentIDs = simRes.ticks.get(currFrame).agentIDs;
	
		
		//If agent is no longer visible, set it invisible
		for(int i = 0; i<visibleCars.size();i++){
			int id = visibleCars.get(i);
			
			//The agent is no longer visible
			if(!agentIDs.contains(id)){
				int index = carIDtoIndex.get(id);
				//Set the agent invisible
				layer.getChild(index).setVisible(false);

				//Update the visible agent list
				visibleCars.remove(i);
			}
			
		}
		
		//Set agent Visible
		for(int i = 0; i<agentIDs.size();i++){
			int id = agentIDs.get(i);
			int index = carIDtoIndex.get(id);
			
			
			Car tempDriver = (Car) layer.getChild(index);
			
			AgentTick tempAgent = agents.get(id);
			
			if(!tempDriver.getVisible())
			{
				tempDriver.setVisible(true);
				visibleCars.add(id);
			}
			
			//Translate visible cars to its position and angle in this frame
			AffineTransform newPos = new AffineTransform();
			newPos.translate(tempAgent.getLocalPos().getX(),tempAgent.getLocalPos().getY());
			newPos.rotate((Math.PI * tempAgent.getAngle())/180);
			tempDriver.setTransform(newPos);
			

		}
	}
}
