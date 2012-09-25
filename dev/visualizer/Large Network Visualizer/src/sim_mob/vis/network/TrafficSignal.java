package sim_mob.vis.network;
import sim_mob.vis.network.Intersection;
import sim_mob.vis.network.SignalHelper;
import sim_mob.vis.util.Utility;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import sim_mob.vis.Main;

import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.simultion.GsonResObj;
import sim_mob.vis.simultion.SimulationResults;

public class TrafficSignal implements DrawableItem, GsonResObj {
	//link based
//	private class Link {
//		public Link(){};
//		public Link(String link_from_, String link_to_){
//			link_from = link_from_;
//			link_to = link_to_;
//		}
//		private String link_from;
//		private String link_to;
	
	//segment based
	public class Segment {
	public Segment(){};
	public Segment(String segment_from_, String segment_to_){
		segment_from = segment_from_;
		segment_to = segment_to_;
	}
	private String segment_from;
	private String segment_to;
	//This will be "null" for TrafficSignal; it will be set properly in "TrafficSignalUpdate"
	private Integer current_color;
	
	public String getSegmentFrom(){ return segment_from; }
	public String getSegmentTo(){ return segment_to; }
	public Integer getCurrColor() { return current_color;}
	}
	
	public class Crossing {
		public Crossing(){};
		public Crossing(String id_){id = id_;}
		private String id;
		
		//This will be "null" for TrafficSignal; it will be set properly in "TrafficSignalUpdate"
		private Integer current_color;
		public String getId() { return id;}
		public Integer getCurrColor() { return current_color;}
	}
	
	public class Phase {
		private String name;
		//segmant based
		private Segment[] segments;
		//link based
//		private Link[] links;
		private Crossing[] crossings;
		
		//getters
		public String getName() {
			return name;
		}
		public Segment[] getSegmens(){
			return segments;
		}
		public Crossing[] getCrossings(){
			return crossings;
		}
		
	}
	private String hex_id;
	private Integer frame;
	private String simmob_id;
	private String node;
	private Phase[] phases;
	
	
	public void addSelfToSimulation(RoadNetwork rdNet, SimulationResults simRes) {
		Main.NEW_SIGNAL = true;
		System.out.println("System NEW_SIGNAL reset to true");
//		System.out.println("Inside TS.addSelfToSimulation");
		SignalHelper signalHelper = new SignalHelper();
		signalHelper.phases = new ArrayList<SignalHelper.Phase>();
		//TODO: Here is where you'd add this traffic signal to the road network.
//		signalHelper.node = Utility.ParseIntOptionalHex(node);
//		signalHelper.hex_id = Utility.ParseIntOptionalHex(hex_id);//intersection id
		signalHelper.node = SignalHelper.HexStringToInt(node);
		signalHelper.hex_id = SignalHelper.HexStringToInt(hex_id);//intersection id
		for(Phase ph: phases)
		{
			SignalHelper.Phase phase = signalHelper.new Phase(ph.name);
			phase.segments = new ArrayList<SignalHelper.Segment>();
			phase.crossings = new ArrayList<SignalHelper.Crossing>();
//			link based
//			for(Link ln : ph.links)
//			{
//				SignalHelper.Link link = signalHelper.new Link(Utility.ParseIntOptionalHex(ln.link_from), Utility.ParseIntOptionalHex(ln.link_to));
//				phase.links.add(link);
//			}
			
//			Segment based
//			System.out.println("ph.segments length= "+ph.segments.length);
			for(Segment rs : ph.segments)
			{
				//TODO: for sync purpose, change all ParseIntOptionalHex() to the local signalhelper converter function
				SignalHelper.Segment segment = signalHelper.new Segment(Utility.ParseIntOptionalHex(rs.segment_from), Utility.ParseIntOptionalHex(rs.segment_to));
				phase.segments.add(segment);
			}
			
			for(Crossing cr : ph.crossings)
			{
				SignalHelper.Crossing crossing = signalHelper.new Crossing(Utility.ParseIntOptionalHex(cr.id));
				phase.crossings.add(crossing);
			}
			signalHelper.phases.add(phase);
		}
		//Something like this?....
//		System.out.println("Adding intersection "+ signalHelper.hex_id + " to road network");
		rdNet.getIntersection().put(signalHelper.hex_id, new Intersection(signalHelper));
	}
	
	public void draw(Graphics2D g, DrawParams params) {
		//TODO: Here is where you'd draw the signal. 
		
	}
	
	public Rectangle2D getBounds() {
		//TODO: You need to estimate the size of the boundary for our spatial index.
		//      Check Node, Crossing, etc., to see how they do this.
		return new Rectangle2D.Double(0, 0, 1, 1);
	}

	
	public int getTimeTick() {
		return frame; //All network items have time tick 0.
	}
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_TRAFFIC_SIGNAL_UPDATE;
	}
}
