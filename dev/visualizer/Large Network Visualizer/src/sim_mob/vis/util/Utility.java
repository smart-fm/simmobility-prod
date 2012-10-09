package sim_mob.vis.util;

import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.*;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.imageio.ImageIO;

import com.google.gson.Gson;

import sim_mob.vis.Main;
import sim_mob.vis.network.basic.FlippedScaledPoint;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.simultion.*;
import sim_mob.vis.network.*;

/**
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Matthew Bremer Bruchon
 */
public class Utility {
	//Use the new parser for log files?
	private static final boolean USE_NEW_PARSER = true;
	
	//NOTE: The fields in this class are VERY important; make sure you add a single field 
	//      for each object you want to be able to parse with the Gson parser. 
	//      Do NOT add any extra fields to this class; we use reflection to scan all fields
	//      and return ONLY the one which is valid.
	@SuppressWarnings("unused") //We need this to avoid throwing out our parameters.
	private final class GsonWrapper {
		//NOTE: These names are mildly confusing, but just follow the pattern for new fields
		//      and everything will work.		
		TrafficSignal TrafficSignal;
		TrafficSignalUpdate TrafficSignalUpdate;

		GsonResObj getResult() {
			ArrayList<GsonResObj> possible = new ArrayList<GsonResObj>();
			Class<GsonWrapper> c = GsonWrapper.class;
			for (Field f : c.getDeclaredFields()) {
				GsonResObj gRes = null;
				try {
					gRes = GsonResObj.class.cast(f.get(this));
				} catch (ClassCastException cex) { 
					continue; 
				} catch (IllegalAccessException iex) { 
					throw new RuntimeException(iex); }
				
				if (gRes==null) { continue; }
				
				possible.add(gRes);
			}
			
			if (possible.size()!=1) {
				throw new ClassCastException("GsonWrapper expected to have one field, not " + possible.size());
			}
			return possible.get(0);
		}
	}
	
	
	
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
	public static long ParseLongOptionalHex(String input) {
		int radix = 10;
		if (input.startsWith("0x")) {
			input = input.substring(2);
			radix = 0x10;
		}
	
		
		return Long.parseLong(input, radix);
	}
	
	
	/*public static void CheckBounds(double[] bounds, double newVal) {
		bounds[0] = Math.min(bounds[0], newVal);
		bounds[1] = Math.max(bounds[1], newVal);
	}*/
	
	public static ArrayList<Integer> ParseLaneNodePos(String input){
		ArrayList<Integer> pos = new ArrayList<Integer>();
		Matcher m = NUM_REGEX.matcher(input);
		while(m.find()){	
			pos.add(Integer.parseInt(m.group(1)));
		}
		
		if(pos.size()!=4){
			System.out.println("Unexpected number of lane coordinates, should be 4 " + "now is  " + pos.size());
		}
		
		//System.out.println(" [(" + pos.get(0) + "," + pos.get(1) + "),(" + pos.get(2) + "," + pos.get(3) + "),]");
		
		return pos;
	}
	public static ScaledPoint ParseCrossingNodePos(String input)throws IOException{
		
		String[] items = input.split(",");
		
		if(items.length!=2){
			throw new IOException("Error! Unexpected input information in ParseCrossingNodePos()");
		}
		Double xPos = Double.parseDouble(items[0]);
		Double yPos = Double.parseDouble(items[1]);
		
		return new FlippedScaledPoint(xPos, yPos);
		
		//Node tempNode = new Node(xPos,yPos,false,null); 
		//return tempNode;
	}
	
	
	private static Hashtable<String, String> ParseLogRHS(String rhs) {
		//Json-esque matching
		Hashtable<String, String> properties = new Hashtable<String, String>();
		Matcher m = Utility.LOG_RHS_REGEX.matcher(rhs);
		while (m.find()) {
			if (m.groupCount()!=2) {
				properties.put("@@EXCEPTION@@", "Unexpected group count (" + m.groupCount() + ") for: " + rhs);
				return properties;
			}
			
			String keyStr = m.group(1);
			String value = m.group(2);
			if (properties.containsKey(keyStr)) {
				properties.put("@@EXCEPTION@@", "Duplicate key: " + keyStr);
				return properties;
			}
			properties.put(keyStr, value);
		}
		return properties;
	}
	
	public static ArrayList<Long> ParseLinkPaths(String input){
		ArrayList<Long> pos = new ArrayList<Long>();
		//System.out.println(input);
		Matcher m = NUMH_REGEX.matcher(input);
		while(m.find()){	
			pos.add(ParseLongOptionalHex(m.group(1)));
		}

		return pos;
	}

	
	
	public static final String printRect(Rectangle2D rect) {
		return rect.getX() + "," + rect.getY() + " => " + rect.getWidth() + "," + rect.getHeight();
	}
	
	
	//Haven't tested downscaling, scale to zero, etc.
	public static void resizeRectangle(Rectangle2D rect, double newWidth, double newHeight) {
		rect.setRect(
			rect.getCenterX() - newWidth/2,
			rect.getCenterY() - newHeight/2,
			newWidth, newHeight);
	}

	
	public static final double Distance(double x1, double y1, double x2, double y2) {
		double dX = x1-x2;
		double dY = y1-y2;
		return Math.sqrt(dX*dX + dY*dY);
	}
	public static final double Distance(Point2D start, Point2D end) {
		return Distance(start.getX(), start.getY(), end.getX(), end.getY());
	}
	
	public static class ParseResults {
		public String type;
		public int frame;
		public long objID;
		public Hashtable<String, String> properties = new Hashtable<String, String>();
		
		//Confirm
		public boolean confirmProps(String[] required) {
			for (String key : required) {
				if (!properties.containsKey(key)) {
					return false;
				}
			}
			return true;
		}
		
		//Error handling
		public String errorMsg;
		public boolean isError() { return (errorMsg!=null); }
	}
	
	
	
	private static ParseResults ParseLogLine_Old(String line) {
		ParseResults res = new ParseResults();
		
		//Parse basic
	    Matcher m = Utility.LOG_LHS_REGEX.matcher(line);
	    if (!m.matches()) {
	    	res.errorMsg = "Invalid line: " + line;
	    	return res;
	    }
	    if (m.groupCount()!=4) {
	    	res.errorMsg = "Unexpected group count (" + m.groupCount() + ") for: " + line;
	    	return res;
	    }
	    
	    //Retrieve known fields: type, id, rhs
	    res.type = m.group(1);
	    res.frame = Integer.parseInt(m.group(2));
	    res.objID = Utility.ParseLongOptionalHex(m.group(3));
	    
	    //Parse RHS
	    res.properties = Utility.ParseLogRHS(m.group(4));
	    
	    //TODO: Better way of propagating errors.
	    res.errorMsg = res.properties.get("@@EXCEPTION@@");
	    return res;
	}
	
	
	
	public static ParseResults ParseLogLine(FastLineParser flp, String line) {		
		if (USE_NEW_PARSER) {
			return flp.getResults(line);
		} else {
			return ParseLogLine_Old(line);
		}
	}

	
	/*private static Gson getGsonReader() {
		RuntimeTypeAdapter<GsonResObj> rtta = RuntimeTypeAdapterFactory.of(GsonResObj.class);
		rtta.registerSubtype(TrafficSignal.class);
		rtta.registerSubtype(TrafficSignalUpdate.class);
		
		GsonBuilder gsb = new GsonBuilder();
		gsb.registerTypeAdapter(GsonResObj.class, rtta);
		return gsb.create();
	}*/
	
	
	public static GsonResObj ParseGsonLine(String line) {
		//Couldn't be easier!
		Gson gson = new Gson();
		GsonResObj res = gson.fromJson(line, GsonWrapper.class).getResult();
		return res;
	}
	
	
	//regex-related
	private static final String rhs = "\\{([^}]*)\\}"; //NOTE: Contains a capture group
	private static final String sep = ", *";
	private static final String strn = "\"([^\"]*)\"";
	private static final String num = "([0-9]+)";
	private static final String numH = "((?:0x)?[0-9a-fA-F]+)";
	private static final Pattern LOG_LHS_REGEX = Pattern.compile("\\(" + strn + sep + num + sep + numH + sep  + rhs + "\\)");
	private static final Pattern LOG_RHS_REGEX = Pattern.compile(strn + ":" + strn + ",?");
	public static final Pattern NUM_REGEX = Pattern.compile(num);
	private static final Pattern NUMH_REGEX = Pattern.compile(numH);
}

