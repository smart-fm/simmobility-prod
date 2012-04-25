package sim_mob.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.io.IOException;

import javax.swing.*;

import sim_mob.vis.controls.NetworkVisualizer;
import sim_mob.vis.util.Utility;


/**
 * Contains all user interface elements in the main visualizer. 
 * 
 * In an effort to separate layout of components from their interconnectedness and
 * runtime behavior, this class will contain all "named" components and their various layouts. 
 * It also contains a "main" method that simply loads the form "as-is", which is useful for debugging
 * layouts. Any "invisible" components should not be hidden in this class, and any text labels, etc.,
 * should have some sample text filled in.
 * 
 * \todo: We can remove a lot of scaffolding if we switch to jgoodies.FormLayout. 
 * 
 * \author Seth N. Hetu
 */
public class MainFrameUI  extends JFrame {
	private static final long serialVersionUID = 1L;
	
	//NOTE: Is this doing what we want it to?
	private static final String clockRateList[] = {"default-50ms","10 ms", "50 ms", "100 ms", "200 ms","500 ms","1000 ms"};
	
	//Shared resources. Loaded once
	private static ImageIcon playIcon;
	private static ImageIcon pauseIcon;
	private static ImageIcon revIcon;
	private static ImageIcon fwdIcon;
	private static ImageIcon loadFileIcon;
	private static ImageIcon loadEmbeddedIcon;
	private static ImageIcon zoomInIcon;
	private static ImageIcon zoomOutIcon;
	private static boolean ResourcesLoaded = false;
	
	//Leftover layout panels. TODO: Remove these if possible:
	private JPanel rhsLayout;
	private JPanel jpLeft;
	private JPanel zoomPnl;
	private JPanel jpLower;
	private JPanel jpLower2;
	
	//Left panel buttons
	protected JLabel memoryUsage;
	protected JButton openLogFile;
	protected JButton openEmbeddedFile;
	protected JComboBox clockRateComboBox;
	
	//Left panel: zoom buttons
	protected JToggleButton zoomSquare;
	protected JButton zoomIn;
	protected JButton zoomOut;
	
	//Lower panel: play, pause, seek
	protected JSlider frameTickSlider;
	protected JButton fwdBtn;
	protected JToggleButton playBtn;
	protected JButton revBtn;
	
	//Right panel: display surface, zoomable interface.
	protected NetworkVisualizer netViewPanel;
	
	//Various feedback components
	protected JProgressBar generalProgress;
	protected JTextField console;
	
	

	public MainFrameUI(String title) {
		super(title);
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		this.setSize(1024, 768);
		
		if (!ResourcesLoaded) {
			LoadResources();
			ResourcesLoaded = true;
		}
		loadComponents();
		layoutComponents(this.getContentPane());
	}
	
	private static final ImageIcon MakeImageIcon(String resourcePath) {
		ImageIcon res = null;
		try {
			res = new ImageIcon(Utility.LoadImgResource(resourcePath));
		} catch (IOException ex) { 
			System.out.println("WARNING: Couldn't find resource: " + resourcePath); 
		}
		return res;
	}
	
	///After this function has been called, any resources which are null simply weren't found.
	private static final void LoadResources() {
		playIcon = MakeImageIcon("res/icons/play.png");
		pauseIcon = MakeImageIcon("res/icons/pause.png");
		revIcon = MakeImageIcon("res/icons/rev.png");
		fwdIcon = MakeImageIcon("res/icons/fwd.png");
		loadFileIcon = MakeImageIcon("res/icons/open.png");
		loadEmbeddedIcon = MakeImageIcon("res/icons/embed.png");
		zoomInIcon = MakeImageIcon("res/icons/zoom_in.png");
		zoomOutIcon = MakeImageIcon("res/icons/zoom_out.png");
	}
	
	
	protected void loadComponents() {
		//"Open" buttons
		openLogFile = new JButton("Open Logfile", loadFileIcon);
		openEmbeddedFile = new JButton("Open Default", loadEmbeddedIcon);
		
		//Feedback items
		memoryUsage = new JLabel("Memory Usage: X mb");
		console = new JTextField();
		
		//Progress bar for loading files/saving video.
		generalProgress = new JProgressBar();
		generalProgress.setMinimum(0);
		generalProgress.setMaximum(100);
		generalProgress.setValue(45);
		generalProgress.setForeground(new Color(0x33, 0x99, 0xEE));
		generalProgress.setStringPainted(true);
		
		//Buttons for controlling simulation playback.
		revBtn = new JButton(revIcon);
		fwdBtn = new JButton(fwdIcon);
		frameTickSlider = new JSlider(JSlider.HORIZONTAL);
		playBtn = new JToggleButton(playIcon);
		playBtn.setSelectedIcon(pauseIcon);
		
		//TODO: We'll probably have to subclass ToggleButton to get the behavior we want. 
		//This comes close:
		//playBtn.setBorderPainted(false);
		//playBtn.setFocusPainted(false);
		//playBtn.setBackground(new Color(0, true));
		
		//Changing the clock rate? Can we do this differently...?
		clockRateComboBox = new JComboBox(clockRateList);
		
		//Zoom buttons
	    zoomSquare = new JToggleButton("Zoom Box");
	    zoomIn = new JButton("");
	    if (zoomInIcon!=null) {
	    	zoomIn.setIcon(zoomInIcon);
	    } else {
	    	zoomIn.setText("+");
	    }
	    zoomOut = new JButton("");
	    if (zoomOutIcon!=null) {
	    	zoomOut.setIcon(zoomOutIcon);
	    } else {
	    	zoomOut.setText("-");
	    }
	    
	    //The main zoomable interface
		netViewPanel = new NetworkVisualizer(300,300);
	}
	
	private void layoutComponents(Container cp) {
		//Left panel
		GridLayout gl = new GridLayout(0,1,0,2);
		jpLeft = new JPanel(gl);
		jpLeft.add(memoryUsage);
		jpLeft.add(openLogFile);
		jpLeft.add(openEmbeddedFile);
		jpLeft.add(clockRateComboBox);
		
		zoomPnl = new JPanel(new FlowLayout());
		zoomPnl.add(zoomSquare);
		zoomPnl.add(zoomIn);
		zoomPnl.add(zoomOut);
		jpLeft.add(zoomPnl);

		
		//Bottom panel
		jpLower = new JPanel(new BorderLayout());
		jpLower.add(BorderLayout.NORTH, frameTickSlider);
		jpLower2 = new JPanel(new GridLayout(1, 0, 10, 0));
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
		//cp.add(netViewPanel, gbc);
		rhsLayout = new JPanel(new BorderLayout());
		rhsLayout.add(BorderLayout.CENTER, netViewPanel);
		rhsLayout.add(BorderLayout.SOUTH, generalProgress);
		cp.add(rhsLayout, gbc);
		
		

		//Add to the main controller: right panel
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 2;
		gbc.weightx = 0.1;
		gbc.weighty = 0.1;
		gbc.anchor = GridBagConstraints.PAGE_END;
		cp.add(jpLower, gbc);
	}
	
	public static void main(String[] args) {
		new MainFrameUI("Sim Mobility Visualizer - Testing Layout").setVisible(true);
	}
	

}
