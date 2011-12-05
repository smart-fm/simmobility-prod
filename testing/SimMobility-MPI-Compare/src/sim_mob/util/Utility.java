package sim_mob.util;

import java.io.*;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Utility
{
	/**
	 * Helper method: load an image resource using the classpath. (This is
	 * required if we package the visualizer as a JAR, e.g., if we create an
	 * applet.)
	 * 
	 * @param path
	 *            The full path of the image to load. E.g.,
	 *            "res/icons/myicon.png"
	 * @return The Image resource as a BufferedImage. Null can technically be
	 *         returned; see ImageIO.read().
	 * @throws IOException
	 *             If the given file isn't found.
	 */

	// Parse an integer that might have an 0x in front.
	public static int ParseIntOptionalHex(String input)
	{
		int radix = 10;
		if (input.startsWith("0x"))
		{
			input = input.substring(2);
			radix = 0x10;
		}
		Integer temp = Integer.parseInt(input, radix);
		if (temp == 157065152)
		{
			System.out.println(input);
		}

		return Integer.parseInt(input, radix);
	}

	public static void CheckBounds(double[] bounds, double newVal)
	{
		bounds[0] = Math.min(bounds[0], newVal);
		bounds[1] = Math.max(bounds[1], newVal);
	}

	public static ArrayList<Integer> ParseLaneNodePos(String input)
	{
		ArrayList<Integer> pos = new ArrayList<Integer>();
		// System.out.println(input);
		Matcher m = NUM_REGEX.matcher(input);
		while (m.find())
		{
			pos.add(Integer.parseInt(m.group(1)));
		}

		if (pos.size() != 4)
		{
			System.out.println("Unexpected number of lane coordinates, should be 4 " + "now is  " + pos.size());
		}
		return pos;
	}

	public static Hashtable<String, String> ParseLogRHS(String rhs, String[] ensure) throws IOException
	{
		// Json-esque matching
		Hashtable<String, String> properties = new Hashtable<String, String>();
		Matcher m = Utility.LOG_RHS_REGEX.matcher(rhs);

		while (m.find())
		{
			if (m.groupCount() != 2)
			{
				throw new IOException("Unexpected group count (" + m.groupCount() + ") for: " + rhs);
			}

			String keyStr = m.group(1);
			String value = m.group(2);
			if (properties.containsKey(keyStr))
			{
				throw new IOException("Duplicate key: " + keyStr);
			}
			properties.put(keyStr, value);
		}

		// Now confirm
		for (String reqKey : ensure)
		{
			if (!properties.containsKey(reqKey))
			{
				if (!"xPos".equals(reqKey) && !"yPos".equals(reqKey) && !"fake".equals(reqKey))
					throw new IOException("Missing key: " + reqKey + " in: " + rhs);
			}
		}

		return properties;

	}

	public static double Distance(double x1, double y1, double x2, double y2)
	{
		double dx = x2 - x1;
		double dy = y2 - y1;
		return Math.sqrt(dx * dx + dy * dy);
	}

	// regex-related
	private static final String rhs = "\\{([^}]*)\\}"; // NOTE: Contains a
	// capture group
	private static final String sep = ", *";
	private static final String strn = "\"([^\"]+)\"";
	private static final String num = "([0-9]+)";
	private static final String numH = "((?:0x)?[0-9a-fA-F]+)";
	public static final Pattern LOG_LHS_REGEX = Pattern.compile("\\(" + strn + sep + num + sep + numH + sep + rhs
			+ "\\)");
	public static final Pattern LOG_RHS_REGEX = Pattern.compile(strn + ":" + strn + ",?");
	public static final Pattern NUM_REGEX = Pattern.compile(num);
}
