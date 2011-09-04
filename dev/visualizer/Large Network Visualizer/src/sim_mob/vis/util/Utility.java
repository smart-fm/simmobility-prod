package sim_mob.vis.util;

import java.awt.image.BufferedImage;
import java.io.*;
import java.util.regex.Pattern;
import javax.imageio.ImageIO;
import sim_mob.vis.Main;

public class Utility {
	/**
	 * Helper method: load an image resource using the classpath. 
	 *   (This is required if we package the visualizer as a JAR, e.g., if we create an applet.)
	 * @param path The full path of the image to load. E.g., "res/icons/myicon.png" 
	 * @return The Image resource as a BufferedImage. Null can technically be returned; see ImageIO.read(). 
	 * @throws IOException If the given file isn't found.
	 */
	public static BufferedImage LoadImgResource(String path) throws IOException {
		InputStream input = Main.class.getClassLoader().getResourceAsStream(path);
		if (input==null) {
			System.out.println("Could not find image resource: " + path);
		}
		return ImageIO.read(input);
	}
	
	
	public static BufferedReader LoadFileResource(String path) throws IOException {
		InputStream input = Main.class.getClassLoader().getResourceAsStream(path);
		if (input==null) {
			System.out.println("Could not find image resource: " + path);
		}
		return new BufferedReader(new InputStreamReader(input));
	}
	
	
	//Parse an integer that might have an 0x in front.
	public static int ParseIntOptionalHex(String input) {
		int radix = 10;
		if (input.startsWith("0x")) {
			input = input.substring(2);
			radix = 0x10;
		}
		return Integer.parseInt(input, radix);
	}
	
	
	public static void CheckBounds(double[] bounds, double newVal) {
		bounds[0] = Math.min(bounds[0], newVal);
		bounds[1] = Math.max(bounds[1], newVal);
	}
	
	
	//regex-related
	private static final String rhs = "\\{([^}]*)\\}"; //NOTE: Contains a capture group
	private static final String sep = ", *";
	private static final String strn = "\"([^\"]+)\"";
	private static final String num = "([0-9]+)";
	private static final String numH = "((?:0x)?[0-9a-fA-F]+)";
	public static final Pattern LOG_LHS_REGEX = Pattern.compile("\\(" + strn + sep + num + sep + numH + sep  + rhs + "\\)");
	public static final Pattern LOG_RHS_REGEX = Pattern.compile(strn + ":" + strn + ",?");

}

