package sim_mob.vis.simultion;

import java.awt.*;
import java.awt.geom.*;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Car extends PPath {
	private static final long serialVersionUID = 1L;

	private int ID;
	private boolean fake;
	private int length;
	private int width;
	private int firstFrameID;
	
	public int getID(){return ID;}
	public int getCarLength(){return length;}
	public int getCarWidth() {return width;}
	public boolean getFake() { return fake; }
	public int getFirstFrameID() { return firstFrameID; }

	//TODO : this is test, to be changed
	//private static final int NODE_SIZE = 4;
	private Path2D.Double poly;

	public Car(){
						
		poly = new Path2D.Double();
		poly.moveTo(0, 0);
		poly.lineTo(0, 0);
		poly.lineTo(0, 0);
		poly.lineTo(0, 0);
		poly.closePath();
		
		this.setPathTo(poly);
	}
	

	public void setItFake(){
		fake = true;
	}
	public void setLenth(int length){
		this.length = length;
	}
	public void setWidth(int width){
		this.width = width;
	}
	public void setID(int id){
		this.ID = id;
	}
	public void setFirstFrameID(int frameID){
		this.firstFrameID = frameID;
	}
	
	protected void paint(PPaintContext paintContext){
		int tempWidth = 3;
		int tempHeight = 2;
		
		poly = new Path2D.Double();

        Graphics2D g = paintContext.getGraphics();
        poly.moveTo(0, 0);
        poly.lineTo(tempWidth,0);
        poly.lineTo(tempWidth, tempHeight);
        poly.lineTo(0, tempHeight);
        poly.closePath();
      
        AffineTransform newPos = new AffineTransform();
        newPos.translate(-tempWidth/2, -tempHeight/2);
        poly.transform(newPos);
		
        this.setPathTo(poly);
		g.setColor(Color.white);
		g.fill(poly);

        /*
        int[] coords = new int[]{(int)0-NODE_SIZE/2, (int)0-NODE_SIZE/2};
        g.setColor(Color.white);
		g.fillOval(coords[0], coords[1], NODE_SIZE, NODE_SIZE);		
		g.drawOval(coords[0], coords[1], NODE_SIZE	, NODE_SIZE);
		*/
	
	}	

}
