package sim_mob.vis;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import sim_mob.conf.BatikCSS_Loader;
import sim_mob.conf.CSS_Interface;
import sim_mob.vis.util.Utility;

/**
 * Holds the main method for loading our visualizer. Also contains several 
 * auxiliary functions (such as loading config files).
 * 
 * \author Seth N. Hetu
 */
public class Main {	
	private static ArrayList<BufferedReader> GetConfigFiles() {
		ArrayList<BufferedReader> files = new ArrayList<BufferedReader>();
		
		//Try adding the default file.
		try {
			files.add(Utility.LoadFileResource("res/config/default_colors.css"));
		} catch (IOException ex) {
			throw new RuntimeException("Can't load default colors file (critical resource).");
		}
		
		//Add a colors.css file if it exists in the working directory.
		File localConf = new File("colors.css");
		if (localConf.exists()) {
			try {
				files.add(new BufferedReader(new FileReader(localConf)));
			} catch (FileNotFoundException fex) {}
		}
		
		return files;
	}
	

	
	public static void main(String[] args) {
		CSS_Interface config = BatikCSS_Loader.LoadCSS_Interface(GetConfigFiles());
		new MainFrame(config).setVisible(true);
		

	}
}

