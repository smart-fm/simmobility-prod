package sim_mob.vis;


import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.regex.Pattern;

import sim_mob.conf.CSS_Interface;
import sim_mob.vis.controls.*;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.StringSetter;
import sim_mob.vis.util.Utility;


public class MainFrame extends JFrame {
	public static final long serialVersionUID = 1L;
	//For JComboBox
	private static final String clockRateList[] = {"default-50ms","10 ms", "50 ms", "100 ms", "200 ms","500 ms","1000 ms"};
    	
	//Regex
	private static final String num = "([0-9]+)";
	public static final Pattern NUM_REGEX = Pattern.compile(num);
	
	//Center (main) panel
	private NetworkPanel newViewPnl;
	private SimulationResults simData;
	private JTextField console;
	
	//LHS panel
	private JButton openLogFile;
	private JButton openEmbeddedFile;
	private JButton showFakeAgent;
	private JButton zoomIn;
	private JButton	zoomOut;
	
    private JComboBox clockRateComboBox;

	
	//Lower panel
	private Timer animTimer;
	private JSlider frameTickSlider;
	private JButton revBtn;
	private JButton playBtn;
	private JButton fwdBtn;
	private ImageIcon playIcon;
	private ImageIcon pauseIcon;
	
	
	//Helper
	public static CSS_Interface Config;
	private boolean showFake;
	
		
	/**
	 * NOTE: Currently, I haven't found a good way to switch between MainFrame as a JFrame and MainFrame as an Applet.
	 *       Basically, it requires changing 4 lines of code.
	 */
	public MainFrame(CSS_Interface config) {
		//Initial setup: FRAME
		super("Sim Mobility Visualization");

		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		MainFrame.Config = config;
		//Initial setup: FRAME AND APPLET
		this.setSize(1024, 768);
		this.showFake = false;
		
		//Components and layout
		try {
			loadComponents();
			createListeners();
			addComponents(this.getContentPane());
			//pack(); //Layout's making this hard...
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
	    clockRateComboBox = new JComboBox(clockRateList);
	    
	    openLogFile = new JButton("Open File From...", new ImageIcon(Utility.LoadImgResource("res/icons/open copy.png")));
		openEmbeddedFile = new JButton("Open Default File", new ImageIcon(Utility.LoadImgResource("res/icons/embed copy.png")));
		showFakeAgent = new JButton("Show Fake Agent", new ImageIcon(Utility.LoadImgResource("res/icons/fake copy.png")));
		zoomIn = new JButton("       Zoom In 	        ", new ImageIcon(Utility.LoadImgResource("res/icons/zoom_in.png")));
		zoomOut = new JButton("      Zoom Out  	    ", new ImageIcon(Utility.LoadImgResource("res/icons/zoom_out.png")));
		
		
		frameTickSlider = new JSlider(JSlider.HORIZONTAL);
		revBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/rev.png")));
		playBtn = new JButton(playIcon);
		fwdBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/fwd.png")));
		
		newViewPnl = new NetworkPanel(new StringSetter() {
			public void set(String str) {
				//Update the status bar
				String oldValue = console.getText();
				if(oldValue.isEmpty())
				{
					console.setText(str);
				}else{
					console.setText(oldValue+"\t"+str);
				}
			}
		});
	}
	
	/**
	 * Add all components to the frame.
	 * @param cp Content Pane of the current Frame.
	 */
	private void addComponents(Container cp) {
		//Left panel
		GridLayout gl = new GridLayout(0,1,0,2);
		JPanel jpLeft = new JPanel(gl);
		jpLeft.add(openLogFile);
		jpLeft.add(openEmbeddedFile);
		jpLeft.add(clockRateComboBox);
		jpLeft.add(showFakeAgent);
		jpLeft.add(zoomIn);
		jpLeft.add(zoomOut);
	
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
		cp.add(newViewPnl, gbc);
		
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
		//Frame tick slider
		frameTickSlider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				if (simData==null) {
					return;
				}
				if (frameTickSlider.isEnabled()) {
					newViewPnl.jumpAnim(frameTickSlider.getValue(), frameTickSlider);
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
		
		//Forward and Backward Button
		fwdBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (newViewPnl.advanceAnimbyStep(1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					return;
				}
				
			}
			
		});

		revBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (newViewPnl.advanceAnimbyStep(-1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					return;
				}
				
			}
			
		});

		//20 FPS, update anim
		animTimer = new Timer(50, new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (!newViewPnl.advanceAnim(1, frameTickSlider)) {
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
							if (!newViewPnl.advanceAnim(1, frameTickSlider)) {
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
		
		showFakeAgent.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				
				if(showFake){
					newViewPnl.showFakeAgent(false);
					showFake = false;
					showFakeAgent.setText("Show Fake Agent");

				}else{
					newViewPnl.showFakeAgent(true);
					showFake = true;
					showFakeAgent.setText("Hide Fake Agent");
				}
			
			}
			
		});
		
		zoomIn.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				
			}
			
		});
		
		zoomOut.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				
			}
			
		});
		
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
		

	}
	
	private void openAFile(boolean isEmbedded) {
		//Pause the animation
		if (animTimer.isRunning()) {
			animTimer.stop();
			playBtn.setIcon(playIcon);
		}
		
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
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
				fileName = "default.log";
			} else {
				br = new BufferedReader(new FileReader(f));
				fileName = f.getName();
			}
 
			rn = new RoadNetwork(br);
			br.close();
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	
		console.setText("Input File Name: "+fileName);
		
		//Load the simulation's results
		try {
			BufferedReader br = null;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
			} else {
				br = new BufferedReader(new FileReader(f));
			}
			simData = new SimulationResults(br, rn);
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
		
		//Add a visualizer
		NetworkVisualizer vis = new NetworkVisualizer();
		vis.setSource(rn, simData, 1.0, newViewPnl.getWidth(), newViewPnl.getHeight(), fileName);
		
		//Update the map
		newViewPnl.drawMap(vis, 0, 0);
	}
	
}


