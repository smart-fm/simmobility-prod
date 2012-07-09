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
		Phase updatingPhase = getCurrPhase();
//		System.out.println("Getting phase " + currPhase + " information");
		SignalHelper.Phase originalPhaseHelper = signalHelper.getPhase(currPhase);
		for(TrafficSignal.Segment updatingSegment:updatingPhase.getSegmens())
		{
			
			int updatingSegmentFrom = signalHelper.HexStringToInt(updatingSegment.getSegmentFrom());
			int updatingSegmentTo = signalHelper.HexStringToInt(updatingSegment.getSegmentTo());
			for(SignalHelper.Segment originalSegmentHelper : originalPhaseHelper.segments)
			{
				if(originalSegmentHelper == null)
					System.out.println("This is NULL");
				if((updatingSegmentFrom == originalSegmentHelper.segment_from)&&(updatingSegmentTo == originalSegmentHelper.segment_to))
				{
					//Now we are in business!
					//we have already saved, in the signalhelper, the record of TrafficSignalLine object, Now we can access it and set its color
					TrafficSignalLine tempTrafficSignalLine = originalSegmentHelper.generatedTrafficSignalLine;
					if(updatingSegment.getCurrColor() == 2)
						System.out.println("Updating Traffic Light To " + updatingSegment.getCurrColor() + "in frame " + this.getTimeTick());
					else
						System.out.println("Updating Traffic Light To " + updatingSegment.getCurrColor());
					
					tempTrafficSignalLine.setLightColor(updatingSegment.getCurrColor());//this were previously being done in the addTrafficLines() of NetworkVisualizer class !!!
				}
			}
		}
		//2.
		Hashtable<String, ArrayList<TrafficSignalLine>> TSLs = new Hashtable<String, ArrayList<TrafficSignalLine>>();
		TSLs = tempIntersection.getAllTrafficSignalLines();//we now give a copy of intersection's trafficsignallines with updated colors to the to-be-created SignalLineTick
//		//debug
//		System.out.println("warming up");
		for(ArrayList<TrafficSignalLine> tsls1 : TSLs.values())
			for(TrafficSignalLine tsl:tsls1)
			{
				if (tsl.getCurrColor() == Color.yellow)
					System.out.println("Tick " + this.getTimeTick() + " color has been set to  yellow");
//				else if (tsl.getCurrColor() == Color.green)
//					System.out.println("color has been set to green");
//				else if (tsl.getCurrColor() == Color.red)
//					System.out.println("color has been set to red");
			}
//		//debug ends
//		SignalLineTick tempSignalLineTick = new SignalLineTick(id,phases);
		SignalLineTick tempSignalLineTick = new SignalLineTick(id,TSLs);
		//Now add it to the place holder as it used to add in SimResLineParser.end(). 
		simRes.reserveTimeTick(this.getTimeTick());
		simRes.ticks.get(this.getTimeTick()).signalLineTicks.put(tempSignalLineTick.getID(), tempSignalLineTick);
	

		
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
