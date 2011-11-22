package sim_mob.conf;

import java.awt.Color;
import java.awt.Stroke;
import java.io.BufferedReader;
import java.io.File;
import java.util.ArrayList;
import java.util.Hashtable;

/**
 * Load a CSS_Interface via the Batik library.
 * 
 * @author sethhetu
 */
public class BatikCSS_Loader {
	//Load each file in order, overwriting settings as you go.
	public static CSS_Interface LoadCSS_Interface(ArrayList<BufferedReader> files) {
		CSS_Interface csi = new CSS_Interface();
		for (BufferedReader f : files) {
			LoadSingleFile(csi, f);
		}
		return csi;
	}
	
	private static void LoadSingleFile(CSS_Interface res, BufferedReader f) {
		
	}

}
