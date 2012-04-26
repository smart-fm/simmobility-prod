package sim_mob.act;

import java.awt.geom.Rectangle2D;

import javax.swing.JProgressBar;
import javax.swing.Timer;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.NetworkVisualizer;
import sim_mob.vis.simultion.SimulationResults;


/**
 * Handles all of the necessary GUI adjustments for beginning to open a file and then 
 * completing/canceling that file load.
 * 
 * \author Seth N. Hetu
 */
public class ManageLoadingFileActivity extends BifurcatedActivity {
	//The frame which is loading this file, and some associated components.
	private MainFrame parentFrame;
	private NetworkVisualizer netView;
	private JProgressBar progress;
	private Timer progressCheckTimer;
	
	//Is this an embedded file? Behavior changes slightly in that case.
	private boolean embedded;
	
	
	public ManageLoadingFileActivity(MainFrame parentFrame, NetworkVisualizer netView, JProgressBar progress, boolean embedded) {
		this.parentFrame = parentFrame;
		this.netView = netView;
		this.progress = progress;
		this.embedded = embedded;
	}
	
	
	public Object begin(Object... args) {
		parentFrame.pauseAnimation();
		progress.setValue(0);
		progress.setVisible(true);
		progress.setIndeterminate(embedded);
		progress.setStringPainted(!embedded);
		progress.requestFocusInWindow();
		netView.setVisible(false);
		
		progressCheckTimer = parentFrame.openAFile(embedded);
		return null;
	}
	
	
	public Object end(Object... args) {
		//Stop our timer.
		progressCheckTimer.stop();
		
		//Hide the progress bar
		progress.setVisible(false);
		
		//We might be done but not actually have a simulation to work with (cancelled dialog)
		SimulationResults simData = parentFrame.getSimulationResults();
		if (simData!=null) {
			//Update the slider
			parentFrame.resetFrameTickSlider(simData.ticks.size());
			
			//TODO
			//console.setText("Input File Name: "+frameTickSlider.getValue());
			
			//Remove all children (if reloading)
			netView.getLayer().removeAllChildren();
			
			//Now add all children again
			parentFrame.rebuildSceneGraph();
			
			//Reset the view
			Rectangle2D initialBounds = netView.getNaturalBounds();
			netView.getCamera().animateViewToCenterBounds(initialBounds, true, 1000);
		}
		
		//Show the network panel again.
		netView.setVisible(true);
		
		//Done, set to null
		parentFrame.clearProgressData();
		
		return null;
	}

}
