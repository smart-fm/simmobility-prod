package sim_mob.vis.controls;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;

import sim_mob.vis.network.ScaledPoint;
import sim_mob.vis.util.IntGetter;


/**
 * Double-buffered panel which can display an image and can do some dynamic drawing. 
 */
public class NetworkPanel extends JPanel implements ComponentListener {
	public static final long serialVersionUID = 1L;
	
	//Double-buffered to prevent flickering.
	private BufferedImage buffer;
	
	public NetworkPanel() {
		this.setIgnoreRepaint(true);
		this.setPreferredSize(new Dimension(300, 300));
		
		//ScaledPoint needs to know the height of the canvas.
		if (ScaledPoint.CanvasHeight==null) {
			ScaledPoint.CanvasHeight = new HeightGetter(this);
		}
		
		this.addComponentListener(this);
	}
	
	public void swapBuffer(int[] newRGB, int scanSize) {
		buffer.setRGB(0, 0, buffer.getWidth(), buffer.getHeight(), 
				newRGB, 0, scanSize);
	}
	
	protected void paintComponent(Graphics g) {
		//Paint background
		super.paintComponent(g);
					
		//Paint the bufer
		g.drawImage(buffer, 0, 0, null);
	}

	
	//Helper
	private static int CenterIfSmaller(int origValue, int srcValue, int destValue) {
		if (srcValue<=destValue) {
			origValue = destValue/2 - srcValue/2;
		}
		return origValue;
	}
	
	
	//Draw the map
	public void drawMap(NetworkVisualizer nv, int offsetX, int offsetY) {
		//Get 2D graphics obj.
		Graphics2D g = (Graphics2D)buffer.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//If the image is smaller than the canvas to draw it on, always center it.
		BufferedImage toDraw = nv.getImage();
		offsetX = CenterIfSmaller(offsetX, toDraw.getWidth(), buffer.getWidth());
		offsetY = CenterIfSmaller(offsetY, toDraw.getHeight(), buffer.getHeight());
		
		//Draw the network at the given offset; repaint
		g.drawImage(toDraw, offsetX, offsetY, null);
		this.repaint();
	}
	
	
	//Resize listener
	public void componentResized(ComponentEvent e) {
		buffer = new BufferedImage(this.getWidth(), this.getHeight(), BufferedImage.TYPE_INT_RGB);
		this.repaint();
	}

	
	//Helper class
	class WidthGetter implements IntGetter {
		NetworkPanel parent;
		WidthGetter(NetworkPanel parent) {
			this.parent = parent;
		}
		public int get() {
			return parent.getWidth();
		}
	}
	class HeightGetter implements IntGetter {
		NetworkPanel parent;
		HeightGetter(NetworkPanel parent) {
			this.parent = parent;
		}
		public int get() {
			return parent.getHeight();
		}
	}
	
	
	//Component method stubs
	public void componentHidden(ComponentEvent e) {}
	public void componentMoved(ComponentEvent e) {}
	public void componentShown(ComponentEvent e) {}
}

