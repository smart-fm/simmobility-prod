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
	
	
	//Returns a JPanel containing all of the components in the UI, properly laid out. 
	//This should be put into an expanding layout, e.g., BorderLayout's CENTER position, or a
	//GridLayout with a single cell.
	private JPanel layoutComponents() {
		//NOTE: MigLayout is the only layout manager for Java that's designed with any amount of
		//      intelligence. (Just look at our old GridBagLayout code if you need convincing). 
		//      Unfortunately, it is very complex. I'll try to comment as much as possible throughout
		//      this function. When in doubt, refer to the original MigLayout white paper.
		
		//The "layout" constraints apply to the entire form.
		//"debug" - Show dotted lines around each component/cell. Very useful for diagnosing layout problems.
		//"insets" - Amount of space to leave around the border of this panel.
		final String layoutConstraints = (DebugLayout?"debug":"")+",insets 5px 5px 5px 5px";
		
		//The "column" constraints define alignment and other rules for each column, individually.
		//We define two columns, one that is "[left]" aligned an contains our toolbar buttons (load, zoom, etc.)
		//  and one that is "[center]" aligned and contains the main network visualization and the 
		//  play/fwd/rev buttons. In between them is "rel" units of space; here, "rel" is a keyword that
		//  means the gap size between "related" components (it's the default). You could also put a pixel
		//  value here if you want.
		final String columnConstraints = "[left]rel[center]";
		
		//The "row" constraints define alignment and other rules for each row, individually.
		//We define two rows, one that is "[top]" aligned and contains the toolbar buttons and 
		//  network visualizer, and one that is "bottom" aligned and contains the play/fwd/zoom buttons.
		//  Between them is a "rel" spacer; see above. 
		final String rowConstraints = "[top]rel[bottom]";
		
		//Now, we initialize our MigLayout and the JPanel holding it. A few things to note about these constraints:
		//  1) Most constraints can be over-ridden on a cell-by-cell basis. They are mostly just defaults.
		//  2) Some items, like our "console", are not actually located in a cell at all (they are "docked").
		//  3) Other items, like each individual toolbar button, are added by "splitting" a cell into multiple
		//     sub-cells. Cells can also "span" into other cells, so this structure is actually quite flexible.
		//  4) Finally, some items (like the fwd/rev buttons) are attached to other items (in this case the play button)
		//     instead of residing in a cell. This allows us to specify components as they naturally occur, instead
		//     of requiring a cell for each one.
		//Note that items are added like so:
		//  jp.add(component, constraints)
		//...where "constraints" is a string containing additional MigLayout constraints for this component.
		MigLayout ml = new MigLayout(layoutConstraints, columnConstraints, rowConstraints);
		JPanel jp = new JPanel(ml);
		
		//Add the buttons in the toolbar on the LHS. We use a "sizegroup" called "lhs_sz" here;
		//  anything that belongs to this sizegroup will have *exactly* the same width/height, as
		//  defined by the largest component in the size group.
		//The "cell 0 0" command means "put memoryUsage into cell (0,0)"
		//The "split, flowy" commands mean "split this cell until further notice, and add components
		//  with an increasing y-position"
		//The "skip" command (shown later) means to break out of the split. After that, the next
		//  component will be added at (1,0) (or 0,1) instead of in the split (0,0).
		String sg = "sizegroup lhs_sz,";
		jp.add(memoryUsage, sg+"cell 0 0, split, flowy");
		jp.add(openLogFile, sg);
		jp.add(openEmbeddedFile, sg);
		jp.add(clockRateComboBox, sg);
		
		//Add the zoom panels. Here we cheat a little by using a BoxLayout; the problem
		// is that MigLayout doesn't allow splitting recursively. We could have attach()'d instead
		// of splitting to add the toolbar buttons (and then split here) but BoxLayout is
		// simple and does what we want here anyway.
		//Here is where we "skip" out of cell (0,0). If you want to add more toolbar items, move
		// the "skip" command to the last one you add.
		JPanel zoomPnl = new JPanel();
		zoomPnl.setLayout(new BoxLayout(zoomPnl, BoxLayout.X_AXIS));
		zoomPnl.add(zoomSquare);
		zoomPnl.add(zoomIn);
		zoomPnl.add(zoomOut);
		jp.add(zoomPnl, sg+"skip");
		
		//Add the network visualizer.
		//Use the "width 100%" and "height 100%" commands to make it take up as much
		//  space as possible. (If there are two components with width/height 100% then
		//  they will share the remaining space in a 50/50 split).
		//The command "id" assigns the identifier "netView" to the "netViewPanel". This will
		//  allow us to access it later, when we want to attach the progress bar to it.
		jp.add(netViewPanel, "id netView, cell 1 0, width 100%, height 100%");
		
		//Add the play, fwd, and reverse buttons. The play button simple goes into cell 
		//  (1,1), which is centered by default. We also assign it an id (playBtn) so that 
		//  we can access it later. Finally, we add it to a "sizegroup" called "play_sz" to force
		//  the other buttons to expand to the size of the biggest button in the group.
		//Next, we specify the rev/fwd buttons' "pos" attributes. pos is specified as "pox x1 y1 x2 y2" where
		//  (x1,y1) is the upper-left corner of the component and (x2,y2) is the lower-right corner.
		//  Any of these can be "null", which instructs the layout engine to figure it out.
		//  If, for example, we set y1 to 50% of the screen height via "pos null 50%" then it will
		//  appear vertically halfway down the screen (and horizontally at x==0). Note that "x" and "y"
		//  mean "x1" and "y1".
		//We specify rev's x2 position relative to the x1 position of play. This pushes it to the LEFT of 
		//  the play button. We do the reverse for fwd. Then, we specify the y1 position of each of these based
		//  on the position and size of the play button (we are just centering it vertically). If, for example,
		//  you *remove* the size group from these three components, you will see that the items are vertically
		//  centered.
		String sg2 = "sizegroup play_sz,";
		jp.add(playBtn, sg2+"id playBtn, cell 1 1");
		jp.add(revBtn, sg2+"id revBtn, pos null (playBtn.y+(playBtn.h/2)-(revBtn.h/2)) (playBtn.x - rel)");
		jp.add(fwdBtn, sg2+"id fwdBtn, pos (playBtn.x2 + rel) (playBtn.y+(playBtn.h/2)-(fwdBtn.h/2))");
		
		//We then add the progress bar, attaching it to the bottom of the netView panel.
		//We set the z-order so that it always appears above the netView panel.
		//TODO: For some reason, the netView panel is overriding the progress bar. This is
		//      a minor issue (since we hide the netview panel when we show the progress bar)
		//      but it should be fixed eventually (for rendering).
		jp.add(generalProgress, "pos (netView.x) null (netView.x2) (netView.y2)");
		jp.setComponentZOrder(generalProgress, 0);
		
		//Add the "console" box. We could put this into a table cell, but there's no reason to 
		//  force ourselves to maintain a number for it (0,2) and spanning properties. Instead,
		//  we simply "dock" it to the "south" of the form, which tells the layout engine to
		//  subtract its height from the total available height of the Container. Very simple.
		jp.add(console, "dock south");
		
		//Finally, we return the panel, which can be placed in any layout or simply made to fill
		// the entire JForm.
		return jp;
	}
	
	
	///This method tests our layout engine. It shows all components at the same time; use this to
	///  see the progressBar glitch, for example.
	public static void main(String[] args) {
		new MainFrameUI("Sim Mobility Visualizer - Testing Layout").setVisible(true);
	}
	

}
