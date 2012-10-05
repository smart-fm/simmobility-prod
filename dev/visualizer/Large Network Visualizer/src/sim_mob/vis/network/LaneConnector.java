package sim_mob.vis.network;


/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class LaneConnector{

	private long fromSegment;
	private long toSegment;
	private long fromLane;
	private long toLane;
	
	private Node fromNode;
	private Node toNode;
	
	public Node getFromNode(){return fromNode; }
	public Node getToNode(){return toNode; }
	
	public long getFromSegment(){return fromSegment; }
	public long getToSegment(){return toSegment;}
	public long getFromLane(){return fromLane;}
	public long getToLane(){return toLane;}
	
	
	public LaneConnector(long fromSegment, long toSegment, long fromLane, long toLane){
		this.fromSegment = fromSegment;
		this.toSegment = toSegment;
		this.fromLane = fromLane;
		this.toLane = toLane;
		
	}
	
	
		
}
