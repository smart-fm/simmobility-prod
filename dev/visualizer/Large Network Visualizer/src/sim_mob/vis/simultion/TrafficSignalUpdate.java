package sim_mob.vis.simultion;

import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.HashMap;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.Intersection;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.SignalHelper;
import sim_mob.vis.network.TrafficSignal;
import sim_mob.vis.network.TrafficSignal.Phase;
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
		
		Intersection tempIntersection = rdNet.getIntersection().get(id);
		SignalHelper signalHelper = tempIntersection.getSignalHelper();
		//since we dont have a mechanism like TrafficSignalLine for crossing, we build its signallinetick requirement right here
		HashMap<Integer,Integer> CRSs= new HashMap<Integer,Integer>();//this initialization is useless. the actuall initialization is done inside the following for loop.this one is just to avoid the errors
//		System.out.println("This crossing has " + tempIntersection.getAllSignalCrossings().values().size() + " Crossings");
//		System.out.println("");
		for(ArrayList<Integer> origCrossings:tempIntersection.getAllSignalCrossings().values())
			for(Integer origCrossing:origCrossings)
			{
				CRSs.put(origCrossing, 1);
//				if((frame == 230))
//					System.out.println("crossing " + origCrossing + " initialized to red");
			}
//		System.out.println("Total of " +CRSs.size()+ " Was set to red" );
		for(TrafficSignal.Phase updatingPhase:phases)
		{
			//segment part
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
				if(updatingPhase.getName().equals(currPhase))
				{
					for (TrafficSignal.Crossing updatingCrossing : updatingPhase.getCrossings()) {
						if(updatingCrossing != null){
							if(updatingCrossing.getId() != null){
							if(updatingCrossing.getId().length() > 1)	
							{
								int updatingCrossingId = signalHelper.HexStringToInt(updatingCrossing.getId());
								CRSs.put(updatingCrossingId,updatingCrossing.getCurrColor());
							}
							}
						}
					}
				}
		}
		//2.
		ArrayList<TrafficSignalLine> TSLs ;//  allocate memory and create a copy in the SignalLineTick constructor = new Hashtable<String, ArrayList<TrafficSignalLine>>();
		
		TSLs = tempIntersection.getPhaseTrafficSignalLines(currPhase);//we now give a reference of intersection's trafficsignallines with updated colors to the to-be-created SignalLineTick
		SignalLineTick tempSignalLineTick = new SignalLineTick(id,TSLs,CRSs,this.getTimeTick(),currPhase);
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
