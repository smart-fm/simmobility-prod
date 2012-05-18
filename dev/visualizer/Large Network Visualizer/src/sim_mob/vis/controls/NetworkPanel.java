package sim_mob.vis.controls;

import java.awt.*;

import java.awt.event.*;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import javax.swing.*;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.Node;
import sim_mob.vis.util.*;



/**
 * Double-buffered panel which can display an image and can do some dynamic drawing.
 * \author Seth N. Hetu
 * \author Zhang Shuai 
 */
public class NetworkPanel extends JPanel implements ComponentListener, MouseListener, MouseMotionListener, MouseWheelListener {
	public static final long serialVersionUID = 1L;
	
	//Final display
	private static Font FrameFont = new Font("Arial", Font.PLAIN, 18);
	
	//Call backs to the parent
	private StringSetter statusBarUpdate;
	
	//For dragging
	private Point mouseDown = new Point(0, 0);
	private Point mouseFirstDown = new Point(0, 0);
	private static final double DRAG_THRESHOLD = 45;
	
	//Current scale multiplier
	public void setScaleMultiplier(int val) { 
		if (netViewCache==null) { return; }
		netViewCache.setScaleMultiplier(val);
	}
	
	//TEMP: This is actually better off somewhere else.
	//private Point offset = new Point(0, 0);
	private NetworkVisualizer netViewCache;
	
	private int currFrameTick;
		
	//Which Agent ID to highlight
	public void setHighlightID(int id) { 
		if (netViewCache!=null)  {
			netViewCache.setHighlightID(id);
		} //NOTE: This might have to be synchronized.
	}
	
	
	private ProgressItem currProgressItem;
	private class ProgressItem {
		double amt;
		boolean amtIsPercent;
		Color color;
		String caption;
		ProgressItem(double amt, boolean amtIsPercent, Color color, String caption) {
			this.amt = amt;
			this.amtIsPercent = amtIsPercent;
			this.color = color;
			this.caption = caption;
		}
	}
	
	
	public NetworkPanel(StringSetter statusBarUpdate) {
		this.setIgnoreRepaint(true);
		this.setPreferredSize(new Dimension(300, 300));
		this.addComponentListener(this);
		this.addMouseListener(this);
		this.addMouseMotionListener(this);
		this.addMouseWheelListener(this);
		
		this.statusBarUpdate = statusBarUpdate;
		
		//Hack a little
		this.setBackground(Color.darkGray);
	}
	
	public boolean incrementCurrFrameTick(int amt) {
		return setCurrFrameTick(currFrameTick+amt);
	}
	public boolean setCurrFrameTick(int newVal) {
		if (newVal<0 || netViewCache==null || newVal>=netViewCache.getMaxFrameTick()) {
			return false;
		}
		currFrameTick = newVal;
		return true;
	}
	
	
	private void drawCurrProgressItem(Graphics2D g) {
		//Clear.
		int width = this.getWidth();
		int height = this.getHeight();
		g.setColor(Color.darkGray);
		g.fillRect(0, 0, width, height);
		
		//Calc
		double percent = currProgressItem.amtIsPercent?currProgressItem.amt:1.0;
		
		//Rectangle.
		int margin = 20;
		int barSize = 100;
		Rectangle2D bar = new Rectangle2D.Double(margin, height/2-barSize/2, width-margin*2, barSize);
		g.setColor(Color.black);
		g.fillRect((int)bar.getX(), (int)bar.getY(), (int)bar.getWidth(), (int)bar.getHeight());
		g.setColor(currProgressItem.color);
		g.fillRect((int)bar.getX(), (int)bar.getY(), (int)(bar.getWidth()*percent), (int)bar.getHeight());
		g.setColor(Color.white);
		g.drawRect((int)bar.getX(), (int)bar.getY(), (int)bar.getWidth(), (int)bar.getHeight());
		
		//Amount
		String toDraw = currProgressItem.amtIsPercent ? currProgressItem.caption : "" + (int)(currProgressItem.amt/1024) + " kB";
		if (!toDraw.isEmpty()) {
			int toDrawLen = g.getFontMetrics().stringWidth(toDraw);
			
			g.setColor(Color.white);
			g.drawString(toDraw, width/2-toDrawLen/2, height/2-g.getFontMetrics().getHeight()/2);
		} 
	}

	
	protected void paintComponent(Graphics g) {
		//Paint background
		super.paintComponent(g);
		
		//Progress item to draw?
		if (this.currProgressItem!=null) {
			drawCurrProgressItem((Graphics2D)g);
			this.currProgressItem = null;
		}
		
		//Anything to draw?
		if (netViewCache==null) {
			return;
		}
		
		//Make a buffer, draw it.
		Point size = new Point(this.getWidth(), this.getHeight());
		BufferedImage buffer = new BufferedImage(size.x, size.y, BufferedImage.TYPE_INT_RGB);
		BufferedImage drawImg = netViewCache.getImageAtTimeTick(getCurrFrameTick(), size);
		drawMapOntoImage(buffer, drawImg, getCurrFrameTick());
		
		//Paint the bufer
		g.drawImage(buffer, 0, 0, null);
	}
	
	
	//Percent is from [0.0 .. 1.0]
	public void drawBufferAsProgressBar(double amt, boolean amtIsPercent, Color color, String caption) {
		//Delay
		this.currProgressItem = new ProgressItem(amt, amtIsPercent, color, caption);
		this.repaint();
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
		if (netViewCache==null || !setCurrFrameTick(toTick)) {
			return false;
		}
		
		//Update the slider, if it exists
		if (slider!=null) {
			slider.setEnabled(false);
			slider.setValue(getCurrFrameTick());
			slider.setEnabled(true);
		}

		netViewCache.redrawFrame(getCurrFrameTick());  //NOTE: This is probably redundant.
		repaint();
		return true;
	}
	
	public boolean advanceAnimbyStep(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !incrementCurrFrameTick(ticks)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public boolean advanceAnim(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !incrementCurrFrameTick(1)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public int getCurrFrameTick() {
		return currFrameTick;
	}
	
	public int getMaxFrameTick() {
		return netViewCache.getMaxFrameTick();
	}
	
	
	//Draw the map
	public void initMapCache(NetworkVisualizer nv) {
		//Save for later.
		//offset.x = offsetX;
		//offset.y = offsetY;
		netViewCache = nv;
		currFrameTick = 0;
		
		repaint();
	}
	
	public void showFakeAgent(boolean drawFakeAgent){
		
		if(netViewCache == null){
			return;
		}
		netViewCache.toggleFakeAgent(drawFakeAgent, getCurrFrameTick());
		repaint();
	}
	
	public void showDebugMode(boolean debugOn){
		
		if(netViewCache == null){
			return;
		}
		netViewCache.toggleDebugOn(debugOn, getCurrFrameTick());
		repaint();
	}
	
	public void setAnnotationLevel(boolean showAimsun, boolean showMitsim) {
		if(netViewCache == null) {
			return; 
		}
		netViewCache.setAnnotationLevel(showAimsun, showMitsim, getCurrFrameTick());
		repaint();
	}
	
	private void clickMap(Point pos) {
		//Anything?
		if (netViewCache==null) {
			return;
		}
		
		//First, add the offset
		//NOTE: This won't work at the moment. We can get this easily be multiplying by the inverse
		//      of the scale factor, but no-one's using this functionality anyway.
		/*pos.x += offset.x;
		pos.y += offset.y;
		
		//Now scale it to the map's co-ordinate system and get
		//  whichever object is at that location
		Node n = netViewCache.getNodeAt(pos);
		if (n!=null) {
			String str = String.format("NODE: %.0f , %.0f", n.getPos().getUnscaledX(), n.getPos().getUnscaledY());
			statusBarUpdate.set(str);
		}*/
	}
	
	
	public BufferedImage drawFrameToExternalBuffer(int tick, boolean showFrameNumber, int imageType) {
		//Sanity check.
		if (netViewCache==null) { throw new RuntimeException("Unexptected: newViewCache is null."); }
		
		//Step 1: prepare a return image
		BufferedImage resImg = new BufferedImage(this.getWidth(), this.getHeight(), imageType);
		
		//Step 2: re-draw the original image.
		Point size = new Point(getWidth(), getHeight());
		BufferedImage drawImg = netViewCache.getImageAtTimeTick(tick, size, imageType);
		drawMapOntoImage(resImg, drawImg, tick, showFrameNumber);
		
		return resImg;
	}
	
	
	private void drawMapOntoImage(BufferedImage destImg, BufferedImage drawImg, int frameNumber) {
		drawMapOntoImage(destImg, drawImg, frameNumber, true);
	}
	private void drawMapOntoImage(BufferedImage destImg, BufferedImage drawImg, int frameNumber, boolean showFrameNumber) {
		//Get 2D graphics obj.
		Graphics2D g = (Graphics2D)destImg.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		//If the image is smaller than the canvas to draw it on, always center it.
		//Check bounds too; we don't want to scroll the map off the screen.
		//offset.x = CenterAndBoundsCheck(offset.x, drawImg.getWidth(), destImg.getWidth());
		//offset.y = CenterAndBoundsCheck(offset.y, drawImg.getHeight(), destImg.getHeight());
		
		//If the image is smaller in at least one dimension;we should re-fill the background with light-gray.
		if ((drawImg.getWidth()<destImg.getWidth()) || (drawImg.getHeight()<destImg.getHeight())) {
			g.setBackground(MainFrame.Config.getBackground("panel"));
			g.clearRect(0, 0, destImg.getWidth(), destImg.getHeight());
		}
		
		//Draw the network at the given offset
		//g.drawImage(drawImg, offset.x, offset.y, null);
		g.drawImage(drawImg, 0, 0, null);
		
		//Draw the current frame ID
		if (showFrameNumber) {
			g.setFont(FrameFont);
			g.setColor(MainFrame.Config.getBackground("framenumber"));
			g.drawString("Frame: "+frameNumber , 15, 10+g.getFontMetrics().getAscent());
		}
	}
	
		
	//Resize listener
	public void componentResized(ComponentEvent e) {
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
		netViewCache.translateBy(getCurrFrameTick(), new Point(getWidth(), getHeight()), -(e.getX()-mouseDown.x), -(e.getY()-mouseDown.y));
		updateMouseDownPos(e);
		
		//Redraw the map at this new offset.
		repaint();
	}
	
	//Zooming with the mouse wheel
	public void mouseWheelMoved(MouseWheelEvent e) {
		zoomView(-e.getWheelRotation());
	}
	
	//Zooming with button click
	public void zoomWithButtonClick(int number){
		zoomView(number);
	}
	
	
	private void zoomView(int number) {
		//Get the current view.
		Rectangle2D view = netViewCache.getCurrentView();
		
		//Get the old width/height for comparison
		//double oldW = netViewCache.getImage().getWidth();
		//double oldH = netViewCache.getImage().getHeight();
		
		//Zoom
		netViewCache.zoomIn(-number, getCurrFrameTick(), new Point(getWidth(), getHeight()));
		
		//NOTE: The math isn't quite right for scaling; will fix this later.
		//      A correct fix will create a temporary "ScaledPoint" that represents the center
		//      of the canvas (offset.x+width/2, offset.y+height/2) as an actual point on the map
		//      After scaling, just translate this point back to screen co-ordinates and subtract
		//      width/2, height/2 to get the correct new offset. 
		//      For now, just "nudging" the value slightly.
		//double modAmtX = netViewCache.getImage().getWidth()>oldW ? 1.1 : 0.9;
		//double modAmtY = netViewCache.getImage().getHeight()>oldH ? 1.1 : 0.9;
		
		//Modify the offset accordingly
		//offset.x = (int)((modAmtX*offset.x*netViewCache.getImage().getWidth())/oldW);
		//offset.y = (int)((modAmtY*offset.y*netViewCache.getImage().getHeight())/oldH);
		
		repaint();
	}
	
	
	public void zoomFitSquare() {
		netViewCache.squareZoom(getCurrFrameTick(), new Point(getWidth(), getHeight()));
		repaint();
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

