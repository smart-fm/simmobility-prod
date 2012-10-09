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
import sim_mob.vis.util.Utility;



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
	 * to reconsider this and update every phase .Let's hope not)
	 * Anyway, This method creates a new SignalLineTick, 
	 * for this SignalLineTick, finds the proper intersection,
	 * find the corresponding trafficsignallines associated with this intersection(only for the current phase)
	 * update their color 
	 * adds this tempSignalLineTick to ticks' signalLineTicks sub-collection
	 * 
	 */
	public void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes) {		
		//This id will be used for 2 purposes:
		//1-updating current colours of the corresponding traffic signal in the corresponding intersection
		//2-adding a singallinetick to ticks container in simulationresults class
		
		//long id = SignalHelper.HexStringToInt(this.hex_id);
		long id = Utility.ParseLongOptionalHex(this.hex_id);
		Intersection tempIntersection = rdNet.getIntersections().get(id);
		
		//since we don't have a mechanism like TrafficSignalLine for crossing, we build its signallinetick requirement right here
		HashMap<Long,Integer> crossingsToColorID = new HashMap<Long,Integer>();
		for(ArrayList<Long> origCrossings : tempIntersection.getAllSignalCrossings().values()) {
			for(Long origCrossing : origCrossings) {
				crossingsToColorID.put(origCrossing, 1);
			}
		}
		
		
		SignalHelper signalHelper = tempIntersection.getSignalHelper();
		for(TrafficSignal.Phase updatingPhase:phases) {
			//segment part
			for(TrafficSignal.Segment updatingSegment:updatingPhase.getSegmens()) {
				long updatingSegmentFrom = Utility.ParseLongOptionalHex(updatingSegment.getSegmentFrom());
				long updatingSegmentTo = Utility.ParseLongOptionalHex(updatingSegment.getSegmentTo());
				SignalHelper.Phase originalPhaseHelper = signalHelper.getPhase(updatingPhase.getName());
				SignalHelper.Segment originalSegmentHelper = originalPhaseHelper.getSegmentPair(updatingSegmentFrom, updatingSegmentTo);
				if(originalSegmentHelper!=null && originalSegmentHelper.generatedTrafficSignalLine!=null) {
					//this was previously being done in the addTrafficLines() of NetworkVisualizer class.
					originalSegmentHelper.generatedTrafficSignalLine.setLightColor(updatingSegment.getCurrColor());
				}
			}
			if(updatingPhase.getName().equals(currPhase)){
				for (TrafficSignal.Crossing updatingCrossing : updatingPhase.getCrossings()) {
					if(updatingCrossing != null){
						if(updatingCrossing.getId() != null){
						if(updatingCrossing.getId().length() > 1) {
							long updatingCrossingId = Utility.ParseLongOptionalHex(updatingCrossing.getId());
							crossingsToColorID.put(updatingCrossingId,updatingCrossing.getCurrColor());
						}
						}
					}
				}
			}
		}
		
		//2.
		//  allocate memory and create a copy in the SignalLineTick constructor = new Hashtable<String, ArrayList<TrafficSignalLine>>();
		//we now give a reference of intersection's trafficsignallines with updated colors to the to-be-created SignalLineTick
		ArrayList<TrafficSignalLine> TSLs = tempIntersection.getPhaseTrafficSignalLines(currPhase);
		SignalLineTick tempSignalLineTick = new SignalLineTick(id,TSLs,crossingsToColorID,this.getTimeTick(),currPhase);
		
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
