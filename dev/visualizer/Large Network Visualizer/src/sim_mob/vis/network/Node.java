package sim_mob.vis.network;

/**
 * Nodes represent locations in our network.
 */
public class Node {
	private ScaledPoint pos;
	private boolean isUni;   //Rather than having multiple classes....
	
	public Node(double x, double y, boolean isUni) {
		pos = new ScaledPoint(x, y);
		this.isUni = isUni;
	}
	
	public ScaledPoint getPos() {
		return pos;
	}
	
	public boolean getIsUni() {
		return isUni;
	}
}
