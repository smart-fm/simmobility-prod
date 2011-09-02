package sim_mob.vis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

import sim_mob.vis.controls.NetworkPanel;
import sim_mob.vis.controls.NetworkVisualizer;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.util.Utility;

public class MainFrame extends JFrame {
	public static final long serialVersionUID = 1L;
	
	//Center (main) panel
	private NetworkPanel newViewPnl;
	
	//LHS panel
	private JButton openLogFile;
	
	//Lower panel
	private JButton revBtn;
	private JButton playBtn;
	private JButton fwdBtn;

	
	public MainFrame() {
		//Initial setup
		super("Sim Mobility Visualization");
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		this.setSize(1024, 768);
		
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
		openLogFile = new JButton("Open Logfile", new ImageIcon(Utility.LoadImgResource("res/icons/open.png")));
		revBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/rev.png")));
		playBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/play.png")));
		fwdBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/fwd.png")));
		
		newViewPnl = new NetworkPanel();
	}
	
	/**
	 * Add all components to the frame.
	 * @param cp Content Pane of the current Frame.
	 */
	private void addComponents(Container cp) {
		//Left panel
		JPanel jpLeft = new JPanel(new GridLayout(0, 1, 0, 10));
		jpLeft.add(openLogFile);
		
		//Bottom panel
		JPanel jpLower = new JPanel(new GridLayout(1, 0, 10, 0));
		jpLower.add(revBtn);
		jpLower.add(playBtn);
		jpLower.add(fwdBtn);
		
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
		
		//Add to the main controller: center panel
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 0;
		gbc.ipadx = 800;
		gbc.ipady = 600;
		gbc.weightx = 0.9;
		gbc.weighty = 0.9;
		gbc.fill = GridBagConstraints.BOTH;
		cp.add(newViewPnl, gbc);
		
		//Add to the main controller: right panel
		gbc = new GridBagConstraints();
		gbc.gridx = 1;
		gbc.gridy = 1;
		gbc.weightx = 0.1;
		gbc.weighty = 0.1;
		gbc.anchor = GridBagConstraints.PAGE_END;
		cp.add(jpLower, gbc);
	}
	
	/**
	 * Create all Listeners and hook them up to callback functions.
	 */
	private void createListeners() {
		openLogFile.addActionListener(new ActionListener() {
			//Temporary, obviously
			public void actionPerformed(ActionEvent arg0) {
				//Load the default visualization
				RoadNetwork rn = null;
				try {
					rn = new RoadNetwork(Utility.LoadFileResource("res/data/default.log.txt"));
				} catch (IOException ex) {
					throw new RuntimeException(ex);
				}
				
				//Add a visualizer
				NetworkVisualizer vis = new NetworkVisualizer();
				vis.setSource(rn, 1.0, newViewPnl.getWidth(), newViewPnl.getHeight());
				
				//Update the map
				newViewPnl.drawMap(vis, 0, 0);
			}
		});
	}
}


