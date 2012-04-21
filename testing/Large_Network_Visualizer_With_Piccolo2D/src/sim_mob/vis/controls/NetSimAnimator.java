package sim_mob.vis.controls;

import javax.swing.JSlider;

import sim_mob.vis.simultion.SimulationResults;


/**
 * This class is used to drive the animation of the simulator. 
 * Whereas the NetworkVisualizer maintains all information regarding the 
 * current road network and agent positions, this class holds on to the current
 * time tick. 
 * 
 * \author Seth N. Hetu
 */
public class NetSimAnimator {
	private NetworkVisualizer currVis;
	private SimulationResults simRes;
	private int currFrameNum;

	
	public NetSimAnimator(NetworkVisualizer currVis, SimulationResults simRes, JSlider slider) {
		this.currVis = currVis;
		this.simRes = simRes;
		this.currFrameNum = 0;
		
		//Probably a good idea:
		if (!simRes.ticks.isEmpty()) {
			jumpAnim(currFrameNum, slider);
		}
	}
	
	public int getMaxFrameTick() { 
		return simRes.ticks.size()-1; 
	}
	
	public boolean jumpAnim(int toTick, JSlider slider){
		//Set
		if (simRes==null || !setCurrFrameTick(toTick)) {
			return false;
		}
		//Update the slider, if it exists
		if (slider!=null) {
			slider.setEnabled(false);
			slider.setValue(getCurrFrameTick());
			slider.setEnabled(true);
		}

		currVis.redrawAtCurrFrame(simRes.ticks.get(getCurrFrameTick()));
		return true;
	}
	
	public boolean setCurrFrameTick(int newVal) {
		if (newVal<0 || simRes==null || newVal>=getMaxFrameTick()) {
			return false;
		}
		currFrameNum = newVal;
		return true;
	}
	public int getCurrFrameTick() {
		return currFrameNum;
	}
	
	
	public boolean advanceAnim(int ticks, JSlider slider) {
		//Increment
		if (simRes==null || !incrementCurrFrameTick(1)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}
	
	public boolean advanceAnimbyStep(int ticks, JSlider slider) {
		//Increment
		if (simRes==null || !incrementCurrFrameTick(ticks)) {
			return false;
		}
		
		return jumpAnim(getCurrFrameTick(), slider);
	}

	public boolean incrementCurrFrameTick(int amt) {
		return setCurrFrameTick(getCurrFrameTick()+amt);
	}
	
	
}
