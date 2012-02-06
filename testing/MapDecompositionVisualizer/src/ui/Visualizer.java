package ui;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;

public class Visualizer extends JFrame {

	private int win_width = 900;
	private int win_height = 700;
	
	public Visualizer() {
		initUI();
	}

	public final void initUI() {

		DrawPanel dpnl = new DrawPanel();
		add(dpnl);

		setSize(win_width, win_height);
		setTitle("MapDecompositionVisualizer");
		setLocationRelativeTo(null);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}

	public static void main(String[] args) {

		SwingUtilities.invokeLater(new Runnable() {

			public void run() {
				Visualizer ex = new Visualizer();
				ex.setVisible(true);
			}
		});
	}
}
