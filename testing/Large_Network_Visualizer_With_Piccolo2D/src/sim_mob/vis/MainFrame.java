package sim_mob.vis;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.text.html.HTMLDocument.Iterator;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import sim_mob.conf.CSS_Interface;

import sim_mob.vis.controls.*;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.StringSetter;
import sim_mob.vis.util.Utility;


public class MainFrame extends JFrame {
	public static final long serialVersionUID = 1L;
		
	private static final String clockRateList[] = {"default-50ms","10 ms", "50 ms", "100 ms", "200 ms","500 ms","1000 ms"};
	
	private static final long MEGABYTE = 1024L * 1024L;

	public static long bytesToMegabytes(long bytes) {
		return bytes / MEGABYTE;
	}
	//Canvas that make use of the PCanvas
	private NetworkPanel virtual_newViewPnl;
	private SimulationResults simData;
	
	private JTextField console;
	
	//LHS panel
	private JButton openLogFile;
	private JButton openEmbeddedFile;
	private JButton zoomIn;
    private JComboBox clockRateComboBox;

	//Lower panel
	private Timer animTimer;
	private JSlider frameTickSlider;
	private JButton fwdBtn;
	private JButton playBtn;
	private JButton revBtn;
	private ImageIcon playIcon;
	private ImageIcon pauseIcon;
	
	//Colors
	public static CSS_Interface Config;

	
	public MainFrame(CSS_Interface config) {
		//Initial setup: FRAME
		super("Sim Mobility Visualization");

		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		MainFrame.Config = config;

		//Initial setup: FRAME AND APPLET
		this.setSize(1024, 768);	
		//Components and layout
		try {
			loadComponents();
			createListeners();
			addComponents(this.getContentPane());
		} catch (Exception ex) {
			throw new RuntimeException(ex);
		}
	
	}
	
	/**
	 * Load all components and initialize them properly.
	 */
	private void loadComponents() throws IOException {

		playIcon = new ImageIcon(Utility.LoadImgResource("res/icons/play.png"));
		pauseIcon = new ImageIcon(Utility.LoadImgResource("res/icons/pause.png"));
		
		
		console = new JTextField();
		openLogFile = new JButton("Open Logfile", new ImageIcon(Utility.LoadImgResource("res/icons/open.png")));
		openEmbeddedFile = new JButton("Open Default", new ImageIcon(Utility.LoadImgResource("res/icons/embed.png")));
	    clockRateComboBox = new JComboBox(clockRateList);

		frameTickSlider = new JSlider(JSlider.HORIZONTAL);
		revBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/rev.png")));
		playBtn = new JButton(playIcon);
		fwdBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/fwd.png")));

		virtual_newViewPnl = new NetworkPanel(300,300);

	}
	
	/**
	 * Add all components to the frame.
	 */
	private void addComponents(Container cp) {
		//Left panel
		GridLayout gl = new GridLayout(0,1,0,2);
		JPanel jpLeft = new JPanel(gl);		
		jpLeft.add(openLogFile);
		jpLeft.add(openEmbeddedFile);
		jpLeft.add(clockRateComboBox);

		
		//Bottom panel
		JPanel jpLower = new JPanel(new BorderLayout());
		jpLower.add(BorderLayout.NORTH, frameTickSlider);
		JPanel jpLower2 = new JPanel(new GridLayout(1, 0, 10, 0));
		jpLower2.add(revBtn);
		jpLower2.add(playBtn);
		jpLower2.add(fwdBtn);
		jpLower.add(BorderLayout.CENTER, jpLower2);
		
		
		//Main Frame uses a grid bag layout
		GridBagConstraints gbc;
		cp.setLayout(new GridBagLayout());
		
		//Add to the main controller: left panel
		gbc = new GridBagConstraints();
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.gridwidth = 1;
		gbc.gridheight = GridBagConstraints.REMAINDER;
		gbc.weightx = 0.1;
		gbc.weighty = 0.1;
		gbc.anchor = GridBagConstraints.PAGE_START;
		gbc.insets = new Insets(10, 5, 10, 5);
		cp.add(jpLeft, gbc);
		
		//Add (and stretch) the console
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 0;
		gbc.weightx = 0.1;
		gbc.weighty = 0.1;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		cp.add(console, gbc);
		
		//Add to the main controller: center panel
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 1;
		gbc.ipadx = 800;
		gbc.ipady = 600;
		gbc.weightx = 0.9;
		gbc.weighty = 0.9;
		gbc.fill = GridBagConstraints.BOTH;
		//cp.add(newViewPnl, gbc);
		cp.add(virtual_newViewPnl, gbc);
		

		//Add to the main controller: right panel
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 2;
		gbc.weightx = 0.1;
		gbc.weighty = 0.1;
		gbc.anchor = GridBagConstraints.PAGE_END;
		cp.add(jpLower, gbc);

	
	}
	
	/**
	 * Create all Listeners and hook them up to callback functions.
	 */
	private void createListeners() {

		openEmbeddedFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				openAFile(true);
			}
		});
		
		openLogFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				openAFile(false);
			}
		});
		

		
		//Frame tick slider
		frameTickSlider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				if (simData==null) {
					return;
				}
				if (frameTickSlider.isEnabled()) {
					virtual_newViewPnl.jumpAnim(frameTickSlider.getValue(), frameTickSlider);
				}
			}
		});
		
		//Play/pause
		playBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//Anything to play?
				if (simData==null) {
					return;
				}
				
				if (!animTimer.isRunning()) {
					animTimer.start();
					playBtn.setIcon(pauseIcon);
				} else {
					animTimer.stop();
					playBtn.setIcon(playIcon);
				}
			}
		});
		
		fwdBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (virtual_newViewPnl.advanceAnimbyStep(1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());

					return;
				}
				
			}
			
		});
		
		revBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (virtual_newViewPnl.advanceAnimbyStep(-1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());
					return;
				}
				
			}
			
		});
		
		animTimer = new Timer(50, new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {

				if (virtual_newViewPnl.advanceAnim(1, frameTickSlider)) {
					console.setText("Input File Name: "+frameTickSlider.getValue());
				
				}else{
					animTimer.stop();
					playBtn.setIcon(playIcon);
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
							
							if (virtual_newViewPnl.advanceAnim(1, frameTickSlider)) {
								console.setText("Input File Name: "+frameTickSlider.getValue());
							
							}else{
								animTimer.stop();
								playBtn.setIcon(playIcon);
								return;
									
							}
						}
					});
					animTimer.start();
				}
			}
		});
		/*
		animTimer = new Timer(50, new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//if (!newViewPnl.advanceAnim(1, frameTickSlider)) {
					if(!virtual_newViewPnl.testTimer())
					{	
						animTimer.stop();
						playBtn.setIcon(playIcon);
						return;
					}
					
				//}
			}
		});
		*/
	}
	
	
	private void openAFile(boolean isEmbedded) {
		
		//Use a FileChooser
		File f = null;
		if (!isEmbedded) {
			final JFileChooser fc = new JFileChooser("src/res/data");
			if (fc.showOpenDialog(MainFrame.this)!=JFileChooser.APPROVE_OPTION) {
				return;
			}
			f = fc.getSelectedFile();
		}

		//Load the default visualization
		RoadNetwork rn = null;
		String fileName;
		try {
			BufferedReader br = null;
			long fileSize = 0;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
				fileName = "default.log";
			} else {
				br = new BufferedReader(new FileReader(f));
				fileSize = f.length();
				fileName = f.getName();
			}
 
			rn = new RoadNetwork(br,virtual_newViewPnl.getWidth(), virtual_newViewPnl.getHeight());
			br.close();
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}

		//Store all Agents returned by this.
		HashSet<Integer> uniqueAgentIDs = new HashSet<Integer>();
		
		//Load the simulation's results
		try {
			BufferedReader br = null;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
			} else {
				br = new BufferedReader(new FileReader(f));
			}
			simData = new SimulationResults(br, rn, uniqueAgentIDs);
			br.close();
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}		
		
		
		//Update the slider
		frameTickSlider.setMinimum(0);
		frameTickSlider.setMaximum(simData.ticks.size()-1);
		frameTickSlider.setMajorTickSpacing(simData.ticks.size()/10);
		frameTickSlider.setMinorTickSpacing(simData.ticks.size()/50);
		frameTickSlider.setValue(0);
		
		console.setText("Input File Name: "+frameTickSlider.getValue());

		//Clear canvas
		//virtual_newViewPnl.getCamera().removeAllChildren();
	
		//virtual_newViewPnl.getCamera().repaint();		
			
		//System.out.println(virtual_newViewPnl.);
		
		NetworkVisualizer vis = new NetworkVisualizer(virtual_newViewPnl.getWidth(), virtual_newViewPnl.getHeight());
				
		vis.setVis(rn, simData, uniqueAgentIDs);
		
		virtual_newViewPnl.iniMapCache(vis);
		//virtual_newViewPnl.drawMap(rn, simData);
		
		// Get the Java runtime
		Runtime runtime = Runtime.getRuntime();
		// Run the garbage collector
		//runtime.gc();
		// Calculate the used memory
		long memory = runtime.totalMemory() - runtime.freeMemory();
		System.out.println("Used memory is bytes: " + memory);
		System.out.println("Used memory is megabytes: "
				+ bytesToMegabytes(memory));
		
	}
	
}


