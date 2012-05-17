import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.HashSet;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;


//A simple Swing form to test our spatial index.
public class MainFrame extends JFrame {
	private static final long serialVersionUID = 1L;
	
	private JSlider zoomSlider;
	private ShapePanel mainPanel;
	private JLabel healthLbl;
	
	//This value is valid during the paint loop.
	private static Graphics2D g;
	
	public MainFrame() {
		super("Lazy Spatial Index Tester");

		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocation(150, 100);
		this.setSize(806, 599);

		setLayout(new BorderLayout());
		loadComponents(this.getContentPane());
		addShapes();
		createListeners();
	}
	
	private void loadComponents(Container cp) {
		//Simple panel setup.
		JPanel topPanel = new JPanel(new BorderLayout());
		mainPanel = new ShapePanel();
		mainPanel.setBackground(Color.lightGray);
		JPanel bottomPanel = new JPanel(new BorderLayout());
		cp.add(BorderLayout.NORTH, topPanel);
		cp.add(BorderLayout.CENTER, mainPanel);
		cp.add(BorderLayout.SOUTH, bottomPanel);
		
		//Fake "insets"
		((BorderLayout)topPanel.getLayout()).setVgap(10);
		topPanel.add(BorderLayout.NORTH, new JPanel());
		topPanel.add(BorderLayout.SOUTH, new JPanel());
		
		//Top panel has our slider
		topPanel.add(BorderLayout.WEST, new JLabel("Scale Zoom Box:"));
		zoomSlider = new JSlider(0, 100);
		zoomSlider.setValue(0);
		topPanel.add(BorderLayout.CENTER, zoomSlider);
		
		//Bottom panel holds health statistics
		healthLbl = new JLabel("Health: (0.0, 0.0)");
		bottomPanel.add(BorderLayout.CENTER, healthLbl);
	}
	
	private void createListeners() {
		zoomSlider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				mainPanel.setZoom(zoomSlider.getValue()/100.0);
			}
		});
		
	}
	
	
	
	//Panel is roughly 800 x 500
	private void addShapes() {
		//Long, thin item
		mainPanel.addShape(new Line2D.Double(130, 240, 700, 50));
  		
		//Series of circles
		mainPanel.addShape(new Ellipse2D.Double(500, 260, 10, 10));
		mainPanel.addShape(new Ellipse2D.Double(500, 270, 10, 10));
		mainPanel.addShape(new Ellipse2D.Double(520, 260, 10, 10));
		mainPanel.addShape(new Ellipse2D.Double(520, 270, 10, 10));
		mainPanel.addShape(new Ellipse2D.Double(480, 240, 10, 10));
		
		//Some rectangles WAY away from the center
		mainPanel.addShape(new Rectangle2D.Double(10, 10, 20, 20));
		mainPanel.addShape(new Rectangle2D.Double(770, 470, 20, 20));
		mainPanel.addShape(new Rectangle2D.Double(760, 470, 20, 20));
		mainPanel.addShape(new Rectangle2D.Double(770, 460, 20, 20));
		mainPanel.addShape(new Rectangle2D.Double(760, 460, 20, 20));
		
		//Save the overall health:
		Point2D health = mainPanel.getHealth();
		healthLbl.setText("Health: (" + health.getX() + ", " + health.getY() + ")");
	}
	
	
	
	//Helper class which draws shapes with different colors based on whether or not they are inside a 
	// centered bounding box.
	class ShapePanel extends JPanel {
		private static final long serialVersionUID = 1L;
		
		private Rectangle2D currZoom;
		private LazySpatialIndex<Shape> objects;
		
		public ShapePanel() {
			currZoom = new Rectangle2D.Double(0, 0, 0, 0);
			objects = new LazySpatialIndex<Shape>();
		}
		
		public void setZoom(double percent) {
			double w = getWidth()*percent;
			double h = getHeight()*percent;
			currZoom.setRect(getWidth()/2-w/2, getHeight()/2-h/2, w, h);
			repaint();
		}
		
		
		public void addShape(Shape sh) {
			objects.addItem(sh, sh.getBounds2D());
		}
		
		public Point2D getHealth() {
			return objects.estimateHealth();
		}
		
		
		public void paint(Graphics g1) {
			g = (Graphics2D)g1;
			g.setColor(getBackground());
			g.fillRect(0, 0, getWidth(), getHeight());
			
			//Prepare a painting action that will avoid painting duplicates.
			DrawAndTagAction painter = new DrawAndTagAction(new HashSet<Shape>(), g);
			g.setStroke(new BasicStroke(1.0F));
			
			//Draw all "matched" items.
			g.setColor(Color.blue);
			objects.forAllItemsInRange(currZoom, painter, null);
			
			//Draw all "false positive" items.
			g.setColor(new Color(0x00, 0x80, 0x00));
			objects.forAllItemsInRange(currZoom, null, painter);
			
			//Draw all remaining items.
			g.setColor(Color.black);
			objects.forAllItems(painter);

			
			//Draw the current zoom level
			if (!currZoom.isEmpty()) {
				//Draw the "actual" zoom box
				g.setStroke(new BasicStroke(2.0F));
				g.setColor(new Color(0x00, 0x80, 0x00));
				g.draw(objects.getActualSearchRectangle(currZoom));
								
				//Draw the search rectangle.
				g.setStroke(new BasicStroke(1.0F));
				g.setColor(Color.red);
				g.draw(currZoom);
			}
			

			//Avoid holding on to this.
			g = null;
		}
		
	}
	
	
	class DrawAndTagAction implements LazySpatialIndex.Action<Shape> {
		private HashSet<Shape> alreadyDrawn;
		private Graphics2D g;
		
		DrawAndTagAction(HashSet<Shape> drawnShapes, Graphics2D g) {
			alreadyDrawn = drawnShapes;
			this.g = g;
		}
		
		public void doAction(Shape item) {
			if (!alreadyDrawn.contains(item)) {
				alreadyDrawn.add(item);
				g.draw(item);
			}
		}
	}


	public static void main(String[] args) {
		new MainFrame().setVisible(true);
	}

}








