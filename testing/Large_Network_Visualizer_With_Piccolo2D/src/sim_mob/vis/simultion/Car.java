package sim_mob.vis.simultion;

import java.awt.*;
import java.awt.geom.*;
import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Car extends PNode {
	private static final long serialVersionUID = 1L;

	private int ID;
	private boolean fake;
	private int carLength;
	private int carWidth;
	private int firstFrameID;
	
	public int getID(){return ID;}
	public int getCarLength(){return carLength;}
	public int getCarWidth() {return carWidth;}
	public boolean getFake() { return fake; }
	public int getFirstFrameID() { return firstFrameID; }

	//TODO : this is test, to be changed
	//private static final int NODE_SIZE = 4;
	//private Path2D.Double poly;

	public Car(int id) {
		this.ID = id;
		
		//For bounds checking.
		this.carLength = 400;
		this.carWidth = 200;
		this.setBounds(0, 0, getCarLength(), getCarWidth());
	}
	

	public void setItFake(){
		fake = true;
	}
	public void setID(int id){
		this.ID = id;
	}
	public void setFirstFrameID(int frameID){
		this.firstFrameID = frameID;
	}
	
	protected void paint(PPaintContext paintContext) {
		//TEMP: For now, just draw a rectangle. We can add in SimpleVectorShape later.
		//NOTE: Look at the RoadName class for an example of how to handle rotation easily. ~Seth
		Rectangle2D r = new Rectangle2D.Double(getX()-getCarLength()/2, getY()-getCarWidth()/2, getCarLength(), getCarWidth());
		
		Graphics2D g = paintContext.getGraphics();
		g.setColor(Color.cyan);
		g.fill(r);
		g.setColor(Color.magenta);
		g.setStroke(new BasicStroke((float)(1.5 / paintContext.getScale())));
		g.draw(r);
	}	

}
