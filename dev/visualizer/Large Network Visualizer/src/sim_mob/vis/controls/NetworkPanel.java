package sim_mob.vis.controls;

import java.awt.*;

import java.awt.event.*;
import java.awt.image.BufferedImage;
import javax.swing.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.Node;
import sim_mob.vis.util.*;



/**
 * Double-buffered panel which can display an image and can do some dynamic drawing. 
 */
public class NetworkPanel extends JPanel implements ComponentListener, MouseListener, MouseMotionListener, MouseWheelListener {
	public static final long serialVersionUID = 1L;
	
	//Final display
	private static Font FrameFont = new Font("Arial", Font.PLAIN, 18);
	
	//Double-buffered to prevent flickering.
	private BufferedImage buffer;
	
	//Call backs to the parent
	private StringSetter statusBarUpdate;
	
	//For dragging
	private Point mouseDown = new Point(0, 0);
	private Point mouseFirstDown = new Point(0, 0);
	private static final double DRAG_THRESHOLD = 45;
	
	//TEMP: This is actually better off somewhere else.
	private Point offset = new Point(0, 0);
	private NetworkVisualizer netViewCache;
	
	
	public NetworkPanel(StringSetter statusBarUpdate) {
		this.setIgnoreRepaint(true);
		this.setPreferredSize(new Dimension(300, 300));
		this.addComponentListener(this);
		this.addMouseListener(this);
		this.addMouseMotionListener(this);
		this.addMouseWheelListener(this);
		
		this.statusBarUpdate = statusBarUpdate;
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
	
	
	public boolean jumpAnim(int toTick, JSlider slider) {
		//Set
		if (netViewCache==null || !netViewCache.setCurrFrameTick(toTick)) {
			return false;
		}
		
		//Update the slider, if it exists
		if (slider!=null) {
			slider.setEnabled(false);
			slider.setValue(netViewCache.getCurrFrameTick());
			slider.setEnabled(true);
		}

		netViewCache.redrawAtCurrScale();
		updateMap();
		return true;
	}
	public boolean advanceAnimbyStep(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !netViewCache.incrementCurrFrameTick(ticks)) {
			return false;
		}
		
		return jumpAnim(netViewCache.getCurrFrameTick(), slider);
	}
	
	
	
	public boolean advanceAnim(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !netViewCache.incrementCurrFrameTick(1)) {
			return false;
		}
		
		return jumpAnim(netViewCache.getCurrFrameTick(), slider);
	}
	
	
	//Draw the map
	public void drawMap(NetworkVisualizer nv, int offsetX, int offsetY) {
		//Save for later.
		offset.x = offsetX;
		offset.y = offsetY;
		netViewCache = nv;
		
		updateMap();
	}
	
	
	private void clickMap(Point pos) {
		//Anything?
		if (netViewCache==null) {
			return;
		}
		
		//First, add the offset
		pos.x += offset.x;
		pos.y += offset.y;
		
		//Now scale it to the map's co-ordinate system and get
		//  whichever object is at that location
		Node n = netViewCache.getNodeAt(pos);
		if (n!=null) {
			String str = String.format("NODE: %.0f , %.0f", n.getPos().getUnscaledX(), n.getPos().getUnscaledY());
			statusBarUpdate.set(str);
		}
		
	}
	
	
	private void updateMap() {
		//Anything?
		if (netViewCache==null) {
			return;
		}
		BufferedImage drawImg = netViewCache.getImage();
		
		//Get 2D graphics obj.
		Graphics2D g = (Graphics2D)buffer.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//If the image is smaller than the canvas to draw it on, always center it.
		//Check bounds too; we don't want to scroll the map off the screen.
		offset.x = CenterAndBoundsCheck(offset.x, drawImg.getWidth(), buffer.getWidth());
		offset.y = CenterAndBoundsCheck(offset.y, drawImg.getHeight(), buffer.getHeight());
		
		//If the image is smaller in at least one dimension;we should re-fill the background with light-gray.
		if ((drawImg.getWidth()<buffer.getWidth()) || (drawImg.getHeight()<buffer.getHeight())) {
			g.setBackground(MainFrame.Config.getBackground("panel"));
			g.clearRect(0, 0, buffer.getWidth(), buffer.getHeight());
		}
		
		//Draw the network at the given offset
		g.drawImage(drawImg, offset.x, offset.y, null);
		
		//Draw traffic lights... it's a bit of a hack
	//	netViewCache.drawTrafficLights(g);
		
		//Draw the current frame
		if (netViewCache!=null) {
			g.setFont(FrameFont);
			g.setColor(MainFrame.Config.getBackground("framenumber"));
			g.drawString("Frame: "+netViewCache.getCurrFrameTick() , 15, 10+g.getFontMetrics().getAscent());
		}
		
		//Repaint
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
		mouseFirstDown.x = e.getX();
		mouseFirstDown.y = e.getY();
		updateMouseDownPos(e);
	}
	public void mouseReleased(MouseEvent e) {
		double diff = mouseFirstDown.distance(e.getX(), e.getY());
		if (diff<=DRAG_THRESHOLD) {
			//This "drag" was probably a mouse click.
			clickMap(new Point(e.getX(), e.getY()));
		}
	}
	public void mouseDragged(MouseEvent e) {
		//Update mouse's position; get offset 
		offset.x += e.getX()-mouseDown.x;
		offset.y += e.getY()-mouseDown.y;
		updateMouseDownPos(e);
		
		//Redraw the map at this new offset.
		updateMap();
	}
	
	
	//Zooming with the mouse wheel
	public void mouseWheelMoved(MouseWheelEvent e) {
		//Get the old width/height for comparison
		double oldW = netViewCache.getImage().getWidth();
		double oldH = netViewCache.getImage().getHeight();
		
		//Zoom
		netViewCache.zoomIn(-e.getWheelRotation());
		
		//NOTE: The math isn't quite right for scaling; will fix this later.
		//      A correct fix will create a temporary "ScaledPoint" that represents the center
		//      of the canvas (offset.x+width/2, offset.y+height/2) as an actual point on the map
		//      After scaling, just translate this point back to screen co-ordinates and subtract
		//      width/2, height/2 to get the correct new offset. 
		//      For now, just "nudging" the value slightly.
		double modAmtX = netViewCache.getImage().getWidth()>oldW ? 1.1 : 0.9;
		double modAmtY = netViewCache.getImage().getHeight()>oldH ? 1.1 : 0.9;
		
		//Modify the offset accordingly
		offset.x = (int)((modAmtX*offset.x*netViewCache.getImage().getWidth())/oldW);
		offset.y = (int)((modAmtY*offset.y*netViewCache.getImage().getHeight())/oldH);
		
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
	
	//Mouse motion method stubs
	public void mouseMoved(MouseEvent e) {}

	
}

