package sim_mob.vis;

import java.awt.*;
import java.io.IOException;

import javax.swing.*;

import net.miginfocom.swing.MigLayout;

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
	
	//Turn this on to draw dotted boxes around all laid out components. Very useful for fixing layout glitches.
	private static final boolean DebugLayout = false;
	
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
	private static ImageIcon zoomSquareIcon;
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
		getContentPane().add(BorderLayout.CENTER, layoutComponents());
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
		zoomSquareIcon = MakeImageIcon("res/icons/zoom_square.png");
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
	    zoomSquare = new JToggleButton();
	    if (zoomSquareIcon!=null) {
	    	zoomSquare.setIcon(zoomSquareIcon);
	    	zoomSquare.setText("Box");
	    } else {
	    	zoomSquare.setText("Zoom Box");
	    }
	    zoomIn = new JButton();
	    if (zoomInIcon!=null) {
	    	zoomIn.setIcon(zoomInIcon);
	    } else {
	    	zoomIn.setText("+");
	    }
	    zoomOut = new JButton();
	    if (zoomOutIcon!=null) {
	    	zoomOut.setIcon(zoomOutIcon);
	    } else {
	    	zoomOut.setText("-");
	    }
	    
	    //The main zoomable interface
		netViewPanel = new NetworkVisualizer(300,300);
	}
	
	
	
	private JPanel layoutComponents() {
		MigLayout ml = new MigLayout((DebugLayout?"debug":"")+",insets 5px 5px 5px 5px", "[left]rel[center]", "[top]rel[bottom]rel[bottom]");
		JPanel jp = new JPanel(ml);
		
		//LHS
		String sg = "sizegroup lhs_sz,";
		jp.add(memoryUsage, sg+"cell 0 0, split, flowy");
		jp.add(openLogFile, sg);
		jp.add(openEmbeddedFile, sg);
		jp.add(clockRateComboBox, sg);
		
		//Zoom: here we cheat a little
		JPanel zoomPnl = new JPanel();
		zoomPnl.setLayout(new BoxLayout(zoomPnl, BoxLayout.X_AXIS));
		zoomPnl.add(zoomSquare);
		zoomPnl.add(zoomIn);
		zoomPnl.add(zoomOut);
		jp.add(zoomPnl, sg+"skip");
		
		//
		jp.add(netViewPanel, "id netView, cell 1 0, width 100%, height 100%");
		
		//
		String sg2 = "sizegroup play_sz,";
		jp.add(playBtn, sg2+"id playBtn, cell 1 1");
		jp.add(revBtn, sg2+"id revBtn, pos null (playBtn.y+(playBtn.h/2)-(revBtn.h/2)) (playBtn.x - rel)");
		jp.add(fwdBtn, sg2+"id fwdBtn, pos (playBtn.x2 + rel) (playBtn.y+(playBtn.h/2)-(fwdBtn.h/2))");
		
		
		//Attachment makes the most sense for the progress bar
		//TODO: The progress bar will disappear when the netView panel is clicked on. Should
		//      be simple enough to put it back on top when we update its value. 
		jp.add(generalProgress, "pos (netView.x) null (netView.x2) (netView.y2)");
		jp.setComponentZOrder(generalProgress, 0);
		
		
		//
		jp.add(console, "dock south");
		
		return jp;
	}
	
	public static void main(String[] args) {
		new MainFrameUI("Sim Mobility Visualizer - Testing Layout").setVisible(true);
	}
	

}
