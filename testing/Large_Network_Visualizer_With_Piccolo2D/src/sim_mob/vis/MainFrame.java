package sim_mob.vis;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;

import javax.swing.*;
import javax.swing.event.*;
import edu.umd.cs.piccolo.PCamera;
import edu.umd.cs.piccolo.util.*;
import java.io.*;
import java.text.DecimalFormat;
import java.util.HashSet;

import sim_mob.act.Activity;
import sim_mob.act.BifurcatedActivity;
import sim_mob.act.ManageLoadingFileActivity;
import sim_mob.conf.CSS_Interface;
import sim_mob.vis.controls.*;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.Utility;


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
	private static final Stroke onePtStroke = new BasicStroke(1.0F);
	private MouseAdapter currZoomer;
	
	public MainFrame(CSS_Interface config) {
		//Initial setup: FRAME
		super("Sim Mobility Visualization");
		MainFrame.Config = config;
			
		//Components and layout
		createListeners();
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

		openEmbeddedFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (progressChecker==null) {
					loadingAFile = new ManageLoadingFileActivity(MainFrame.this, netViewPanel, generalProgress, true);
					loadingAFile.begin();
				}
			}
		});
		
		openLogFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (progressChecker==null) {
					loadingAFile = new ManageLoadingFileActivity(MainFrame.this, netViewPanel, generalProgress, false);
					loadingAFile.begin();
				}
			}
		});
		
		zoomIn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				factorZoom(1.1);
			}
		});
		zoomOut.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				factorZoom(0.9);
			}
		});
		
		zoomSquare.addActionListener(new ActionListener() {
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
		});
		
		

		
		//Frame tick slider
		frameTickSlider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				if (simData==null || netViewAnimator==null) {
					return;
				}
				if (frameTickSlider.isEnabled()) {
					netViewAnimator.jumpAnim(frameTickSlider.getValue(), frameTickSlider);
				}
			}
		});
		
		//Play/pause
		playBtn.addActionListener(new ActionListener() {
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
		});
		
		fwdBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (netViewAnimator.advanceAnimbyStep(1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setSelected(false);
					//playBtn.setIcon(MainFrameUI.playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());

					return;
				}
				
			}
			
		});
		
		revBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (netViewAnimator.advanceAnimbyStep(-1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setSelected(false);
					//playBtn.setIcon(MainFrameUI.playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());
					return;
				}
				
			}
			
		});
		
		memoryUsageTimer = new Timer(1000, new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				Runtime runtime = Runtime.getRuntime();
				long memory = runtime.totalMemory() - runtime.freeMemory();
				String memTxt = new DecimalFormat("#.#").format(memory / Math.pow(1024, 2)); //e.g. "1.2"
				memoryUsage.setText("Memory: " + memTxt + " Mb");
			}
		});
		memoryUsageTimer.start();
		
		animTimer = new Timer(50, new ActionListener() {
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
		
		clockRateComboBox.addActionListener(new ActionListener() {
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
		});
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

	
	
}


