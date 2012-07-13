package sim_mob.vis.network;

import java.awt.Color;
import java.util.ArrayList;

//This class is used to store the int representation of the parent class's public members
//since all these information has be sent to intersection, they are bundled in a helpre class called SignalHelper
//so SignalHelper has a member variable in intersection+get/set functions
public class SignalHelper {
	public SignalHelper() {
	}

//	public class Link {
//		public Link() {
//		};
//
//		public Link(int link_from_, int link_to_) {
//			link_from = link_from_;
//			link_to = link_to_;
//		}
//		public Link(int link_from_, int link_to_) {
//			link_from = HexStringToInt(link_from_);
//			link_to = HexStringToInt(link_to_);
//		}
//
//		public int link_from;
//		public int link_to;
//	}
	public class Segment {
//		public Segment() {
//		};

		public Segment(int segment_from_, int segment_to_) {
			segment_from = segment_from_;
			segment_to = segment_to_;
		}
		public Segment(String segment_from_, String segment_to_) {
			segment_from =  HexStringToInt(segment_from_);
			segment_to =  HexStringToInt(segment_to_);
		}

		public int segment_from;
		public int segment_to;
		
		public TrafficSignalLine generatedTrafficSignalLine;
	}

	public class Crossing {
		public Crossing() {
		};

		public Crossing(int id_) {
			id = id_;
		}

		public int id;
		// public Integer current_color;
	}

	public class Phase {
		public Phase() {
		};

		public Phase(String name_) {
			name = name_;
		};

		public String name;
//		public ArrayList<Link> links;
		public ArrayList<Segment> segments;
		public ArrayList<Crossing> crossings;
		
		public Segment getSegmentPair(int segFrom, int segTo)
		{
			for(Segment segment:segments)
				if((segment.segment_from == segFrom)&&(segment.segment_to == segTo))
					return segment;
			return null;
		}
		
	}

	public int hex_id;
	public int node;
	public ArrayList<Phase> phases;
	public int STempVal;
	
	
	
	public Phase getPhase(String name_)
	{
		for(Phase phase : phases)
		{
			if(phase.name.equals(name_))
				return phase;
		}
		return null;
	}
	//we need a function to convert id from hex(string) representation to integer
	//I couldn't find any place better than signalhelper to put this function
	//I put it there and declared it as static to be accessible without instantiation
	public static Integer HexStringToInt(String res)
	{
		int i = 2;
		StringBuffer sb = new StringBuffer();
		if(!((res.charAt(0) == '0')&&(res.charAt(1) == 'x'))) return -1;
		for (;i < res.length(); i++) {
			sb.append(res.charAt(i));
		}
		return Integer.parseInt(sb.toString(), 16);
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
