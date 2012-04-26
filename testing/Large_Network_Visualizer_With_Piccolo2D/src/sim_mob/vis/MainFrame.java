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


/**
 * The main frame of our simulation visualizer. This extends the MainFrameUI class
 * and connects all controls with their respective Action Listeners. 
 * 
 * \author Seth N. Hetu
 */
public class MainFrame extends MainFrameUI {
	private static final long serialVersionUID = 1L;
	
	//Colors and strokes
	//An empty config allows us to use "default" successfully.
	public static CSS_Interface Config = new CSS_Interface();
	
	//Our current simulation.
	private SimulationResults simData;
	
	//Related to opening the simulation file.
	private FileOpenThread progressData;
	BifurcatedActivity loadingAFile;
	
	//For controlling the visualization's animation.
	private NetSimAnimator netViewAnimator;
	
	//Various Swing timers.
	private Timer memoryUsageTimer;
	private Timer progressChecker;
	protected Timer animTimer;

	//For zooming
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
	
	
	/**
	 * Check our loading progress. Respond if done. 
	 */
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
	
	
	
	/////////////////////////////////////////////////////////////////////////////
	// Many of the following functions were added to allow MouseRectZoomer and FileOpenThread
	// to be moved into separate files. Basically, MainFrame used to be a single, gigantic class,
	// so these functions became needlessly interconnected. As more functionality is migrated
	// OUT of MainFrame, some of these functions will disappear. For example, the play/fwd/rev
	// buttons should be moved into their own "simulation playback" interface, so "resetFrameTickSlider()"
	// will no longer be needed (or will become part of that class instead".
	/////////////////////////////////////////////////////////////////////////////
	
	
	//Convenient activity wrapper for releaseZoomSquare()
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
	
	
	//Zoom by a given factor. 
	private void factorZoom(double amt) {
		PCamera c = netViewPanel.getCamera();
		PBounds vb = c.getViewBounds();
		c.scaleViewAboutPoint(amt, vb.getCenterX(), vb.getCenterY());
	}
	
	public SimulationResults getSimulationResults() {
		return simData;
	}
	
	
	///Reset the frame tick slider to range from 0...ticks animation ticks, and set it to 0.
	public void resetFrameTickSlider(int ticks) {
		frameTickSlider.setMinimum(0);
		frameTickSlider.setMaximum(ticks-1);
		frameTickSlider.setMajorTickSpacing(ticks/10);
		frameTickSlider.setMinorTickSpacing(ticks/50);
		frameTickSlider.setValue(0);
	}
	
	///Start a thread which will open the simulation file.
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
	
	///Nullify the progress checker and the thread used to open the simulation file.
	///Calling this function when the thread is still working may be dangerous.
	public void clearProgressData() {
		progressData = null;
		progressChecker = null;
	}
	
	///Reload the network visualizer's scene graph from the current Simulation Results object.
	public void rebuildSceneGraph() {
		netViewPanel.buildSceneGraph(progressData.getRoadNetwork(), simData, progressData.getUniqueAgentIDs());
		netViewAnimator = new NetSimAnimator(netViewPanel, simData, frameTickSlider);
	}

	
	
	/////////////////////////////////////////////////////////////////////////////
	// The following classes are used as Listeners to respond to the GUI's various
	// button presses, mouse clicks, and combo box selections. Most of them should be
	// extracted into their own generic classes (and files). In addition, some of them
	// are very heavy-handed (e.g., trying to keep the play button's icon in sync
	// could be done much more easily with an MVC-style approach) and should be simplified.
	/////////////////////////////////////////////////////////////////////////////
	
	
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


