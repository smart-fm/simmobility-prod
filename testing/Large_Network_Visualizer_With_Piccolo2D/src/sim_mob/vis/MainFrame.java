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
import sim_mob.conf.CSS_Interface;
import sim_mob.vis.controls.*;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.Utility;


public class MainFrame extends JFrame {
	public static final long serialVersionUID = 1L;
		
	private static final String clockRateList[] = {"default-50ms","10 ms", "50 ms", "100 ms", "200 ms","500 ms","1000 ms"};
	
	private static final double MEGABYTE = 1024.0 * 1024.0;
	public static double bytesToMegabytes(long bytes) {
		return bytes / MEGABYTE;
	}
	
	//Canvas that make use of the PCanvas
	private NetworkVisualizer netViewPanel;
	private NetSimAnimator netViewAnimator;
	private SimulationResults simData;
	
	private JTextField console;
	
	//LHS panel
	private Timer memoryUsageTimer;
	private JLabel memoryUsage;
	private JButton openLogFile;
	private JButton openEmbeddedFile;
	private JToggleButton zoomSquare;
	private JButton zoomIn;
	private JButton zoomOut;
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
	
	//For zooming
	private static final Stroke onePtStroke = new BasicStroke(1.0F);
	private MouseAdapter currZoomer;
	class MouseRectZoomer extends MouseAdapter {
		Point startPoint;
		
		public void mousePressed(MouseEvent e) {
			startPoint = e.getPoint();
		}
		public void mouseExited(MouseEvent e) {
			//startPoint = null;  
		}
		public void mouseReleased(MouseEvent e) {
			if (startPoint!=null) {
				netViewPanel.setZoomBox(new Rectangle2D.Double(startPoint.x, startPoint.y, e.getX()-startPoint.x, e.getY()-startPoint.y));
				netViewPanel.zoomToBox();
			}
			netViewPanel.setZoomBox(null);
			releaseZoomSquare();
		}
		public void mouseDragged(MouseEvent e) {
			//Provide some feedback.
			if (startPoint != null) {
				//netViewPanel.repaint();  //NOTE: This is probably better done with a camera-constant object.
				//Graphics2D g = (Graphics2D)netViewPanel.getGraphics();
				//g.setColor(Color.red);
				//g.setStroke(onePtStroke);
				netViewPanel.setZoomBox(new Rectangle2D.Double(startPoint.x, startPoint.y, e.getX()-startPoint.x, e.getY()-startPoint.y));
				//Rectangle2D rect = new Rectangle2D.Double(startPoint.x, startPoint.y, e.getX()-startPoint.x, e.getY()-startPoint.y);
				//g.draw(rect);
			}
		}
	}

	
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
		memoryUsage = new JLabel();
		openLogFile = new JButton("Open Logfile", new ImageIcon(Utility.LoadImgResource("res/icons/open.png")));
		openEmbeddedFile = new JButton("Open Default", new ImageIcon(Utility.LoadImgResource("res/icons/embed.png")));
	    clockRateComboBox = new JComboBox(clockRateList);
	    zoomSquare = new JToggleButton("Zoom Box");
	    zoomIn = new JButton("+");
	    zoomOut = new JButton("-");
	    

		frameTickSlider = new JSlider(JSlider.HORIZONTAL);
		revBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/rev.png")));
		playBtn = new JButton(playIcon);
		fwdBtn = new JButton(new ImageIcon(Utility.LoadImgResource("res/icons/fwd.png")));

		netViewPanel = new NetworkVisualizer(300,300);
	}
	
	/**
	 * Add all components to the frame.
	 */
	private void addComponents(Container cp) {
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
		//cp.add(newViewPnl, gbc);
		cp.add(netViewPanel, gbc);
		

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
					currZoomer = new MouseRectZoomer();
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
					playBtn.setIcon(pauseIcon);
				} else {
					animTimer.stop();
					playBtn.setIcon(playIcon);
				}
			}
		});
		
		fwdBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (netViewAnimator.advanceAnimbyStep(1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());

					return;
				}
				
			}
			
		});
		
		revBtn.addActionListener(new ActionListener(){
			
			public void actionPerformed(ActionEvent arg0) {
				if (netViewAnimator.advanceAnimbyStep(-1, frameTickSlider)) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
					console.setText("Input File Name: "+frameTickSlider.getValue());
					return;
				}
				
			}
			
		});
		
		memoryUsageTimer = new Timer(1000, new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				Runtime runtime = Runtime.getRuntime();
				long memory = runtime.totalMemory() - runtime.freeMemory();
				String memTxt = new DecimalFormat("#.#").format(bytesToMegabytes(memory)); //"1.2"
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
							
							if (netViewAnimator.advanceAnim(1, frameTickSlider)) {
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
	
	
	private void openAFile(boolean isEmbedded) {
		
		//System.out.println(virtual_newViewPnl.getCamera().getScale());
		//System.out.println(virtual_newViewPnl.getLayer().getScale());
		
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
		//String fileName;
		try {
			BufferedReader br = null;
			//long fileSize = 0;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
				//fileName = "default.log";
			} else {
				br = new BufferedReader(new FileReader(f));
				//fileSize = f.length();
				//fileName = f.getName();
			}
 
			rn = new RoadNetwork(br,netViewPanel.getWidth(), netViewPanel.getHeight());
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
		
		//Remove all children (if reloading)
		netViewPanel.getLayer().removeAllChildren();
		
		//Now add all children again
		netViewPanel.buildSceneGraph(rn, simData, uniqueAgentIDs);
		netViewAnimator = new NetSimAnimator(netViewPanel, simData, frameTickSlider);
		
		
		//NetworkVisualizer vis = new NetworkVisualizer(virtual_newViewPnl.getWidth(), virtual_newViewPnl.getHeight());
				
		//vis.setVis(rn, simData, uniqueAgentIDs);
		
		//virtual_newViewPnl.iniMapCache(vis);
		//virtual_newViewPnl.drawMap(rn, simData);
		
		
		//Reset the view
		Rectangle2D initialBounds = netViewPanel.getNaturalBounds();
		netViewPanel.getCamera().animateViewToCenterBounds(initialBounds, true, 1000);
		
	}
	
}


