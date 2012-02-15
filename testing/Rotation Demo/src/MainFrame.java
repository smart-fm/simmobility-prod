import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;


public class MainFrame extends JFrame {
	private static final long serialVersionUID = 1L;

	private static final String[] allCarPaths = new String[] {
		"res/car.json.txt", "res/bus.json.txt", "res/car_lq.json.txt", "res/car_v1.json.txt",
		"res/car_v2.json.txt", "res/car_v3.json.txt", "res/person.json.txt", "res/truck.json.txt"
	};
	private static SimpleVectorImage[] allCarImages;
	static {
		try {
			allCarImages = new SimpleVectorImage[allCarPaths.length];
			int i=0;
			for (String path : allCarPaths) {
				allCarImages[i++] = SimpleVectorImage.LoadFromFile(new BufferedReader(new FileReader(path)));
			}
		} catch (FileNotFoundException ex) { throw new RuntimeException(ex); }
	};
	
	
	public MainFrame() {
		super("Rotation Testing Frame");
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		this.setSize(700, 500);
		
		loadComponents(getContentPane());
	}
	
	private static JPanel addInitPanel(JPanel pnl, JComponent sub) {
		JPanel res = new JPanel(new BorderLayout(0, 15));
		if (pnl!=null) {
			pnl.add(BorderLayout.NORTH, sub);
			pnl.add(BorderLayout.CENTER, res);
		}
		return res;
	}

	
	private void loadComponents(Container cp) {
		cp.setLayout(new BorderLayout(5, 5));
		
		//Left panel
		JPanel lPnl = addInitPanel(null, null);
		lPnl.setBorder(BorderFactory.createTitledBorder("Toolkit"));
		cp.add(BorderLayout.WEST, lPnl);
		
		//Pick an image
		JComboBox carImgs = new JComboBox();
		for (String path : allCarPaths) {
			//Too lazy to work on regexes now
			String[] pathParts = path.split("/");
			String name = pathParts[pathParts.length-1];
			String[] exts = name.split("[.]");
			String fn = exts[0];
			String[] fnParts = fn.split("_");
			
			//Res
			String res = "";
			for (String s : fnParts) {
				if (res.isEmpty()) {
					res = s.substring(0,1).toUpperCase() + s.substring(1, s.length());
				} else {
					res = res + " " + s;
				}
			}
			carImgs.addItem(res);
		}
		
		//Slider: rotation
		JLabel rotLbl = new JLabel("Rotation");
		JSlider rotationAmt = new JSlider();
		rotationAmt.setMinimum(0);
		rotationAmt.setMaximum(360-1);
		rotationAmt.setMajorTickSpacing(10);
		rotationAmt.setMinorTickSpacing(1);
		
		//Slider: scale
		JLabel scaleLbl = new JLabel("Scale");
		JSlider scaleAmt = new JSlider();
		scaleAmt.setMinimum(100);
		scaleAmt.setMaximum(1000);
		scaleAmt.setMajorTickSpacing(100);
		scaleAmt.setMinorTickSpacing(10);
		
		//Sub-panel 1
		JPanel lSub = new JPanel(new BorderLayout());
		lSub.add(BorderLayout.NORTH, new JLabel("Vector Image"));
		lSub.add(BorderLayout.CENTER, carImgs);
		lPnl = addInitPanel(lPnl, lSub);
		
		//Sub-panel 2
		lSub = new JPanel(new BorderLayout());
		lSub.add(BorderLayout.NORTH, rotLbl);
		lSub.add(BorderLayout.CENTER, rotationAmt);
		lPnl = addInitPanel(lPnl, lSub);
		
		//Sub-panel 3
		lSub = new JPanel(new BorderLayout());
		lSub.add(BorderLayout.NORTH, scaleLbl);
		lSub.add(BorderLayout.CENTER, scaleAmt);
		lPnl = addInitPanel(lPnl, lSub);
		
		
		
		//Right panel
		VectImgPanel vectPnl = new VectImgPanel();
		cp.add(BorderLayout.CENTER, vectPnl);
		
		
		//Listeners
		carImgs.addActionListener(new CarComboChangedListener(carImgs, vectPnl));
		carImgs.setSelectedIndex(0);
		
		rotationAmt.addChangeListener(new RotateLblChangeListener(rotationAmt, rotLbl, vectPnl));
		rotationAmt.setValue(0);
		
		scaleAmt.addChangeListener(new ScaleLblChangeListener(scaleAmt, scaleLbl, vectPnl));
		scaleAmt.setValue(0);
		
		
	}
	
	
	class CarComboChangedListener implements ActionListener {
		JComboBox jc;
		VectImgPanel vectPnl;
		CarComboChangedListener(JComboBox jc, VectImgPanel vectPnl) { this.jc=jc; this.vectPnl=vectPnl; }
		public void actionPerformed(ActionEvent e) {
			int i = jc.getSelectedIndex();
			if (i<0 || i>=allCarImages.length) {
				vectPnl.setCurrShape(null);
			} else {
				vectPnl.setCurrShape(allCarImages[i]);
			}
		}
	}
	
	class RotateLblChangeListener implements ChangeListener {
		JSlider slide;
		JLabel lbl;
		VectImgPanel vectPnl;
		public RotateLblChangeListener(JSlider slide, JLabel lbl, VectImgPanel vectPnl) { this.slide=slide; this.lbl=lbl; this.vectPnl=vectPnl; }
		public void stateChanged(ChangeEvent arg0) {
			lbl.setText("Rotation (" + slide.getValue() + ")");
			vectPnl.setCurrRotation(slide.getValue());
		}
	}
	
	class ScaleLblChangeListener implements ChangeListener {
		JSlider slide;
		JLabel lbl;
		VectImgPanel vectPnl;
		public ScaleLblChangeListener(JSlider slide, JLabel lbl, VectImgPanel vectPnl) { this.slide=slide; this.lbl=lbl; this.vectPnl=vectPnl; }
		public void stateChanged(ChangeEvent arg0) {
			lbl.setText("Scale (" + slide.getValue() + "%)");
			vectPnl.setCurrScale(slide.getValue());
		}
	}
	
	
	
	public static void main(String[] args) {
		new MainFrame().setVisible(true);
	}

}
