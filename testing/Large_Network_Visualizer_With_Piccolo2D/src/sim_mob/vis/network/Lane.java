package sim_mob.vis.network;

import java.awt.geom.Point2D;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 */
public class Lane {
	private int laneNumber;
	private Point2D startMiddlePt;
	private Point2D endMiddlePt;

	
	public int getLaneNumber(){return laneNumber;}
	public Point2D getStartMiddlePoint(){return startMiddlePt;}
	public Point2D getEndMiddlePoint(){return endMiddlePt;}
	
	public Lane(int laneNumber, Point2D startMiddlePt, Point2D endMiddlePt){
		this.laneNumber = laneNumber;
		this.startMiddlePt = startMiddlePt;
		this.endMiddlePt = endMiddlePt;
		
	}
				
}
