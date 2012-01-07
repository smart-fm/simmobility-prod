package sim_mob.vis;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.Hashtable;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import sim_mob.components.RangeSlider;


public class VideoEncodeDialog extends JDialog {
	public static final long serialVersionUID = 1L;
	
	private JTextField outFileName;
	private JLabel outFileCodec;
	private JSlider outFileQuality;
	private RangeSlider outFileRange;
	private JLabel outRangeLower;
	private JLabel outRangeUpper;
	
	private JButton okBtn;
	private JButton cancelBtn;
	
	private static Hashtable<String, String> SupportedEncodings;

	public VideoEncodeDialog(JFrame parent, int currFrameID, int maxFrameID) {
		super(parent, "Encode Video");
		
		//Add known encodings
		if (SupportedEncodings==null) {
			SupportedEncodings = new Hashtable<String, String>();
			SupportedEncodings.put("mp4", "MPEG-4");
			SupportedEncodings.put("m4v", "MPEG-4");
			SupportedEncodings.put("mov", "Quicktime");
			SupportedEncodings.put("flv", "Flash");
			SupportedEncodings.put("avi", "AVI");
			SupportedEncodings.put("h263", "H.263");
			SupportedEncodings.put("h264", "H.264");
			SupportedEncodings.put("ogg", "Ogg");
			SupportedEncodings.put("swf", "Flash");
		}
		
		Container cp = getContentPane();
		cp.setLayout(new GridLayout(0, 1));
		addComponents(cp, currFrameID, maxFrameID);
		addWindowListener(new WindowListener() {
			public void windowOpened(WindowEvent arg0) {}
			public void windowIconified(WindowEvent arg0) {}
			public void windowDeiconified(WindowEvent arg0) {}
			public void windowDeactivated(WindowEvent arg0) {}
			public void windowClosing(WindowEvent arg0) {
				outFileName.setText("");
			}
			public void windowClosed(WindowEvent arg0) {}
			public void windowActivated(WindowEvent arg0) {}
		});
		setSize(350, 250);
		setModal(true);
	}
	
	private void addComponents(Container cp, int currFrameID, int maxFrameID) {
		Font f = new Font("Arial", Font.PLAIN, 14);
		Font f2 = new Font("Arial", Font.PLAIN, 12);
		
		//The top panel
		{
		JLabel outFileLbl = new JLabel("<html><b>File:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b></html>");
		outFileLbl.setFont(f);
		outFileName = new JTextField();
		outFileCodec = new JLabel("<html><b>Codec:</b> <i>Unknown</i></html>");
		outFileCodec.setFont(f2);
		JPanel innerPnl = new JPanel(new BorderLayout());
		innerPnl.add(BorderLayout.NORTH, new JPanel());
		innerPnl.add(BorderLayout.CENTER, outFileName);
		innerPnl.add(BorderLayout.SOUTH, outFileCodec);
		JPanel pnl = new JPanel(new BorderLayout());
		pnl.add(BorderLayout.WEST, outFileLbl);
		pnl.add(BorderLayout.CENTER, innerPnl);
		cp.add(pnl);
		}
		
		//The middle panel
		{
		JLabel outQualityLbl = new JLabel("<html><b>Quality:&nbsp;&nbsp;</b></html>");
		outQualityLbl.setFont(f);
		outFileQuality = new JSlider(0, 20, 20);
		Hashtable<Integer, JLabel> labels = new Hashtable<Integer, JLabel>();
		labels.put(new Integer(0), new JLabel("Min"));
		labels.put(new Integer(20), new JLabel("Max"));
		labels.get(new Integer(0)).setFont(f2);
		labels.get(new Integer(20)).setFont(f2);
		outFileQuality.setLabelTable(labels);
		outFileQuality.setPaintLabels(true);
		JPanel pnl = new JPanel(new BorderLayout());
		pnl.add(BorderLayout.WEST, outQualityLbl);
		pnl.add(BorderLayout.CENTER, outFileQuality);
		cp.add(pnl);
		}
		
		//The lower panel
		{
		JLabel outRangeLbl = new JLabel("<html><b>Range:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b></html>");
		outRangeLbl.setFont(f);
		outRangeLower = new JLabel();
		outRangeLower.setFont(f2);
		outRangeUpper = new JLabel();
		outRangeUpper.setFont(f2);
		outFileRange = new RangeSlider(0, maxFrameID);
		outFileRange.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				outRangeLower.setText(String.valueOf(outFileRange.getValue()));
				outRangeUpper.setText(String.valueOf(outFileRange.getUpperValue()));
			}
		});
		outFileRange.setValue(currFrameID);
		outFileRange.setUpperValue(maxFrameID);
		JPanel innerPnl2 = new JPanel(new BorderLayout());
		innerPnl2.add(BorderLayout.WEST, outRangeLower);
		innerPnl2.add(BorderLayout.EAST, outRangeUpper);
		innerPnl2.add(BorderLayout.CENTER, new JPanel());
		JPanel innerPnl = new JPanel(new BorderLayout());
		innerPnl.add(BorderLayout.CENTER, outFileRange);
		innerPnl.add(BorderLayout.SOUTH, innerPnl2);
		JPanel pnl = new JPanel(new BorderLayout());
		pnl.add(BorderLayout.WEST, outRangeLbl);
		pnl.add(BorderLayout.CENTER, innerPnl);
		cp.add(pnl);
		}
		
		//A blank panel in between
		cp.add(new JPanel(new BorderLayout()));
		
		//The Ok/Cancel buttons
		{
		okBtn = new JButton("<html><b>Render</b></html>");
		okBtn.setFont(f);
		cancelBtn = new JButton("Cancel");
		cancelBtn.setFont(f);
		JPanel innerPnl = new JPanel(new BorderLayout());
		innerPnl.add(BorderLayout.WEST, okBtn);
		innerPnl.add(BorderLayout.EAST, cancelBtn);
		JPanel pnl = new JPanel(new FlowLayout(FlowLayout.RIGHT));
		pnl.add(innerPnl);
		cp.add(pnl);
		}

		//Add a few more action listeners
		okBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				setVisible(false);
			}
		});
		cancelBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				outFileName.setText("");
				setVisible(false);
			}
		});
		
		//Encoding checker
		outFileName.getDocument().addDocumentListener(new DocumentListener() {
			public void removeUpdate(DocumentEvent arg0) {
				update();
			}
			public void insertUpdate(DocumentEvent arg0) {
				update();
			}
			public void changedUpdate(DocumentEvent arg0) {
				update();
			}
			private void update() {
				String codec = "Unknown";
				if (!outFileName.getText().isEmpty()) {
					String[] parts = outFileName.getText().split("[.]");
					String ext = parts[parts.length-1].toLowerCase();
					if (SupportedEncodings.containsKey(ext)) {
						codec = SupportedEncodings.get(ext);
					}
				}
				outFileCodec.setText("<html><b>Codec:</b> <i>" + codec + "</i></html>");
			}
		});
		
		//And suggest something
		outFileName.setText("output.mp4");
	}
	
	//Helpers
	public String getOutFileName() {
		return outFileName.getText();
	}
	public int getOutFileQuality() {
		return outFileQuality.getMaximum() - outFileQuality.getValue();
	}
	public int getOutFileFirstFrame() {
		return outFileRange.getValue();
	}
	public int getOutFileLastFrame() {
		return outFileRange.getUpperValue();
	}
	
}


