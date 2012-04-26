package sim_mob.vis;


import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;
import edu.umd.cs.piccolo.PCamera;
import edu.umd.cs.piccolo.util.*;
import java.text.DecimalFormat;

import sim_mob.act.Activity;
import sim_mob.act.BifurcatedActivity;
import sim_mob.act.ManageLoadingFileActivity;
import sim_mob.conf.CSS_Interface;
import sim_mob.vis.controls.*;
import sim_mob.vis.simultion.SimulationResults;


public class MainFrame extends MainFrameUI {
	private static final long serialVersionUID = 1L;
	
	//For controlling the simulation.
	private NetSimAnimator netViewAnimator;
	private SimulationResults simData;
	
	private Timer memoryUsageTimer;
	private FileOpenThread progressData;
	private Timer progressChecker;
	protected Timer animTimer;
	
	//Our current loading activity
	BifurcatedActivity loadingAFile;
	
	//Colors
	public static CSS_Interface Config = new CSS_Interface(); //An empty config allows us to use "default"
	
	//For zooming
	//private static final Stroke onePtStroke = new BasicStroke(1.0F);
	private MouseAdapter currZoomer;
	
	public MainFrame(CSS_Interface config) {
		//Initial setup: FRAME
		super("Sim Mobility Visualization");
		MainFrame.Config = config;
			
		//Components and layout
		createListeners();
		
		//Start running the memory timer
		memoryUsageTimer.start();
	}
	
	
	@Override
	protected void loadComponents() {
		super.loadComponents();
		
		memoryUsage.setText("");
		generalProgress.setVisible(false);
	}
	

	
	/**
	 * Create all Listeners and hook them up to callback functions.
	 */
	private void createListeners() {
		openEmbeddedFile.addActionListener(new OpenFileListener(true));		
		openLogFile.addActionListener(new OpenFileListener(false));
		
		zoomIn.addActionListener(new ZoomMapListener(1.1));
		zoomOut.addActionListener(new ZoomMapListener(0.9));
		zoomSquare.addActionListener(new ZoomSquareListener());
		
		//Frame tick slider
		frameTickSlider.addChangeListener(new FrameTickChangeListener());
		
		//Play/pause
		playBtn.addActionListener(new PlayPauseListener());
		fwdBtn.addActionListener(new FwdRevListener(true));
		revBtn.addActionListener(new FwdRevListener(false));

		memoryUsageTimer = new Timer(1000, new MemoryUsageAction()); 
		animTimer = new Timer(50, new AnimTimerAction());
				
		clockRateComboBox.addActionListener(new ClockRateListener());
	}
	
	
	//Check our loading progress. Respond if done
	private void checkProgress() {
		//First, are we done?
		simData = progressData.getResults();
		if (progressData.isDone()) {
			loadingAFile.end();
		} else {
			//If not done, just update our progress.
			generalProgress.setValue((int)(progressData.getPercentDone() * generalProgress.getMaximum()));
		}
	}
	
	
	
	/**
	 * Request this frame to stop animating the current simulation. 
	 */
	public void pauseAnimation() {
		if (animTimer.isRunning()) {
			animTimer.stop();
			playBtn.setSelected(false);
		}
	}
	
	private final Activity releaseZoomActivity = new Activity() {
		public Object run(Object... args) {
			releaseZoomSquare();
			return null;
		}
	};
	private void releaseZoomSquare() {
		if (zoomSquare.isSelected()) {
			zoomSquare.setSelected(false);
		}
		netViewPanel.removeMouseListener(currZoomer);
		netViewPanel.removeMouseMotionListener(currZoomer);
		currZoomer = null;
		netViewPanel.setEnabled(true);
	}
	
	
	private void factorZoom(double amt) {
		//Test
		PCamera c = netViewPanel.getCamera();
		PBounds vb = c.getViewBounds();
		c.scaleViewAboutPoint(amt, vb.getCenterX(), vb.getCenterY());
	}
	
	public SimulationResults getSimulationResults() {
		return simData;
	}
	
	public void resetFrameTickSlider(int ticks) {
		frameTickSlider.setMinimum(0);
		frameTickSlider.setMaximum(ticks-1);
		frameTickSlider.setMajorTickSpacing(ticks/10);
		frameTickSlider.setMinorTickSpacing(ticks/50);
		frameTickSlider.setValue(0);
	}
	
	public Timer openAFile(boolean embedded) {
		progressData = new FileOpenThread(this, embedded);
		progressData.start();
		progressChecker = new Timer(200, new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				checkProgress();
			}
		});
		progressChecker.start();
		return progressChecker;
	}
	
	public void clearProgressData() {
		progressData = null;
		progressChecker = null;
	}
	
	public void rebuildSceneGraph() {
		netViewPanel.buildSceneGraph(progressData.getRoadNetwork(), simData, progressData.getUniqueAgentIDs());
		netViewAnimator = new NetSimAnimator(netViewPanel, simData, frameTickSlider);
	}

	
	class OpenFileListener implements ActionListener {
		private boolean embedded;
		OpenFileListener(boolean embedded) {
			this.embedded = embedded;
		}
		public void actionPerformed(ActionEvent arg0) {
			if (progressChecker==null) {
				loadingAFile = new ManageLoadingFileActivity(MainFrame.this, netViewPanel, generalProgress, embedded);
				loadingAFile.begin();
			}
		}
	}
	
	class ZoomMapListener implements ActionListener {
		private double zoomFactor;
		ZoomMapListener(double zoomFactor) {
			this.zoomFactor = zoomFactor;
		}
		public void actionPerformed(ActionEvent e) {
			factorZoom(zoomFactor);
		}
	}
	
	class ZoomSquareListener implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (zoomSquare.isSelected()) {
				//Start zoom-select
				netViewPanel.setEnabled(false);
				currZoomer = new MouseRectZoomer(MainFrame.this, netViewPanel, releaseZoomActivity);
				netViewPanel.addMouseListener(currZoomer);
				netViewPanel.addMouseMotionListener(currZoomer);
			} else {
				//Cancel
				releaseZoomSquare();
			}
		}
	}

	
	class FrameTickChangeListener implements ChangeListener {
		public void stateChanged(ChangeEvent arg0) {
			if (simData==null || netViewAnimator==null) {
				return;
			}
			if (frameTickSlider.isEnabled()) {
				netViewAnimator.jumpAnim(frameTickSlider.getValue(), frameTickSlider);
			}
		}
	}

	
	class PlayPauseListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			//Anything to play?
			if (simData==null || netViewAnimator==null) {
				return;
			}
			
			if (!animTimer.isRunning()) {
				animTimer.start();
				playBtn.setSelected(true);
				//playBtn.setIcon(MainFrameUI.pauseIcon);
			} else {
				animTimer.stop();
				playBtn.setSelected(false);
				//playBtn.setIcon(MainFrameUI.playIcon);
			}
		}
	}
	
	class FwdRevListener implements ActionListener {
		private int amtFwd;
		FwdRevListener(boolean fwd) {
			this.amtFwd = fwd ? 1 : -1;
		}
		public void actionPerformed(ActionEvent arg0) {
			if (netViewAnimator.advanceAnimbyStep(amtFwd, frameTickSlider)) {
				animTimer.stop();
				playBtn.setSelected(false);
				
				//NOTE: Why is this here? It seems completely out of place. ~Seth
				console.setText("Input File Name: "+frameTickSlider.getValue());
			}
		}
	}
	
	class MemoryUsageAction implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			Runtime runtime = Runtime.getRuntime();
			long memory = runtime.totalMemory() - runtime.freeMemory();
			String memTxt = new DecimalFormat("#.#").format(memory / Math.pow(1024, 2)); //e.g. "1.2"
			memoryUsage.setText("Memory: " + memTxt + " Mb");
		}
	}

	class AnimTimerAction implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {

			if (netViewAnimator.advanceAnim(1, frameTickSlider)) {
				console.setText("Input File Name: "+frameTickSlider.getValue());
			
			}else{
				animTimer.stop();
				playBtn.setSelected(false);
				//playBtn.setIcon(MainFrameUI.playIcon);
				return;
					
			}
		}
	}

	class ClockRateListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			
			String speed = (String) clockRateComboBox.getSelectedItem();
			if(!speed.contains("default")){
				String [] items = speed.split(" ");
				
				int clockRate = Integer.parseInt(items[0]);
				
				animTimer.stop();
				
				animTimer = new Timer(clockRate, new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						
						if (netViewAnimator.advanceAnim(1, frameTickSlider)) {
							console.setText("Input File Name: "+frameTickSlider.getValue());
						}else{
							animTimer.stop();
							playBtn.setSelected(false);
							//playBtn.setIcon(MainFrameUI.playIcon);
							return;
								
						}
					}
				});
				animTimer.start();
			}
		}
	}
	
	
}


