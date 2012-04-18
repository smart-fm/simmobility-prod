require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'
require 'parse_network'
require 'parse_deps'
require 'parse_vehs'
require 'parse_traject'
require 'parse_convert'
require 'output_procs'


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
  #Internal data structures
  network = Mitsim::RoadNetwork.new()
  network.sm_network = SimMob::RoadNetwork.new()
  drivers = {}

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
  MS_TrajectoryParser.read_traj_file('traj_compact.txt', network, drivers)

  #Final validation
  final_validate(network, drivers, minDepTime)

  #Print 
  Output.print_network(network)
  Output.print_agents(network, drivers, min, max)

end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




