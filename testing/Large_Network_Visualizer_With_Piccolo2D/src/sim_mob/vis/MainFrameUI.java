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

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.Timer;

import sim_mob.conf.BatikCSS_Loader;
import sim_mob.conf.CSS_Interface;
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
	
	private static final String clockRateList[] = {"default-50ms","10 ms", "50 ms", "100 ms", "200 ms","500 ms","1000 ms"};
	
	//Our network panel is placed within a FlowLayout'd JPanel to make swapping 
	// items out easier later.
	protected JPanel rhsLayout;
	protected JProgressBar generalProgress;
	protected FileOpenThread progressData;
	protected Timer progressChecker;
	
	protected JTextField console;
	
	//Canvas that make use of the PCanvas
	protected NetworkVisualizer netViewPanel;
	
	//LHS panel
	protected Timer memoryUsageTimer;
	protected JLabel memoryUsage;
	protected JButton openLogFile;
	protected JButton openEmbeddedFile;
	protected JToggleButton zoomSquare;
	protected JButton zoomIn;
	protected JButton zoomOut;
	protected JComboBox clockRateComboBox;

	//Lower panel
	protected Timer animTimer;
	protected JSlider frameTickSlider;
	protected JButton fwdBtn;
	protected JButton playBtn;
	protected JButton revBtn;
	protected ImageIcon playIcon;
	protected ImageIcon pauseIcon;
	
	public MainFrameUI(String title) {
		super(title);
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		this.setSize(1024, 768);
		
		loadComponents();
		layoutComponents(this.getContentPane());
	}
	
	protected void loadComponents() {
		//TODO: These can go into a "load resources" function; we might want to allow optionally 
		//      running with a warning if some icons are missing, or having a fallback (e.g., "+" instead of zoom in)
		try {
			playIcon = new ImageIcon(Utility.LoadImgResource("res/icons/play.png"));
			pauseIcon = new ImageIcon(Utility.LoadImgResource("res/icons/pause.png"));
			openLogFile = new JButton("Open Logfile", new ImageIcon(Utility.LoadImgResource("res/icons/open.png")));
			openEmbeddedFile = new JButton("Open Default", new ImageIcon(Utility.LoadImgResource("res/icons/embed.png")));
			revBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/rev.png")));
			fwdBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/fwd.png")));
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		console = new JTextField();
		memoryUsage = new JLabel("Memory Usage: X mb");
	    clockRateComboBox = new JComboBox(clockRateList);
	    zoomSquare = new JToggleButton("Zoom Box");
	    zoomIn = new JButton("+");
	    zoomOut = new JButton("-");
	    

		frameTickSlider = new JSlider(JSlider.HORIZONTAL);
		playBtn = new JButton(playIcon);

		netViewPanel = new NetworkVisualizer(300,300);
		generalProgress = new JProgressBar();
		generalProgress.setMinimum(0);
		generalProgress.setMaximum(100);
		generalProgress.setValue(45);
		generalProgress.setForeground(new Color(0x33, 0x99, 0xEE));
		generalProgress.setStringPainted(true);
	}
	
	private void layoutComponents(Container cp) {
		//Left panel
		GridLayout gl = new GridLayout(0,1,0,2);
		JPanel jpLeft = new JPanel(gl);
		jpLeft.add(memoryUsage);
		jpLeft.add(openLogFile);
		jpLeft.add(openEmbeddedFile);
		jpLeft.add(clockRateComboBox);
		
		JPanel zoomPnl = new JPanel(new FlowLayout());
		zoomPnl.add(zoomSquare);
		zoomPnl.add(zoomIn);
		zoomPnl.add(zoomOut);
		jpLeft.add(zoomPnl);

		
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
