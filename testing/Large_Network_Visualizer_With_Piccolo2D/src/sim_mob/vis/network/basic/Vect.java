package sim_mob.vis.network.basic;

import java.awt.geom.Point2D;


/**
 * Vector class, with convenience methods for scaling, etc.
 * 
 * \author Seth N. Hetu
 */
public class Vect {
	private DPoint pos;
	private DPoint mag;
	
	public Vect(double fromX, double fromY, double toX, double toY) {
		this.pos = new DPoint(fromX, fromY);
		this.mag = new DPoint(toX-fromX, toY-fromY);
	}
	
	public Vect(Point2D from, Point2D to) {
		this(from.getX(), from.getY(), to.getX(), to.getY());
	}
	  
	public Vect(double magX, double magY) {
		this(0, 0, magX, magY);
	}
	  
	public Vect(Vect other) {
		this(other.pos.x, other.pos.y, other.mag.x, other.mag.y);
	}
	  
	public double getEndX() {
		return pos.x + mag.x;
	}
	  
	public double getEndY() {
		return pos.y + mag.y;
	}
	
	public double getMagX() {
		return mag.x;
	}
	
	public double getMagY() {
		return mag.y;
	}
	
	public void setMag(double x, double y) {
		mag.x = x;
		mag.y = y;
	}
	
	public double getX() {
		return pos.x;
	}
	
	public double getY() {
		return pos.y;
	}
	  
	public double getMagnitude() {
		return Math.sqrt(mag.x*mag.x + mag.y*mag.y);
	}
	  
	public void makeUnit()  {
		scaleVect(1/getMagnitude());
	}
	  
	public void scaleVect(double val) {
		mag.x *= val;
		mag.y *= val;
	}

	public void translateVect(double dX, double dY) {
		pos.x += dX;
		pos.y += dY;
	}
	  
	public void translateVect() { //No arguments means translate by yourself.
		translateVect(mag.x, mag.y);
	}
	
	/*public double computeAngle() {
		return Math.atan(mag.y/mag.x);
	}*/
	  
	public void flipVecNormal(boolean direction) {
		int sign = direction ? 1 : -1;
		double newX = -mag.y*sign;
		double newY = mag.x*sign;
		mag.x = newX;
		mag.y = newY;
	}
}



