package main;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import simulation_results.SimulationCompare;
import simulation_results.SimulationExtractor;
import simulation_results.Simulation_Result;

public class SimMobility_Compare
{
	private String one_pc_results_folder_str = "src//simmobility_results_A";
	// private String one_pc_results_folder_str =
	// "src//simmobility_results_others";
	private String many_pc_results_folder_str = "src//simmobility_results_B";
	// private String many_pc_results_folder_str =
	// "src//simmobility_results_others_";
	
	private BufferedReader[] one_pc_file_source;
	private BufferedReader[] many_pc_file_source;
	
	private Simulation_Result one_pc_results;
	private Simulation_Result many_pc_results;

	public static void main(String[] args) throws IOException
	{
		SimMobility_Compare start = new SimMobility_Compare();
		start.loadinOnePCResultSource();
		start.loadinManyPCResultSource();
		start.extractOnePCResult();
		start.extractManyPCResult();
		start.compareResults();
	}

	// only load in the first file
	private void loadinOnePCResultSource() throws IOException
	{
		File one_pc_results_folder = new File(one_pc_results_folder_str);
		File files[] = one_pc_results_folder.listFiles();

		if (files.length > 0)
		{
			int file_size = files.length;
			one_pc_file_source = new BufferedReader[file_size];

			for (int count = 0; count < file_size; count++)
			{
				one_pc_file_source[count] = new BufferedReader(new FileReader(files[count]));
			}
		}
		else
			throw new IOException("No SimMobility results in folder:" + one_pc_results_folder_str);
	}

	// load in files inside
	private void loadinManyPCResultSource() throws IOException
	{
		File one_pc_results_folder = new File(many_pc_results_folder_str);
		File files[] = one_pc_results_folder.listFiles();

		if (files.length > 0)
		{
			int file_size = files.length;
			many_pc_file_source = new BufferedReader[file_size];

			for (int count = 0; count < file_size; count++)
			{
				many_pc_file_source[count] = new BufferedReader(new FileReader(files[count]));
			}
		}
		else
			throw new IOException("No SimMobility-MPI results in folder:" + many_pc_results_folder_str);
	}

	private void extractOnePCResult() throws IOException
	{
		one_pc_results = SimulationExtractor.getResultsFromOneFile(one_pc_file_source);
	}

	private void extractManyPCResult() throws IOException
	{
		many_pc_results = SimulationExtractor.getResultsFromFiles(many_pc_file_source);
	}

	private void compareResults() throws IOException
	{
		SimulationCompare.compareSimulationResults(one_pc_results, many_pc_results);
	}
}
