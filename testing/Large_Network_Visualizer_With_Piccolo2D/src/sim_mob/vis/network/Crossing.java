package sim_mob.vis.network;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Path2D;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class Crossing extends PPath{
	
	/**
	 * 
	 */
	
	private static final long serialVersionUID = 1L;
	//Constants/Resources
	private static Color crossingColor = new Color(0x00, 0x9a, 0xcd);
	private float alpha = 0.5f;
	
	private int status = 0;
	
	private Node nearOne;
	private Node nearTwo;
	private Node farOne;
	private Node farTwo;
	private int id;
	
	private Path2D.Double poly;

	public Crossing(Node nearOne, Node nearTwo, Node farOne, Node farTwo,int id) {
		this.nearOne = nearOne;
		this.nearTwo = nearTwo;
		this.farOne = farOne;
		this.farTwo = farTwo;
		this.id = id;
		
		poly = new Path2D.Double();
		poly.moveTo(0, 0);
		poly.lineTo(0, 0);
		poly.lineTo(0, 0);
		poly.lineTo(0, 0);
		poly.closePath();
		
		this.setPathTo(poly);
	}

	public Node getNearOne() { return nearOne; }
	public Node getNearTwo() { return nearTwo; }
	public Node getFarOne() { return farOne; }
	public Node getFarTwo() { return farTwo; }
	public int getId() { return id; }


	
	public void paint(PPaintContext paintContext){

		Graphics2D g = paintContext.getGraphics();
		poly = new Path2D.Double();
		
		poly.moveTo(nearOne.getLocalPos().getX(), nearOne.getLocalPos().getY());
		poly.lineTo(nearTwo.getLocalPos().getX(), nearTwo.getLocalPos().getY());
		poly.lineTo(farTwo.getLocalPos().getX(), farTwo.getLocalPos().getY());
		poly.lineTo(farOne.getLocalPos().getX(), farOne.getLocalPos().getY());
		poly.closePath();
		
		this.setPathTo(poly);
		if(status == 0){	
			g.setColor(crossingColor);
		}if(status == 1){
			g.setColor(Color.RED);
		}if(status == 2){
			g.setColor(Color.YELLOW);
		}if(status == 3){
			g.setColor(Color.GREEN);
		}
	
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,alpha));
		g.fill(poly);	
		
	}
	

	public void changeColor(int st){		
		status = st;
		this.repaint();
		//crossingColor = new Color(0x00, 0x9a, 0xcd);
	}
	
}
