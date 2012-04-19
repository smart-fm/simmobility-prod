package sim_mob.vis.controls;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.util.Hashtable;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JSlider;

import edu.umd.cs.piccolo.PCanvas;
import edu.umd.cs.piccolo.PLayer;
import edu.umd.cs.piccolo.PNode;
import edu.umd.cs.piccolo.activities.PActivity;
import edu.umd.cs.piccolo.event.PBasicInputEventHandler;
import edu.umd.cs.piccolo.event.PInputEvent;
import edu.umd.cs.piccolo.nodes.PImage;
import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

import sim_mob.vis.MainFrame;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.Crossing;
import sim_mob.vis.network.LaneMarking;
import sim_mob.vis.network.Link;
import sim_mob.vis.network.Node;
import sim_mob.vis.network.Segment;
import sim_mob.vis.simultion.AgentTick;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.simultion.TimeTick;
import sim_mob.vis.util.StringSetter;

public class NetworkPanel extends PCanvas{

    private NetworkVisualizer netViewCache;

	private RoadNetwork network;
	private SimulationResults simRes;
	private PLayer layer;

	
	private int currFrameTick;

	public NetworkPanel(int width, int height) {
    	setPreferredSize(new Dimension(width, height));
    	this.layer = this.getLayer();
 
    	this.setBackground(MainFrame.Config.getBackground("panel"));
    	
	}
	
	public void iniMapCache(NetworkVisualizer nv){
		this.netViewCache = nv;
		this.getCamera().addLayer(nv.getMyLayer());
		this.currFrameTick = 0;
		
	}
	
	public boolean incrementCurrFrameTick(int amt) {
		return setCurrFrameTick(currFrameTick+amt);
	}
	public boolean jumpAnim(int toTick,JSlider slider){
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

		netViewCache.redrawAtCurrFrame(getCurrFrameTick());
		
		return true;
		
	}
	
	public boolean advanceAnim(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !incrementCurrFrameTick(1)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public boolean advanceAnimbyStep(int ticks, JSlider slider) {
		//Increment
		if (netViewCache==null || !incrementCurrFrameTick(ticks)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public boolean setCurrFrameTick(int newVal) {
		if (newVal<0 || netViewCache==null || newVal>=netViewCache.getMaxFrameTick()) {
			return false;
		}
		currFrameTick = newVal;
		return true;
	}
	
	public int getCurrFrameTick() {
		return currFrameTick;
	}
	
	public int getMaxFrameTick() {
		return netViewCache.getMaxFrameTick();
	}
	
}



