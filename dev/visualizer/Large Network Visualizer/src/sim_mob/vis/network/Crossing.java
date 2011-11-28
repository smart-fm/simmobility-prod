package sim_mob.vis.network;

import java.awt.*;

import sim_mob.vis.controls.DrawableItem;

public class Crossing implements DrawableItem{
	//Constants/Resources
	private static Color crossingColor = new Color(0x00, 0x9a, 0xcd);
	private static Stroke crossingStroke = new BasicStroke(1.0F);
	private float alpha = 0.5f;
	

	private Node nearOne;
	private Node nearTwo;
	private Node farOne;
	private Node farTwo;
	private int id;
	
	public Crossing(Node nearOne, Node nearTwo, Node farOne, Node farTwo,int id) {
		this.nearOne = nearOne;
		this.nearTwo = nearTwo;
		this.farOne = farOne;
		this.farTwo = farTwo;
		this.id = id;
	}

	public Node getNearOne() { return nearOne; }
	public Node getNearTwo() { return nearTwo; }
	public Node getFarOne() { return farOne; }
	public Node getFarTwo() { return farTwo; }
	public int getId() { return id; }
	
	public void draw(Graphics2D g) {
		g.setColor(crossingColor);
		g.setStroke(crossingStroke);
		drawCrossing(g);
	}
	
	
	public void drawSignalCrossing(Graphics2D g, Integer light){

		if(light == 2){
			
			g.setColor(Color.yellow);
			g.setStroke(crossingStroke);
			drawCrossing(g);
			
		} else if(light == 3){
			g.setColor(Color.green);
			g.setStroke(crossingStroke);
			drawCrossing(g);
		}
		
	}
	
	public void drawCrossing(Graphics2D g){

		g.drawLine((int)nearOne.getPos().getX(), (int)nearOne.getPos().getY(), (int)nearTwo.getPos().getX(), (int)nearTwo.getPos().getY()); 
		g.drawLine((int)farOne.getPos().getX(), (int)farOne.getPos().getY(), (int)farTwo.getPos().getX(), (int)farTwo.getPos().getY()); 

		Polygon poly = new Polygon();		
		poly.addPoint((int)nearOne.getPos().getX(), (int)nearOne.getPos().getY());
		poly.addPoint((int)nearTwo.getPos().getX(), (int)nearTwo.getPos().getY());
		poly.addPoint((int)farTwo.getPos().getX(), (int)farTwo.getPos().getY());
		poly.addPoint((int)farOne.getPos().getX(), (int)farOne.getPos().getY());
        g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,alpha));
		g.fillPolygon(poly);
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,1.0f));
	}
	

}
