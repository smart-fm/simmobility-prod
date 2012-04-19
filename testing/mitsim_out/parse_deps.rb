require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'


#The dep.out file contains the departure record of each vehicle in the simulation.
#This comprises of:
#  departure time (s)
#  agent (vehicle) ID
#  originNode
#  destNode
#  vehicleType
#  path (but ONLY if the vehicles have a path)


module MS_DepartureParser



#Main method: parse a dep file.
def self.read_dep_file(depFileName, nw, drivers)
  min = max = nil
  File.open(depFileName).each { |line|
    if line =~ /([0-9.]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+)/
      dr = Mitsim::Driver.new($2.to_i)
      dr.departure = $1.to_f
      dr.originNode = $3
      dr.destNode = $4
      dr.vehicleType = $5.to_i

      #Make sure these nodes exist in mitsim, then substitute them in place of the IDs
      raise "Origin Node doesn't exist: #{dr.originNode}" unless nw.nodes.has_key? dr.originNode
      raise "Destination Node doesn't exist: #{dr.destNode}" unless nw.nodes.has_key? dr.destNode
      dr.originNode = nw.nodes[dr.originNode]
      dr.destNode = nw.nodes[dr.destNode]

      unless drivers.has_key? dr.agentID
        drivers[dr.agentID] = dr
        min = dr unless min and min.agentID<=dr.agentID
        max = dr unless max and max.agentID>=dr.agentID
      else
        raise "Agent ID already exists: #{dr.agentID}"
      end
    else 
      puts "Skipped line: #{line}"
    end
  }
  return min, max
end


end #module MS_DepartureParser

