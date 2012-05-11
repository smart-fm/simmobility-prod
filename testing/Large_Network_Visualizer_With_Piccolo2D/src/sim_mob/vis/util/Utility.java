package sim_mob.vis.util;

import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.io.*;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.imageio.ImageIO;
import sim_mob.vis.Main;
import sim_mob.vis.network.Node;

public class Utility {

	public static BufferedImage LoadImgResource(String path) throws IOException {
		InputStream input = Main.class.getClassLoader().getResourceAsStream(path);
		if (input==null) {
			throw new IOException("Could not find image resource: " + path);
		}
		return ImageIO.read(input);
	}
	
	public static BufferedReader LoadFileResource(String path) throws IOException {
		InputStream input = Main.class.getClassLoader().getResourceAsStream(path);
		if (input==null) {
			throw new IOException("Could not find file resource: " + path);
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
	
	public static ArrayList<Integer> ParseLaneNodePos(String input){
		ArrayList<Integer> pos = new ArrayList<Integer>();
		Matcher m = NUM_REGEX.matcher(input);
		while(m.find()){	
			pos.add(Integer.parseInt(m.group(1)));
		}
		
		if(pos.size()!=4){
			System.out.println("Unexpected number of lane coordinates, should be 4 " + "now is  " + pos.size());
		}
		return pos;
	}
	public static ArrayList<Integer> ParseLinkPaths(String input){
		ArrayList<Integer> pos = new ArrayList<Integer>();
		//System.out.println(input);
		Matcher m = NUMH_REGEX.matcher(input);
		while(m.find()){	
			pos.add(ParseIntOptionalHex(m.group(1)));
		}

		return pos;
	}
	public static Node ParseCrossingNodePos(String input)throws IOException{
		
		String[] items = input.split(",");
		
		if(items.length!=2){
			throw new IOException("Error! Unexpected input information in ParseCrossingNodePos()");
		}
		Double xPos = Double.parseDouble(items[0]);
		Double yPos = Double.parseDouble(items[1]);
		
		Node tempVirtualNode = new Node(xPos,yPos,false,null); 
		return tempVirtualNode;
	}
	
	public static Hashtable<String, String> ParseLogRHS(String rhs, String[] ensure) throws IOException {
		//Json-esque matching
		Hashtable<String, String> properties = new Hashtable<String, String>();
		Matcher m = Utility.LOG_RHS_REGEX.matcher(rhs);
		while (m.find()) {
			if (m.groupCount()!=2) {
				throw new IOException("Unexpected group count (" + m.groupCount() + ") for: " + rhs);
			}
			
			String keyStr = m.group(1);
			String value = m.group(2);
			if (properties.containsKey(keyStr)) {
				throw new IOException("Duplicate key: " + keyStr);
			}
			properties.put(keyStr, value);
		}
		
		//Now confirm
		for (String reqKey : ensure) {
			if (!properties.containsKey(reqKey)) {
				throw new IOException("Missing key: " + reqKey + " in: " + rhs);
			}
		}
		
		return properties;
		
	}
	
	public static double Distance(double x1, double y1, double x2, double y2) { 
		double dx   = x2 - x1;
		double dy   = y2 - y1;
		return Math.sqrt(dx*dx + dy*dy);
	}
	public static double Distance(Point2D from, Point2D to) {
		return Distance(from.getX(), from.getY(), to.getX(), to.getY());
	}
	
	public static final String Point2Str(Point2D pt) {
		return "(" + pt.getX() + "," + pt.getY() + ")";
	}

	
	//regex-related
	private static final String rhs = "\\{([^}]*)\\}"; 
	private static final String sep = ", *";
	private static final String strn = "\"([^\"]+)\"";
	private static final String num = "([0-9]+)";
	private static final String numH = "((?:0x)?[0-9a-fA-F]+)";
	public static final Pattern LOG_LHS_REGEX = Pattern.compile("\\(" + strn + sep + num + sep + numH + sep  + rhs + "\\)");
	public static final Pattern LOG_RHS_REGEX = Pattern.compile(strn + ":" + strn + ",?");
	public static final Pattern NUM_REGEX = Pattern.compile(num);
	public static final Pattern NUMH_REGEX = Pattern.compile(numH);
	public static final Pattern LANE_SIDEWALK_REGEX = Pattern.compile("line-"+num+"-?is-sidewalk");
	public static final Pattern LANE_LINE_REGEX = Pattern.compile("line-"+num);

}

