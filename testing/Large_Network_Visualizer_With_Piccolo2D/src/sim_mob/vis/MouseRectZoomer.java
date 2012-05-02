package sim_mob.vis;

import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Rectangle2D;

import sim_mob.act.Activity;
import sim_mob.vis.controls.NetworkVisualizer;


/**
 * A simple class which helps us scroll our network visualizer.
 * 
 * \author Seth N. Hetu
 */
public class MouseRectZoomer extends MouseAdapter {
	//private MainFrame parentFrame;
	private NetworkVisualizer netView;
	private Activity releaseZoomActivity;
	
	private Point startPoint;
	
	public MouseRectZoomer(MainFrame parentFrame, NetworkVisualizer netView, Activity releaseZoomActivity) {
		//this.parentFrame = parentFrame;
		this.netView = netView;
		this.releaseZoomActivity = releaseZoomActivity;
	}
	
	public void mousePressed(MouseEvent e) {
		startPoint = e.getPoint();
	}
	
	public void mouseExited(MouseEvent e) {
		//startPoint = null;  
	}
	
	public void mouseReleased(MouseEvent e) {
		if (startPoint!=null) {
			netView.setZoomBox(new Rectangle2D.Double(startPoint.x, startPoint.y, e.getX()-startPoint.x, e.getY()-startPoint.y));
			netView.zoomToBox();
		}
		netView.setZoomBox(null);
		releaseZoomActivity.run();
	}
	public void mouseDragged(MouseEvent e) {
		//Provide some feedback.
		if (startPoint != null) {
			netView.setZoomBox(new Rectangle2D.Double(startPoint.x, startPoint.y, e.getX()-startPoint.x, e.getY()-startPoint.y));
		}
	}
}
