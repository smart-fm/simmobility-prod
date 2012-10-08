package sim_mob.vis.network;

import java.awt.Color;
import java.util.ArrayList;

import sim_mob.vis.util.Utility;

//This class is used to store the int representation of the parent class's public members
//since all these information has be sent to intersection, they are bundled in a helpre class called SignalHelper
//so SignalHelper has a member variable in intersection+get/set functions
public class SignalHelper {
	public class Segment {
		public Segment(long segment_from_, long segment_to_) {
			segment_from = segment_from_;
			segment_to = segment_to_;
		}
		public Segment(String segment_from_, String segment_to_) {
			segment_from =  Utility.ParseLongOptionalHex(segment_from_);
			segment_to =  Utility.ParseLongOptionalHex(segment_to_);
		}

		public long segment_from;
		public long segment_to;
		
		public TrafficSignalLine generatedTrafficSignalLine;
	}

	public class Crossing {
		public Crossing(long id) {
			this.id = id;
		}

		public long id;
	}

	public class Phase {
		public Phase(String name) {
			this.name = name;
		};

		public String name;
		public ArrayList<Segment> segments;
		public ArrayList<Crossing> crossings;
		
		public Segment getSegmentPair(long segFrom, long segTo) {
			for(Segment segment : segments) {
				if((segment.segment_from == segFrom) && (segment.segment_to == segTo)) {
					return segment;
				}
			}
			return null;
		}
		
	}

	public long hex_id;
	public long node;
	public ArrayList<Phase> phases;
	public int STempVal;
	
	public Phase getPhase(String name) {
		for(Phase phase : phases) {
			if(phase.name.equals(name)) {
				return phase;
			}
		}
		return null;
	}
	
	public Color getColorObject(Integer color) {
		switch(color)
		{
		case 1:
			return Color.red;
		case 2:
			return Color.yellow;
		case 3:
			return Color.green;
		case 6:
			return Color.cyan;//just for now
		default:
			System.out.println("getColorObject Error, No such kind of traffic light[" + color + "  -- TrafficSignalCrossing, drawSignalCrossing()");
			return Color.darkGray;
		}
	}
	
}
