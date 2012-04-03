package sim_mob.vis.network;

import java.awt.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;

/**
 * Annotations show small bits of additional data and may be offset by an amount.
 * 
 * \author Seth N. Hetu
 */
public class Annotation implements DrawableItem {
	//Drawable characteristics
	private static final Stroke AnnotationStroke = new BasicStroke(1.0F);
	private static final int MinimumAnnotationWidth = 42; //pixels
	private Color bkgrdColor;
	private Color borderColor;
	private Color fontColor;

	//Position to draw this annotation
	//The offset is a magnitude (x,y) amount to offset the position by.
	private ScaledPoint pos;
	private ScaledPoint offset;
	
	//The message to show at this location.
	public String message;
	
	
	public Annotation(Point position, String message) {
		pos = new ScaledPoint(position.x, position.y, null);
		this.message = message;
		bkgrdColor = Color.gray;
		borderColor = Color.white;
		fontColor = Color.black;
	}
	
	//Accessors/mutators
	public void setBackgroundColor(Color clr) { bkgrdColor = clr; }
	public void setBorderColor(Color clr) { borderColor = clr; }
	public void setFontColor(Color clr) { fontColor = clr; }
	public void setOffset(Point offset) { this.offset = new ScaledPoint(offset.x, offset.y, null); }
	public ScaledPoint getPos() { return pos; }
	

	public void draw(Graphics2D g) {
		//Skip?
		if (message.isEmpty()) { return; }
		
		//Measure
		final int extra = 4;
		final int margin = 1;
		g.setFont(new Font("Arial", Font.PLAIN, 10));
		int width = Math.max(g.getFontMetrics().stringWidth(message), MinimumAnnotationWidth);
		int height = g.getFontMetrics().getHeight() + margin;
		int startX = (int)(pos.getX() + (offset!=null?offset.getX():0));
		int startY = (int)(pos.getY() + (offset!=null?offset.getY():0)) - (height + extra);
		
		//Position the baseline accurately
		int fontH = height/2 + g.getFontMetrics().getAscent()/2 + 1;
		
		//Draw
		g.setColor(bkgrdColor);
		g.fillRect(startX, startY, width, height);
		g.setColor(borderColor);
		g.setStroke(AnnotationStroke);
		g.drawRect(startX, startY, width, height);
		g.setColor(fontColor); 
		g.drawString(message, startX+extra, startY+fontH);
	}
	
	
	public String toString() {
		return "(" + pos.getUnscaledX() + "," + pos.getUnscaledY() + ")"; 
	}
}

