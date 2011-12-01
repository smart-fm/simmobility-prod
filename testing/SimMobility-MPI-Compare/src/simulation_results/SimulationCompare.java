package simulation_results;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

public class SimulationCompare
{

	private static Simulation_Result buffer_one_result;
	private static Simulation_Result buffer_the_other_result;

	/**
	 * Compare
	 */
	private static HashMap<Integer, Integer> map_driver_ids;
	private static HashMap<Driver, Driver> map_drivers;

	private static HashMap<Integer, Integer> map_pedestrian_ids;
	private static HashMap<Pedestrian, Pedestrian> map_pedestrians;

	private static HashMap<Integer, Integer> map_signal_ids;
	private static HashMap<Signal, Signal> map_signals;

	/**
	 * Compare Count
	 */
	private static long driver_mapping_count;
	private static long driver_miss_mapping_count;
	private static long driver_count;
	private static double driver_miss_distance_sum;

	private static long pedestrian_mapping_count;
	private static long pedestrian_miss_mapping_count;
	private static long pedestrian_count;
	private static double pedestrian_miss_distance_sum;

	private static long signal_mapping_count;
	private static long signal_miss_mapping_count;
	private static long signal_count;

	public static void compareSimulationResults(Simulation_Result one_result, Simulation_Result the_other_result)
			throws IOException
	{
		if (one_result.time_steps != the_other_result.time_steps)
		{
			System.out.println("Error: The simulation time steps are not the same");
			throw new IOException("Error: The simulation time steps are not the same");
		}

		buffer_one_result = one_result;
		buffer_the_other_result = the_other_result;

		initStaticStructure();
		buildMappingEntity();

//		HashMap<Integer, Integer> one = map_driver_ids;
//		HashMap<Integer, Integer> two = map_pedestrian_ids;
//		HashMap<Integer, Integer> three = map_signal_ids;

		compareMappingEntity();

		calculateErrorRate();
	}

	private static void initStaticStructure()
	{
		map_driver_ids = new HashMap<Integer, Integer>();
		map_drivers = new HashMap<Driver, Driver>();

		map_pedestrian_ids = new HashMap<Integer, Integer>();
		map_pedestrians = new HashMap<Pedestrian, Pedestrian>();

		map_signal_ids = new HashMap<Integer, Integer>();
		map_signals = new HashMap<Signal, Signal>();

		driver_mapping_count = 0;
		driver_miss_mapping_count = 0;
		driver_miss_distance_sum = 0;
		driver_count = 0;

		pedestrian_mapping_count = 0;
		pedestrian_miss_mapping_count = 0;
		pedestrian_miss_distance_sum = 0;
		pedestrian_count = 0;

		signal_mapping_count = 0;
		signal_miss_mapping_count = 0;
		signal_count = 0;
	}

	private static void buildMappingEntity()
	{
		for (int count = 0; count <= buffer_one_result.time_steps; count++)
		{
			TimeTick one_stick = buffer_one_result.time_ticks.get(count);
			TimeTick the_other_stick = buffer_the_other_result.time_ticks.get(count);

			buildMaapingAtOneTimeStep(one_stick, the_other_stick);
		}
	}

	private static void buildMaapingAtOneTimeStep(TimeTick one_stick, TimeTick the_other_stick)
	{
		Iterator itr_driver = one_stick.drivers.iterator();
		while (itr_driver.hasNext())
		{
			Driver one_driver = (Driver) itr_driver.next();
			buildDriverMaapingAtOneTimeStep(one_driver, the_other_stick.drivers);
		}

		Iterator itr_pedestrian = one_stick.pedestrians.iterator();
		while (itr_pedestrian.hasNext())
		{
			Pedestrian one_pedestrian = (Pedestrian) itr_pedestrian.next();
			buildPedestrianMaapingAtOneTimeStep(one_pedestrian, the_other_stick.pedestrians);
		}

		Iterator itr_signal = one_stick.signals.iterator();
		while (itr_signal.hasNext())
		{
			Signal one_signal = (Signal) itr_signal.next();
			buildSignalMaapingAtOneTimeStep(one_signal, the_other_stick.signals);
		}
	}

	/*
	 * Firstly, check whether ID has been mapped; If so, use the mapped id to
	 * find the mapping Object; If not, find the mapping object using attributes
	 * and then add the mapping id list;
	 */
	private static void buildDriverMaapingAtOneTimeStep(Driver one_driver, HashSet<Driver> drivers)
	{

		int driver_id = one_driver.getId();
		if (map_driver_ids.containsKey(driver_id))
		{
			int mapping_driver_id = map_driver_ids.get(driver_id);
			Driver the_other_driver = getMappingDriver(mapping_driver_id, drivers);

			map_drivers.put(one_driver, the_other_driver);
		}
		else
		{
			Driver the_other_driver = getMappingDriver(one_driver, drivers);
			map_drivers.put(one_driver, the_other_driver);

			if (the_other_driver != null)
				map_driver_ids.put(driver_id, the_other_driver.getId());
		}
	}

	private static void buildPedestrianMaapingAtOneTimeStep(Pedestrian one_pedestrian, HashSet<Pedestrian> pedestrians)
	{
		int pedestrian_id = one_pedestrian.getId();
		if (map_pedestrian_ids.containsKey(pedestrian_id))
		{
			int mapping_pedestrian_id = map_pedestrian_ids.get(pedestrian_id);
			Pedestrian the_other_pedestrian = getMappingPedestrian(mapping_pedestrian_id, pedestrians);

			map_pedestrians.put(one_pedestrian, the_other_pedestrian);
		}
		else
		{
			Pedestrian the_other_pedestrian = getMappingPedestrian(one_pedestrian, pedestrians);
			map_pedestrians.put(one_pedestrian, the_other_pedestrian);

			if (the_other_pedestrian != null)
				map_pedestrian_ids.put(pedestrian_id, the_other_pedestrian.getId());
		}
	}

	private static void buildSignalMaapingAtOneTimeStep(Signal one_signal, HashSet<Signal> signals)
	{
		int signal_id = one_signal.getId();
		if (map_signal_ids.containsKey(signal_id))
		{
			int mapping_signal_id = map_signal_ids.get(signal_id);
			Signal the_other_signal = getMappingSignal(mapping_signal_id, signals);

			map_signals.put(one_signal, the_other_signal);
		}
		else
		{
			Signal the_other_signal = getMappingSignal(one_signal, signals);
			map_signals.put(one_signal, the_other_signal);

			if (the_other_signal != null)
				map_signal_ids.put(signal_id, the_other_signal.getId());
		}
	}

	private static Driver getMappingDriver(int one_driver_id, HashSet<Driver> drivers)
	{
		Iterator itr = drivers.iterator();
		while (itr.hasNext())
		{
			Driver one = (Driver) itr.next();
			if (one.getId() == one_driver_id)
				return one;
		}

		return null;
	}

	private static Driver getMappingDriver(Driver one_driver, HashSet<Driver> drivers)
	{
		Iterator itr = drivers.iterator();
		while (itr.hasNext())
		{
			Driver one = (Driver) itr.next();
			if (one.getX_pos() == one_driver.getX_pos() && one.getY_pos() == one_driver.getY_pos())
				return one;
		}

		return null;
	}

	private static Pedestrian getMappingPedestrian(int one_pedestrian_id, HashSet<Pedestrian> pedestrians)
	{
		Iterator itr = pedestrians.iterator();
		while (itr.hasNext())
		{
			Pedestrian one = (Pedestrian) itr.next();
			if (one.getId() == one_pedestrian_id)
				return one;
		}

		return null;
	}

	private static Pedestrian getMappingPedestrian(Pedestrian one_pedestrian, HashSet<Pedestrian> pedestrians)
	{
		Iterator itr = pedestrians.iterator();
		while (itr.hasNext())
		{
			Pedestrian one = (Pedestrian) itr.next();
			if (one.getX_pos() == one_pedestrian.getX_pos() && one.getY_pos() == one_pedestrian.getY_pos())
				return one;
		}

		return null;
	}

	private static Signal getMappingSignal(int one_signal_id, HashSet<Signal> signals)
	{
		Iterator itr = signals.iterator();
		while (itr.hasNext())
		{
			Signal one = (Signal) itr.next();
			if (one.getId() == one_signal_id)
				return one;
		}

		return null;
	}

	private static Signal getMappingSignal(Signal one_signal, HashSet<Signal> signals)
	{
		Iterator itr = signals.iterator();
		while (itr.hasNext())
		{
			Signal one = (Signal) itr.next();
			if (one.getxPos() == one_signal.getxPos() && one.getyPos() == one_signal.getyPos())
				return one;
		}

		return null;
	}

	private static void compareMappingEntity()
	{
		Iterator itr_drivers = map_drivers.keySet().iterator();
		while (itr_drivers.hasNext())
		{
			Driver one_driver = (Driver) itr_drivers.next();
			Driver the_other_driver = map_drivers.get(one_driver);

			compareOnevehicle(one_driver, the_other_driver);
		}

		Iterator itr_pedestrian = map_pedestrians.keySet().iterator();
		while (itr_pedestrian.hasNext())
		{
			Pedestrian one_pedestrian = (Pedestrian) itr_pedestrian.next();
			Pedestrian the_other_pedestrian = map_pedestrians.get(one_pedestrian);

			compareOnePedestrian(one_pedestrian, the_other_pedestrian);
		}

		int i = 0;
		
		Iterator itr_signal = map_signals.keySet().iterator();
		while (itr_signal.hasNext())
		{
			Signal one_signal = (Signal) itr_signal.next();
			Signal the_other_signal = map_signals.get(one_signal);

			compareOneSignal(one_signal, the_other_signal);
		}
	}

	private static void compareOnevehicle(Driver one_driver, Driver the_other_driver)
	{
		driver_count++;

		if (the_other_driver == null)
		{
			driver_miss_mapping_count++;
			return;
		}

		if (one_driver.getX_pos() != the_other_driver.getX_pos()
				|| one_driver.getY_pos() != the_other_driver.getY_pos())
		{
			driver_miss_mapping_count++;

			double from_x = one_driver.getX_pos();
			double from_y = one_driver.getY_pos();
			double to_x = the_other_driver.getX_pos();
			double to_y = the_other_driver.getY_pos();

			double distance = distance(from_x, from_y, to_x, to_y);
			driver_miss_distance_sum += distance;

			return;
		}

		driver_mapping_count++;
	}

	private static void compareOnePedestrian(Pedestrian one_pedestrian, Pedestrian the_other_pedestrian)
	{
		pedestrian_count++;

		if (the_other_pedestrian == null)
		{
			pedestrian_miss_mapping_count++;
			return;
		}

		if (one_pedestrian.getX_pos() != the_other_pedestrian.getX_pos()
				|| one_pedestrian.getY_pos() != the_other_pedestrian.getY_pos())
		{
			pedestrian_miss_mapping_count++;

			double from_x = one_pedestrian.getX_pos();
			double from_y = one_pedestrian.getY_pos();
			double to_x = the_other_pedestrian.getX_pos();
			double to_y = the_other_pedestrian.getY_pos();

			double distance = distance(from_x, from_y, to_x, to_y);
			pedestrian_miss_distance_sum += distance;

			return;
		}

		pedestrian_mapping_count++;
	}

	private static void compareOneSignal(Signal one_signal, Signal the_other_signal)
	{
		signal_count++;

		if (the_other_signal == null)
		{
			signal_miss_mapping_count++;
			return;
		}

		if (!one_signal.getPa().equals(the_other_signal.getPa())
				|| !one_signal.getPb().equals(the_other_signal.getPb())
				|| !one_signal.getPc().equals(the_other_signal.getPc())
				|| !one_signal.getPd().equals(the_other_signal.getPd()))
		{
			signal_miss_mapping_count++;
			return;
		}

		if (!one_signal.getVa().equals(the_other_signal.getVa())
				|| !one_signal.getVb().equals(the_other_signal.getVb())
				|| !one_signal.getVc().equals(the_other_signal.getVc())
				|| !one_signal.getVd().equals(the_other_signal.getVd()))
		{
			signal_miss_mapping_count++;
			return;
		}

		signal_mapping_count++;
	}

	private static double distance(double from_point_x, double from_point_y, double to_point_x, double to_point_y)
	{
		double x_dis = from_point_x - to_point_x;
		double y_dis = from_point_y - to_point_y;

		return Math.sqrt(x_dis * x_dis + y_dis * y_dis);
	}

	private static void calculateErrorRate()
	{
		System.out.println("Error Ratio Results:");
		System.out.println("----------------------------------------");
		System.out.println("Total Simulation Time Step:" + buffer_one_result.time_steps);
		System.out.println("----------------------------------------");
		System.out.println("Divers:");
		System.out.println("Divers Count:" + driver_count);
		System.out.format("Divers Error Ratio: (%d/%d)%.3f(precent)\n", driver_miss_mapping_count, driver_count,
				driver_miss_mapping_count * 100.0 / driver_count);
		System.out.format("Divers Average Error Distance: %.2f (cm)\n", driver_miss_distance_sum * 1.0
				/ driver_count);

		System.out.println("----------------------------------------");
		System.out.println("Pedestrians:");
		System.out.println("Pedestrians Count:" + pedestrian_count);
		System.out.format("Pedestrians Error Ratio: (%d/%d)%.3f(precent)\n", pedestrian_miss_mapping_count,
				pedestrian_count, pedestrian_miss_mapping_count * 100.0 / pedestrian_count);
		System.out.format("Pedestrians Average Error Distance: %.2f (cm)\n", pedestrian_miss_distance_sum * 1.0
				/ pedestrian_count);

		System.out.println("----------------------------------------");
		System.out.println("Signals:");
		System.out.println("Signals Count:" + signal_count);
		System.out.format("Signals Error Ratio: (%d/%d)%.2f(precent)\n", signal_miss_mapping_count, signal_count,
				signal_miss_mapping_count * 100.0 / signal_count);
	}
}
