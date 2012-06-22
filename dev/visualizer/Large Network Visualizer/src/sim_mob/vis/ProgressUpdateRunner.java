package sim_mob.vis;

import java.awt.Color;

import sim_mob.vis.controls.NetworkPanel;

/**
 * Helper callback for updating the progress bar.
 * \author Seth N. Hetu
 */

public class ProgressUpdateRunner extends Thread {
	NetworkPanel pnl;
	double value;
	boolean knownSize;
	Color color;
	String caption;
	
	public ProgressUpdateRunner(NetworkPanel pnl, double value, boolean knownSize, Color color, String caption) {
		if (pnl==null) { throw new RuntimeException("Can't create progress runner with a null panel."); }
		
		this.pnl = pnl;
		this.value = value;
		this.knownSize = knownSize;
		this.color = color;
		this.caption = caption;
	}
	
	public void run() {
		pnl.drawBufferAsProgressBar(value, knownSize, color, caption);
	}
}
