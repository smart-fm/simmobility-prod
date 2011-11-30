package simulation_results;

import java.util.HashSet;

public class TimeTick {
	public HashSet<Driver> drivers = new HashSet<Driver>();
	public HashSet<Pedestrian> pedestrians = new HashSet<Pedestrian>();
	public HashSet<Signal> signals = new HashSet<Signal>();
}
