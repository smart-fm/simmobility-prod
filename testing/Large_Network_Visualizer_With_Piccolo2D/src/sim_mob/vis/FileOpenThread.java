package sim_mob.vis;

import java.io.*;
import java.util.HashSet;
import javax.swing.*;

import sim_mob.vis.network.RoadNetwork;
import sim_mob.vis.simultion.SimulationResults;
import sim_mob.vis.util.Utility;


/**
 * Helper class for loading a large simulation output file. This class operates in its own thread, and
 * provides several synchronized functions for detecting the progress of loading that file. 
 * 
 * \author Seth N. Hetu
 */
public class FileOpenThread extends Thread {
	private MainFrame parent;
	private boolean isEmbedded;
	
	private boolean done;
	private SimulationResults results;
	private RoadNetwork roadNetwork;
	private HashSet<Integer> uniqueAgentIDs;
	private double percentDone;
	
	FileOpenThread(MainFrame parent, boolean isEmbedded) {
		this.parent = parent;
		this.isEmbedded = isEmbedded;
	}
	
	public boolean isDone() {
		synchronized (this) {
			return done;
		}
	}
	
	public SimulationResults getResults() {
		synchronized (this) {
			return results;
		}
	}
	
	//Guaranteed to have data if getResults() returns true. 
	public RoadNetwork getRoadNetwork() {
		return roadNetwork;
	}
	public HashSet<Integer> getUniqueAgentIDs() {
		return uniqueAgentIDs;
	}
	
	//Returns, e.g., 0.9 for 90%
	public double getPercentDone() {
		synchronized (this) {
			return percentDone;
		}
	}
	
	//Used by the file loader.
	public void setPercentDone(double value) {
		synchronized (this) {
			percentDone = value;
		}
	}
	
	public void run() {
		//Use a FileChooser
		File f = null;
		if (!isEmbedded) {
			final JFileChooser fc = new JFileChooser("src/res/data");
			if (fc.showOpenDialog(parent)!=JFileChooser.APPROVE_OPTION) {
				synchronized (this) {
					done = true;
				}
				return;
			}
			f = fc.getSelectedFile();
		}

		//Load the default visualization
		//String fileName;
		try {
			BufferedReader br = null;
			long fileSize = 0;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
				//fileName = "default.log";
			} else {
				br = new BufferedReader(new FileReader(f));
				fileSize = f.length();
				//fileName = f.getName();
			}

			roadNetwork = new RoadNetwork(br, fileSize, this);
			
			br.close();
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		//console.setText("Input File Name: "+fileName);
		
		//Load the simulation's results
		SimulationResults simData;
		this.uniqueAgentIDs = new HashSet<Integer>();
		try {
			BufferedReader br = null;
			if (isEmbedded) {
				br = Utility.LoadFileResource("res/data/default.log.txt");
			} else {
				br = new BufferedReader(new FileReader(f));
			}
			simData = new SimulationResults(br, roadNetwork, uniqueAgentIDs);
			br.close();
		} catch (IOException ex) {
			throw new RuntimeException(ex);
		}
		
		
		//Done
		synchronized (this) {
			results = simData;
			done = true;
		}
	}
}
