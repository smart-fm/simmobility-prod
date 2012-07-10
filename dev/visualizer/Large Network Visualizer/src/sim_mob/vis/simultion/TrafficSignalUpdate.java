package sim_mob.vis.simultion;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Hashtable;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.Intersection;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.SignalHelper;
import sim_mob.vis.network.TrafficSignal;
import sim_mob.vis.network.TrafficSignal.Phase;
import sim_mob.vis.network.TrafficSignal.Segment;
import sim_mob.vis.network.TrafficSignalLine;
import sim_mob.vis.simultion.GsonResObj;



/**
 * 
 * IMPORTANT NOTE: You might not have to subclass "DrawableItem". Instead, you might
 *                 have this class just "update" the traffic signal, which is what I 
 *                 think you were intending. Anyway, this is up to whoever develops the
 *                 visualization for this component. ~Seth
 *
 */
public class TrafficSignalUpdate implements DrawableItem, GsonResObj {
	private String hex_id;
	private Integer frame;
	private String currPhase;
	private Phase[] phases;
	
	
	/*Algorithm:
	 * (non-Javadoc)
	 * @see sim_mob.vis.simultion.GsonResObj#addSelfToSimulation(sim_mob.vis.network.RoadNetwork, sim_mob.vis.simultion.SimulationResults)
	 * here we have phases with current color, we just update the colors of the current phase(we may need
	 * to reconsider tis and update evrey phase .Let's ope not)
	 * Anyway, This method creates a new SignalLineTick, 
	 * for this SignalLineTick, finds the proper intersection,
	 * find the corresponding trafficsignallines associated with this intersection(only for the current phase)
	 * update their color 
	 * adds this tempSignalLineTick to ticks' signalLineTicks sub-collection
	 * 
	 */
	public void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes) {
		//TODO: Here is where you'd add this traffic signal to the road network.
//		System.out.println("Inside TSU.trafficsignalupdate");
		
		//This id will be used for 2 purposes:
		//1-updating current colours of the corresponding traffic signal in the corresponding intersection
		//2-adding a singallinetick to ticks container in simulationresults class
		
		int id = SignalHelper.HexStringToInt(this.hex_id);
		
		//1.
//		System.out.println("Looking for Intersection " + id + "  which must be equal to " + rdNet.getIntersections().get(id).getIntersectID());
		Intersection tempIntersection = rdNet.getIntersections().get(id);
		SignalHelper signalHelper = tempIntersection.getSignalHelper();
		
		for(TrafficSignal.Phase updatingPhase:phases)
		for(TrafficSignal.Segment updatingSegment:updatingPhase.getSegmens())
		{
			
			int updatingSegmentFrom = signalHelper.HexStringToInt(updatingSegment.getSegmentFrom());
			int updatingSegmentTo = signalHelper.HexStringToInt(updatingSegment.getSegmentTo());
			SignalHelper.Phase originalPhaseHelper = signalHelper.getPhase(updatingPhase.getName());
			SignalHelper.Segment originalSegmentHelper = originalPhaseHelper.getSegmentPair(updatingSegmentFrom, updatingSegmentTo);
			if(originalSegmentHelper !=null)
			{
				originalSegmentHelper.generatedTrafficSignalLine.setLightColor(updatingSegment.getCurrColor());//this were previously being done in the addTrafficLines() of NetworkVisualizer class !!!
			}
		}
		//2.
		Hashtable<String, ArrayList<TrafficSignalLine>> TSLs ;//  allocate memory and create a copy in the SignalLineTick constructor = new Hashtable<String, ArrayList<TrafficSignalLine>>();
		TSLs = tempIntersection.getAllTrafficSignalLines();//we now give a copy of intersection's trafficsignallines with updated colors to the to-be-created SignalLineTick
//		//debug
		
		for(ArrayList<TrafficSignalLine> tsls1 : TSLs.values())
			for(TrafficSignalLine tsl:tsls1)
			{
				if((tsl.getPhaseName().equals("C")) &&((this.getTimeTick() == 230)||(this.getTimeTick() == 240)||(this.getTimeTick() == 250))){
					System.out.println("AddSelfToSimulation0 before adding updated color to signalLineTicks");
				if (tsl.getCurrColor() == Color.yellow)
					System.out.println("AddSelfToSimulation0 Tick " + this.getTimeTick() +   " yellow");
				else if (tsl.getCurrColor() == Color.green)
					System.out.println("AddSelfToSimulation0 Tick " + this.getTimeTick() +  "  green");
				else if (tsl.getCurrColor() == Color.red)
					System.out.println("AddSelfToSimulation0 Tick " + this.getTimeTick() + "  red\n");
				}
			}
//		//debug ends
//		SignalLineTick tempSignalLineTick = new SignalLineTick(id,phases);
		SignalLineTick tempSignalLineTick = new SignalLineTick(id,TSLs,this.getTimeTick(),currPhase);
		//Now add it to the place holder as it used to add in SimResLineParser.end(). 
		simRes.reserveTimeTick(this.getTimeTick());
		simRes.ticks.get(this.getTimeTick()).signalLineTicks.put(tempSignalLineTick.getID(), tempSignalLineTick);
		if((this.getTimeTick() == 230)||(this.getTimeTick() == 240)||(this.getTimeTick() == 250))
			System.out.println("AddSelfToSimulation0 after adding a color updated set of TSL to signalLineTicks");
		
		
		for(ArrayList<TrafficSignalLine> tsls1 : simRes.ticks.get(this.getTimeTick()).signalLineTicks.get(tempSignalLineTick.getID()).getAllTrafficSignalLines().values())
			for(TrafficSignalLine tsl:tsls1)
			{
				if((tsl.getPhaseName().equals("C")) &&((this.getTimeTick() == 230)||(this.getTimeTick() == 240)||(this.getTimeTick() == 250))){
					System.out.println("AddSelfToSimulation1 Testing after adding to see if it is really there!");
				if (tsl.getCurrColor() == Color.yellow){
					System.out.println("AddSelfToSimulation1 Tick " + this.getTimeTick() +  " yellow");
//					System.out.println("(" + (int)tsl.getFromNode().getPos().getX()+ " : " + (int)tsl.getFromNode().getPos().getY()+ "),  (" +(int)tsl.getToNode().getPos().getX()+ " : " +(int)tsl.getToNode().getPos().getY()+")");
				}
				else if (tsl.getCurrColor() == Color.green)
					System.out.println("AddSelfToSimulation1 Tick " + this.getTimeTick() +  "  green");
				else if (tsl.getCurrColor() == Color.red)
					System.out.println("AddSelfToSimulation1 Tick " + this.getTimeTick() +  "  red\n");
				
				}
			}
	

		
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		//TODO: Here is where you'd draw the signal/update. 
		
	}
	
	public Rectangle2D getBounds() {
		//TODO: You need to estimate the size of the boundary for our spatial index.
		//      Check Node, Crossing, etc., to see how they do this.
		return new Rectangle2D.Double(0, 0, 1, 1);
	}

	
	public int getTimeTick() {
		return this.frame;
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TRAFFIC_SIGNAL_UPDATE;
	}
	
	public Phase getCurrPhase()
	{
		for(Phase phase : phases)
		{
			if(phase.getName() == currPhase)
				return phase;
		}
		return null;
	}
	

}
