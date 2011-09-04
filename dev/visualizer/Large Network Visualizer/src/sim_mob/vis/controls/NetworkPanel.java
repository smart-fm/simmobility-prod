package sim_mob.vis.controls;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import javax.swing.JPanel;
import sim_mob.vis.util.IntGetter;


/**
 * Double-buffered panel which can display an image and can do some dynamic drawing. 
 */
public class NetworkPanel extends JPanel implements ComponentListener, MouseListener, MouseMotionListener, MouseWheelListener {
	public static final long serialVersionUID = 1L;
	
	//Double-buffered to prevent flickering.
	private BufferedImage buffer;
	
	//For dragging
	private Point mouseDown = new Point(0, 0);
	
	//TEMP: This is actually better off somewhere else.
	private Point offset = new Point(0, 0);
	private BufferedImage drawImg;
	
	public NetworkPanel() {
		this.setIgnoreRepaint(true);
		this.setPreferredSize(new Dimension(300, 300));
		this.addComponentListener(this);
		this.addMouseListener(this);
		this.addMouseMotionListener(this);
		this.addMouseWheelListener(this);
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
	private static int CenterAndBoundsCheck(int origValue, int srcValue, int destValue) {
		if (srcValue<=destValue) {
			//Center
			origValue = destValue/2 - srcValue/2;
		} else {
			//Bounds check; it's easier to do this negatively.
			origValue *= -1;
			origValue = -Math.min(Math.max(0,origValue), srcValue-destValue);
		}
		return origValue;
	}
	
	
	//Draw the map
	public void drawMap(NetworkVisualizer nv, int offsetX, int offsetY) {
		//Save for later.
		offset.x = offsetX;
		offset.y = offsetY;
		drawImg = nv.getImage();
		
		updateMap();
	}
	
	
	private void updateMap() {
		//Anything?
		if (drawImg==null) {
			return;
		}
		
		//Get 2D graphics obj.
		Graphics2D g = (Graphics2D)buffer.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//If the image is smaller than the canvas to draw it on, always center it.
		//Check bounds too; we don't want to scroll the map off the screen.
		offset.x = CenterAndBoundsCheck(offset.x, drawImg.getWidth(), buffer.getWidth());
		offset.y = CenterAndBoundsCheck(offset.y, drawImg.getHeight(), buffer.getHeight());
		
		//Draw the network at the given offset; repaint
		g.drawImage(drawImg, offset.x, offset.y, null);
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
	
	
	//Mouse click+drag
	void updateMouseDownPos(MouseEvent e) {
		mouseDown.x = e.getX();
		mouseDown.y = e.getY();
	}
	public void mousePressed(MouseEvent e) {
		updateMouseDownPos(e);
	}
	public void mouseDragged(MouseEvent e) {
		//Update mouse's position; get offset 
		offset.x += e.getX()-mouseDown.x;
		offset.y += e.getY()-mouseDown.y;
		updateMouseDownPos(e);
		
		//Redraw the map at this new offset.
		updateMap();
	}
	
	
	//Component method stubs
	public void componentHidden(ComponentEvent e) {}
	public void componentMoved(ComponentEvent e) {}
	public void componentShown(ComponentEvent e) {}
	
	//Mouse method stubs
	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}
	public void mouseExited(MouseEvent e) {}
	public void mouseReleased(MouseEvent e) {}
	
	//Mouse motion method stubs
	public void mouseMoved(MouseEvent e) {}
	
	//Mouse wheel listener stubs
	public void mouseWheelMoved(MouseWheelEvent e) {}
	
}

