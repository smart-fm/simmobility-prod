package sim_mob.vis;


import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import com.xuggle.xuggler.ICodec;
import com.xuggle.xuggler.IContainer;
import com.xuggle.xuggler.IPacket;
import com.xuggle.xuggler.IPixelFormat;
import com.xuggle.xuggler.IRational;
import com.xuggle.xuggler.IStream;
import com.xuggle.xuggler.IStreamCoder;
import com.xuggle.xuggler.IVideoPicture;
import com.xuggle.xuggler.video.ConverterFactory;
import com.xuggle.xuggler.video.IConverter;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.regex.Pattern;

import sim_mob.conf.CSS_Interface;
import sim_mob.vis.controls.*;
import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.network.basic.ScaledPoint;
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
	private JButton debug;
	
    private JComboBox clockRateComboBox;
    private ImageIcon debugIcon;
    private ImageIcon displayIcon;
    
    private JComboBox trackAgentIDs;
    private JButton renderVideo;
    
	
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
	private boolean showDebugMode;
	
	
	//Helper class
	private class StringItem {
		private String name;
		private int value;
		public StringItem(String name, int value) {
			this.name = name;
			this.value = value;
		}
		public String toString() {
			return name;
		}
		public int getValue() {
			return value;
		}
	}
	
		
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
		this.showDebugMode = false;
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
	
	
	public static Hashtable<String, Color> GetOverrides(String prefix, String[] bkgrdColors, String[] lineColors) {
		Hashtable<String, Color> overrides = new Hashtable<String, Color>();
		for (String id : bkgrdColors) {
			Color clr = MainFrame.Config.getBackground(prefix+"-"+id);
			if (clr!=null) {
				overrides.put(id, clr);
			}
		}
		for (String id : lineColors) {
			Color clr = MainFrame.Config.getLineColor(prefix+"-"+id);
			if (clr!=null) {
				overrides.put(id, clr);
			}
		}
		return overrides;
	}
	
	
	/**
	 * Load all components and initialize them properly.
	 */
	private void loadComponents() throws IOException {
		playIcon = new ImageIcon(Utility.LoadImgResource("res/icons/play.png"));
		pauseIcon = new ImageIcon(Utility.LoadImgResource("res/icons/pause.png"));
		displayIcon = new ImageIcon(Utility.LoadImgResource("res/icons/display.png"));
		debugIcon = new ImageIcon(Utility.LoadImgResource("res/icons/bug.png"));
		
		console = new JTextField();
	    clockRateComboBox = new JComboBox(clockRateList);
	    trackAgentIDs = new JComboBox(new String[]{"", ""});
	    resetTrackAgentIDs(null);
	    
	    renderVideo = new JButton("Render Video");
	    
	    //Disable our render button if this clearly won't work.
	    renderVideo.setEnabled(false);
	    String osName = System.getProperty("os.name").toLowerCase();
	    if (osName.indexOf("nix")>=0 || osName.indexOf("nux")>=0) {
		    String xugglerPath = "/usr/local/xuggler/lib";
		    File f = new File(xugglerPath);
		    if (f.exists() && f.isDirectory()) {
		    	try {
		    		//Need to set java.library.path or JNI will fail.
		    		System.setProperty( "java.library.path", xugglerPath);
		    		
		    		//Need to set LD_LIBRARY_PATH or _actual_ native lookup will fail.
		    		if(!System.getenv().containsKey("LD_LIBRARY_PATH")) {
		    			throw new RuntimeException("LD_LIBRARY_PATH not set.");
		    		}
		    		String ldLib = System.getenv("LD_LIBRARY_PATH");
		    		if (!ldLib.contains(xugglerPath)) {
		    			System.out.println("LD_LIBRARY_PATH is = " + ldLib);
		    			throw new RuntimeException("LD_LIBRARY_PATH doesn't contain xuggler.");
		    		}
		    		//TODO: There are ways to hack this in too. Do it later.
		    		
		    		//Kind of a hack: refresh jni at runtime.
		    		Field fieldSysPath = ClassLoader.class.getDeclaredField( "sys_paths" );
		    		fieldSysPath.setAccessible( true );
		    		fieldSysPath.set(null, null);
		    		
		    		//Everything should work now.
		    		renderVideo.setEnabled(true);
		    	} catch (Throwable t) {}
		    }
	    }

	    
	    openLogFile = new JButton("Open File From...", new ImageIcon(Utility.LoadImgResource("res/icons/open.png")));
		openEmbeddedFile = new JButton("Open Default File", new ImageIcon(Utility.LoadImgResource("res/icons/embed.png")));
		showFakeAgent = new JButton("Show Proxy Agent", new ImageIcon(Utility.LoadImgResource("res/icons/fake.png")));
		zoomIn = new JButton("       Zoom In 	        ", new ImageIcon(Utility.LoadImgResource("res/icons/zoom_in.png")));
		zoomOut = new JButton("      Zoom Out  	    ", new ImageIcon(Utility.LoadImgResource("res/icons/zoom_out.png")));
		debug = new JButton("    Display Mode    ", displayIcon);
		
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
		jpLeft.add(showFakeAgent);
		jpLeft.add(zoomIn);
		jpLeft.add(zoomOut);
		jpLeft.add(debug);
		jpLeft.add(clockRateComboBox);
		jpLeft.add(trackAgentIDs);
		jpLeft.add(renderVideo);
		
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
		
		trackAgentIDs.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (trackAgentIDs.getSelectedIndex()==-1) {
					return;
				}
					
				newViewPnl.setHighlightID(((StringItem)trackAgentIDs.getSelectedItem()).getValue());
			}
		});
		
		showFakeAgent.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				
				if(showFake){
					newViewPnl.showFakeAgent(false);
					showFake = false;
					showFakeAgent.setText("Show Proxy Agent");

				}else{
					newViewPnl.showFakeAgent(true);
					showFake = true;
					showFakeAgent.setText("Hide Proxy Agent");
				}
			
			}
			
		});
		
		zoomIn.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				newViewPnl.zoomWithButtonClick(1);
			}
			
		});
		
		zoomOut.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				newViewPnl.zoomWithButtonClick(-1);
			}

		});

		debug.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
		
				if(showDebugMode){
					newViewPnl.showDebugMode(false);
					showDebugMode = false;
					debug.setText("    Display Mode    ");
					debug.setIcon(displayIcon);
					
				}else{
					newViewPnl.showDebugMode(true);
					showDebugMode = true;
					debug.setText("     Debug Mode    ");
					debug.setIcon(debugIcon);
				}
			}

		});
		
		renderVideo.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//Nothing to render?
				if (simData==null) {
					return;
				}
				
				//Stop animation
				if (animTimer.isRunning()) {
					animTimer.stop();
					playBtn.setIcon(playIcon);
				}
				
				//Render
				try {
					renderToFile();
				} catch (Throwable t) {
					System.out.println("Exception while rendering to file: " + t.getClass().getName());
					System.out.println(t.getMessage());
					t.printStackTrace();
				}
			}
		});

		
		openEmbeddedFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				new FileOpenThread(true).start();
			}
		});
		
		
		openLogFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				new FileOpenThread(false).start();
			}
		});
		

	}
	
	private void resetTrackAgentIDs(HashSet<Integer> allIds) {
		trackAgentIDs.removeAllItems();
		trackAgentIDs.addItem(new StringItem("Track no Agent", -1));
		if (allIds!=null) {
			//Sort
			Integer[] ids_sorted = allIds.toArray(new Integer[]{});
			Arrays.sort(ids_sorted);
			
			//Add
			for (Integer it : ids_sorted) {
				trackAgentIDs.addItem(new StringItem("Track Agent " + it.intValue(), it.intValue()));
			}
		}
	}
	
	
	//Helper
	class FileOpenThread extends Thread {
		boolean isEmbedded;
		FileOpenThread(boolean isEmbedded) {
			this.isEmbedded = isEmbedded;
		}
		
		public void run() {
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
				long fileSize = 0;
				if (isEmbedded) {
					br = Utility.LoadFileResource("res/data/default.log.txt");
					fileName = "default.log";
				} else {
					br = new BufferedReader(new FileReader(f));
					fileSize = f.length();
					fileName = f.getName();
				}

				rn = new RoadNetwork();
				rn.loadFileAndReport(br, fileSize, newViewPnl);
				
				br.close();
			} catch (IOException ex) {
				throw new RuntimeException(ex);
			}
		
			console.setText("Input File Name: "+fileName);
			
			//Clear our global scaled points array.
			ScaledPoint.ClearGlobalGroup();
			
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
			
			//Reset our Agent ID combo box.
			resetTrackAgentIDs(uniqueAgentIDs);
			
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
	
	
	private void renderToFile() {
		System.out.println("Rendering started.");
		String outFilePath = "test.mpg";
		
		//Create and open the container for our video.
		IContainer outContainer = IContainer.make();
		int retval = outContainer.open(outFilePath, IContainer.Type.WRITE, null); 
		if (retval<0) { throw new RuntimeException("Could not open output file: " + outFilePath); }
		
		//Add a stream to this container, along with a coder for that stream.
		IStream outStream = outContainer.addNewStream(0); 
		IStreamCoder outStreamCoder = outStream.getStreamCoder();
		
		//Guess encoding for this coder.
		//TODO: Force ffmpeg
		ICodec codec = ICodec.guessEncodingCodec(null, null, outFilePath, null, ICodec.Type.CODEC_TYPE_VIDEO);
		
		//Set parameters for this video.
		outStreamCoder.setNumPicturesInGroupOfPictures(10); 
		outStreamCoder.setCodec(codec); 
		outStreamCoder.setBitRate(25000); 
		outStreamCoder.setBitRateTolerance(9000); 
		outStreamCoder.setPixelType(IPixelFormat.Type.YUV420P); 
		outStreamCoder.setWidth(newViewPnl.getWidth()); 
		outStreamCoder.setHeight(newViewPnl.getHeight());
		outStreamCoder.setFlag(IStreamCoder.Flags.FLAG_QSCALE, true); 
		outStreamCoder.setGlobalQuality(0); 
		
		//Set the framerate
		if (simData.frame_length_ms==-1) { throw new RuntimeException("Simulation output data missing fps."); }
		IRational frameRate = IRational.make(30,1); 
		outStreamCoder.setFrameRate(frameRate); 
		outStreamCoder.setTimeBase(IRational.make(frameRate.getDenominator(), frameRate.getNumerator())); 
		frameRate = null;
		
		//Write the header.
		retval = outStreamCoder.open();
		if (retval<0) { throw new RuntimeException("Could not open out stream codec."); }
		
		retval = outContainer.writeHeader();
		if (retval<0) { throw new RuntimeException("Could not write header."); }
		
		//Write through each frame
		int lastPercent = 0;
		int currPercent = 0;
		int startFrame = newViewPnl.getCurrFrameTick();
		int endFrame = newViewPnl.getMaxFrameTick();
		System.out.println("0%");
		for (int i=startFrame; i<=endFrame; i++) {
			//Update
			currPercent = ((i-startFrame)*100)/(endFrame-startFrame);
			if (currPercent-lastPercent > 9) {
				lastPercent = currPercent;
				System.out.println(currPercent + "%");
			}
			
			//Get the buffered image for this frame.
			//TODO;
			BufferedImage originalImage = newViewPnl.drawFrameToExternalBuffer(i);
			
			//Convert it to the format xuggler expects.
			BufferedImage worksWithXugglerBufferedImage = convertToType(originalImage, BufferedImage.TYPE_3BYTE_BGR);
			IPacket packet = IPacket.make(); 
			IConverter converter = ConverterFactory.createConverter(worksWithXugglerBufferedImage, IPixelFormat.Type.YUV420P);
			
			//Add a timestamp
			long timeStamp = (i-startFrame)*simData.frame_length_ms*1000; // convert to microseconds 
			IVideoPicture outFrame = converter.toPicture(worksWithXugglerBufferedImage, timeStamp); 
			outFrame.setQuality(0); 
			
			retval = outStreamCoder.encodeVideo(packet, outFrame, 0);
			if (retval<0) { throw new RuntimeException("Could not encode frame id: " + (i-startFrame)); }
			
			if (packet.isComplete()) {
				retval = outContainer.writePacket(packet);
				if (retval<0) { throw new RuntimeException("Could not write frame id: " + (i-startFrame)); }
			}
		}
		
		//Finalize and close the image.
		retval = outContainer.writeTrailer();
		if (retval<0) { throw new RuntimeException("Could not write trailer"); }
		
		retval = outContainer.close();
		if (retval<0) { System.out.println("Could not close file (video may still have encoded ok)."); }
		
		System.out.println("Done");
	}
	
	//Helper method for image conversion (TODO: Do this manually in our draw step)
	private static BufferedImage convertToType(BufferedImage sourceImage, int targetType) {
		BufferedImage image;
		
		// if the source image is already the target type, return the source image
		if (sourceImage.getType() == targetType) { 
			image = sourceImage; 
		} else {
			// create a new image of the target type and draw the new image
			image = new BufferedImage(sourceImage.getWidth(), sourceImage.getHeight(), targetType);
			image.getGraphics().drawImage(sourceImage, 0, 0, null);
		}
		return image;
	}
	
}


