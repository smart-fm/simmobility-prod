package sim_mob.vis.network;

import java.util.ArrayList;

public class SetOfTurnings {
	public int linkNumber;
	
	public ArrayList<TrafficSignalLine> leftTurnConnectors = new ArrayList<TrafficSignalLine>();
	public ArrayList<TrafficSignalLine> rightTurnConnectors = new ArrayList<TrafficSignalLine>();
	public ArrayList<TrafficSignalLine> straightConnectors = new ArrayList<TrafficSignalLine>();
}