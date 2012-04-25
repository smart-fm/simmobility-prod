package sim_mob.vis.network;

import java.awt.Font;
import java.awt.Graphics2D;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.Vect;

import edu.umd.cs.piccolo.nodes.PText;
import edu.umd.cs.piccolo.util.PPaintContext;

public class RoadName extends PText {
	private static final long serialVersionUID = 1L;
	private static Font roadNameFont = new Font("Arial", Font.PLAIN, 16);

	public RoadName(String name, Node start, Node end) {
		super(name);
		this.setFont(roadNameFont);
		
		//Translate
		float targetX = (float)(start.getX()+(end.getX()-start.getX())/2);
		float targetY = (float)(start.getY()+(end.getY()-start.getY())/2);
		setX(targetX);
		setY(targetY);
		
		//TEMP: This might be required in some form or another.
		setWidth(100);
		setHeight(100);
		
		//Rotate
		Vect line = new Vect(start.getX(), start.getY(), end.getX(), end.getY());
		rotateAboutPoint(Math.atan2(line.getMagY(), line.getMagX()), targetX, targetY);
	}
	
	protected void paint(PPaintContext paintContext) {
		Graphics2D g = paintContext.getGraphics();
		g.setColor(MainFrame.Config.getLineColor("roadname"));
		
		//Measure original
		double scaledHeight = roadNameFont.getSize() / paintContext.getScale();
		g.setFont(new Font(roadNameFont.getName(), roadNameFont.getStyle(), (int)scaledHeight));
		
		//Temp: make it look better
		double transUp = 350 * 4; //We should actually sccale it out based on how zoomed in it is. 
		
		//Measure translated
		double scaledWidth = g.getFontMetrics().stringWidth(getText());
		g.drawString(getText(), (int)(getX()-scaledWidth/2), (int)(getY()-transUp));
	}
	
}
