package simulation_results;

import java.io.BufferedReader;
import java.io.IOException;
import java.util.Hashtable;
import java.util.regex.Matcher;
import sim_mob.vis.util.Utility;

public class SimulationExtractor
{
	/**
	 * Extract
	 */
	public static Simulation_Result getResultsFromOneFile(BufferedReader[] one_pc_file_source) throws IOException
	{
		Simulation_Result results = new Simulation_Result();

		for (int count = 0; count < one_pc_file_source.length; count++)
		{
			getResultsFromOneFile(one_pc_file_source[count], results);
		}
		
		return results;
	}

	private static Simulation_Result getResultsFromOneFile(BufferedReader one_pc_file_source, Simulation_Result results)
			throws IOException
	{
		// Read
		String line;
		while ((line = one_pc_file_source.readLine()) != null)
		{

			line = line.trim();
			if (line.isEmpty() || !line.startsWith("(") || !line.endsWith(")"))
			{
				continue;
			}

			Matcher m = Utility.LOG_LHS_REGEX.matcher(line);
			if (!m.matches())
			{
				throw new IOException("Invalid line: " + line);
			}
			if (m.groupCount() != 4)
			{
				throw new IOException("Unexpected group count (" + m.groupCount() + ") for: " + line);
			}

			// Known fields: type, id, rhs
			String type = m.group(1);
			int frameID = Integer.parseInt(m.group(2));
			int objID = Utility.ParseIntOptionalHex(m.group(3));
			String rhs = m.group(4);

			if (frameID > results.time_steps)
				results.time_steps = frameID;

			try
			{
				dispatchConstructionRequest(type, frameID, objID, rhs, results);
			}
			catch (IOException ex)
			{
				throw new IOException(ex.getMessage() + "\n...on line: " + line);
			}
		}

		return null;
	}

	public static Simulation_Result getResultsFromFiles(BufferedReader[] many_pc_file_source) throws IOException
	{
		Simulation_Result results = new Simulation_Result();

		for (int count = 0; count < many_pc_file_source.length; count++)
		{
			getResultsFromOneFile(many_pc_file_source[count], results);
		}

		return results;
	}

	private static void dispatchConstructionRequest(String objType, int frameID, int objID, String rhs,
			Simulation_Result results) throws IOException
	{
		if (objType.equals("Driver"))
		{
			parseDriver(frameID, objID, rhs, results);
		}
		else if (objType.equals("Signal"))
		{
			parseSignal(frameID, objID, rhs, results);
		}
		else if (objType.equals("pedestrian") || objType.equals("Pedestrian"))
		{
			parsePedestrian(frameID, objID, rhs, results);
		}
	}

	private static void parseDriver(int frameID, int objID, String rhs, Simulation_Result results) throws IOException
	{
		Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[] { "xPos", "yPos", "angle", "fake" });

		String isfake = props.get("fake");
		if ("true".equals(isfake))
			return;

		Driver one_driver = new Driver();
		one_driver.setId(objID);

		one_driver.setX_pos(Long.parseLong(props.get("xPos")));
		one_driver.setY_pos(Long.parseLong(props.get("yPos")));

		TimeTick one_time_tick = results.time_ticks.get(frameID);
		if (one_time_tick == null)
		{
			one_time_tick = new TimeTick();
			results.time_ticks.put(frameID, one_time_tick);
		}
		one_time_tick.drivers.add(one_driver);
	}

	private static void parseSignal(int frameID, int objID, String rhs, Simulation_Result results) throws IOException
	{
		Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[] { "va", "vb", "vc", "vd", "pa", "pb",
				"pc", "pd", "xPos", "yPos", "fake" });

		String isfake = props.get("fake");
		if ("true".equals(isfake))
			return;

		Signal one_signal = new Signal();
		one_signal.setId(objID);

		one_signal.setVa(props.get("va"));
		one_signal.setVb(props.get("vb"));
		one_signal.setVc(props.get("vc"));
		one_signal.setVd(props.get("vd"));
		one_signal.setPa(props.get("pa"));
		one_signal.setPb(props.get("pb"));
		one_signal.setPc(props.get("pc"));
		one_signal.setPd(props.get("pd"));
		one_signal.setxPos(Long.parseLong(props.get("xPos")));
		one_signal.setyPos(Long.parseLong(props.get("yPos")));

		TimeTick one_time_tick = results.time_ticks.get(frameID);
		if (one_time_tick == null)
		{
			one_time_tick = new TimeTick();
			results.time_ticks.put(frameID, one_time_tick);
		}

		one_time_tick.signals.add(one_signal);
	}

	private static void parsePedestrian(int frameID, int objID, String rhs, Simulation_Result results)
			throws IOException
	{
		Hashtable<String, String> props = Utility.ParseLogRHS(rhs, new String[] { "xPos", "yPos", "fake" });

		String isfake = props.get("fake");
		if ("true".equals(isfake))
			return;

		Pedestrian one_pedestrian = new Pedestrian();
		one_pedestrian.setId(objID);

		one_pedestrian.setX_pos(Long.parseLong(props.get("xPos")));
		one_pedestrian.setY_pos(Long.parseLong(props.get("yPos")));

		TimeTick one_time_tick = results.time_ticks.get(frameID);
		if (one_time_tick == null)
		{
			one_time_tick = new TimeTick();
			results.time_ticks.put(frameID, one_time_tick);
		}

		one_time_tick.pedestrians.add(one_pedestrian);
	}

}