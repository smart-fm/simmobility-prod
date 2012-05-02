package sim_mob.act;


/**
 * An Activity is some action which can be performed at a later time, combined
 * with the necessary state variables it must maintain to do so. In this sense, it
 * is somewhat similar to a closure (but its subclasses change this behavior).
 * 
 * \author Seth N. Hetu
 */
public abstract class Activity {
	
	/**
	 * Perform this activity.
	 * 
	 * @param args Usually empty; provided to facilitate more complicated activities.
	 * @return Most activities return null, but you may also return any Object.
	 */
	public abstract Object run(Object... args);

}
