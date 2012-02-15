import java.awt.*;
import java.awt.geom.Point2D;
import java.awt.geom.Point2D.Double;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;

import javax.swing.*;


public class VectImgPanel extends JPanel {
	private static final long serialVersionUID = 1L;
	
	private static final Stroke thinLine = new BasicStroke(1.0F);
	
	private SimpleVectorImage currImg = null;
	private int currAngle = 0;
	private int currScale = 100;
	public void setCurrShape(SimpleVectorImage img) {
		this.currImg = img;
		this.repaint();
	}
	public void setCurrRotation(int value) {
		this.currAngle = value;
		this.repaint();
	}
	public void setCurrScale(int value) {
		this.currScale = value;
		this.repaint();
	}
	
	private void drawBox(Graphics2D g, Point2D center, int squareSz) {
		//Draw the border
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0));
		g.drawLine((int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0));
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0));
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0), 
				   (int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0));
		
		//Draw 45 degree angle marks
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0));
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()+squareSz/2.0), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()-squareSz/2.0));
		
		//Draw the 90 degree angle marks
		g.drawLine((int)Math.round(center.getX()), (int)Math.round(center.getY()-squareSz/2.0), 
				   (int)Math.round(center.getX()), (int)Math.round(center.getY()+squareSz/2.0));
		g.drawLine((int)Math.round(center.getX()-squareSz/2.0), (int)Math.round(center.getY()), 
				   (int)Math.round(center.getX()+squareSz/2.0), (int)Math.round(center.getY()));
	}
	

	protected void paintComponent(Graphics g1) {
		//Convert
		Graphics2D g = (Graphics2D)g1;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//Paint background
		super.paintComponent(g);
		
		//Anything to draw?
		if (currImg==null) {
			//Something 
			g.setColor(Color.red);
			g.fillOval(50, 50, getWidth()-100, getHeight()-100);
			return;
		}
		
		//Draw a few lines for comparison
		Point2D center = new Point2D.Double(getWidth()/2.0, getHeight()/2.0);
		int squareSz = Math.min(getWidth(), getHeight()) - 20;
		g.setColor(Color.blue);
		g.setStroke(thinLine);
		drawBox(g, center, squareSz);
		
		//Draw the current image in the center
		BufferedImage img = currImg.getImage(currScale/100.0, currAngle);
		g.drawImage(img, getWidth()/2-img.getWidth()/2, getHeight()/2-img.getHeight()/2, null);
		

	}
}
