package sim_mob.act;


/**
 * An activity which is divided into two parts: a begin() and an end(). 
 * 
 * \author Seth N. Hetu
 */
public abstract class BifurcatedActivity extends Activity {
	
	/**
	 * Calls begin(args) then end(args) and returns the result of end(). 
	 * In general, there is no reason to call this function; a Bifurcated Activity
	 * is meant to be broken into two pieces by its very nature.  
	 */
	public Object run(Object... args) {
		begin(args);
		return end(args);
	}
	
	public abstract Object begin(Object... args);
	public abstract Object end(Object... args);

}
