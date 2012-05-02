begin
  require 'simmob_classes'
  require 'mitsim_classes'
  require 'misc_compute'
  require 'parse_network'
  require 'parse_deps'
  require 'parse_vehs'
  require 'parse_traject'
  require 'parse_convert'
  require 'output_procs'
rescue LoadError => e
  puts "ERROR: #{e.message}"
  puts "If this file exists in the current directory, add this to your .bashrc file:"
  puts "  export RUBYLIB=\".\""
  puts "...but make sure you know the implications of this."
  exit
end

#Type of processing (set via the command line)
#true => Perform agent and network processing.
#false => Same, but also process the entire trajectory file.
$ProcessSimple = true

#Workaround
$FlipRight = true
$LaneWidthMS = 3.6 #Lane width in Mitsim
$LaneWidthSM = 3.5 #Lane width in Sim Mobility (for estimated lanes)


#TODO: We can overcome some of these checks by writing our own <simulation> tag. 
#      This is not an issue now; might want to clean it up later.
def final_validate(network, drivers, minDepTime)
  drivers.each{|id, dr|
    #Make sure sim mobility node IDs exist
    raise "No Sim Mobility node id for: #{dr.originNode}" unless dr.originNode.sm_node
    raise "No Sim Mobility node id for: #{dr.destNode}" unless dr.destNode.sm_node

    #Start our departure times at zero, and convert to ms (then int)
    dr.departure =  ((dr.departure - minDepTime)*1000).to_i
  }
end


def run_main()
  #Simple
  if ARGV.length < 1
    puts 'Please specify a command line argument:'
    puts '  basic => Process all Nodes and Agent starting positions'
    puts '  all => Same as basic, but also read the entire trajectory file and'
    puts '         generate a trace (this can take some time).'
    puts 'Example:'
    puts '  ruby  process.rb  basic'
    exit
  end
  if (ARGV[0] == 'basic')
    $ProcessSimple = true
  elsif (ARGV[0] == 'all')
    $ProcessSimple = false
  else
    raise "Error, unknown argument: #{ARGV[0]}"
  end

  #Internal data structures
  network = Mitsim::RoadNetwork.new()
  network.sm_network = SimMob::RoadNetwork.new()
  drivers = {}
  time_ticks = {} #<tick_id> => {<driver_id>=>driver_tick, <driver_id>=>driver_tick,...]

  #Read the network file
  MS_NetworkParser.read_network_file('network-BUGIS.dat', network)

  #Build up a list of all Driver IDs
  min, max = MS_DepartureParser.read_dep_file('dep.out', network, drivers)

  #Cross-reference with the vehicles file
  minDepTime = MS_VehicleParser.read_veh_file('vehicle.out', drivers)

  #Compare with sim mobility nodes
  MS_ConvertParser.read_convert_file('ms_sm_node_convert.txt', network, drivers)

  #Parse the trajectory file
  #We use a trimmed version of trajectory.out, but loading the original would work too.
  #If we are asked to perform a comprehensive analysis, read in the (entire) trajectory file.
  puts 'Parsing trajectory file (this may take a while).'
  if $ProcessSimple
    MS_TrajectoryParser.read_traj_file('traj_compact.txt', network, time_ticks, drivers)
  else 
    MS_TrajectoryParser.read_traj_file('trajectory.out', network, time_ticks, drivers)
  end
  puts 'Trajectory file done.'

  #Final validation
  final_validate(network, drivers, minDepTime)

  #Print 
  puts 'Saving output files (this may take a while).'
  Output.print_network(network, time_ticks)
  Output.print_agents(network, time_ticks, drivers, min, max)
  puts 'Done saving output.'

end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




