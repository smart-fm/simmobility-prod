package sim_mob.vis.network;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class Lane {
	private int laneNumber;
	private Node startMiddleNode;
	private Node endMiddleNode;

	
	public int getLaneNumber(){return laneNumber;}
	public Node getStartMiddleNode(){return startMiddleNode;}
	public Node getEndMiddleNode(){return endMiddleNode;}
	
	public Lane(int laneNumber, Node startMiddleNode, Node endMiddleNode){
		this.laneNumber = laneNumber;
		this.startMiddleNode = startMiddleNode;
		this.endMiddleNode = endMiddleNode;
		
	}
				
}
