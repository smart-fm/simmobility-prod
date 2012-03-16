
class Driver
  def initialize(agentID)
    @agentID = agentID
    @departure = 0
    @arrival = 0
    @completed = false

    #Note: Origin and dest might be switched; we can easily confirm this later.
    @originNode = nil
    @destNode = nil

    @tempDep5 = nil #Either vehicle_type or path
    @tempVeh2 = nil #Something boolean
    @tempVeh6 = nil #Usually equals the destination node except in very rare cases (~38 of them)

    @tempVeh3 = nil #Might be distance in meters
    @tempVeh9 = nil #Same

    @tempVeh10 = nil #Almost definitely speed in m/s
    @tempVeh11 = nil #Always zero
  end  

  attr_reader :agentID
  attr_accessor :departure
  attr_accessor :arrival
  attr_accessor :completed

  attr_accessor :originNode
  attr_accessor :destNode
  attr_accessor :tempDep5
  attr_accessor :tempVeh2
  attr_accessor :tempVeh3
  attr_accessor :tempVeh6
  attr_accessor :tempVeh9
  attr_accessor :tempVeh10
  attr_accessor :tempVeh11
end


#The dep.out file contains the departure record of each vehicle in the simulation.
#This comprises of:
#    departure time, 
#    vehicle ID, 
#    origin, 
#    destination, 
#    vehicle type,
#    path
#(Actually: 5 values -1)
def read_dep_file(list)
  min = max = nil
  File.open('dep.out').each { |line|
    if line =~ /([0-9.]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+)/
      dr = Driver.new($2.to_i)
      dr.departure = $1.to_f
      dr.originNode = $3.to_i
      dr.destNode = $4.to_i
      dr.tempDep5 = $5.to_i

      unless list.has_key? dr.agentID
        list[dr.agentID] = dr
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


#The vehicle.out file contains, for vehicles that have reached their destinations, data pertaining to:
#    origin,
#    destination,
#    departure time,
#    arrival time,
#    mileage
#    speed
#(Actually: 11 values +5)
def read_veh_file(list)
  File.open('vehicle.out').each { |line|
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9]+)/
      id = $1.to_i
      if list.has_key? id
        dr = list[id]
        raise "Agent completed twice: #{id}" if dr.completed
        dr.arrival = $8.to_i
        dr.completed = true
        raise "Origin mismatch for agent: #{id}" if dr.originNode != $4.to_i
        raise "Dest mismatch for agent: #{id}" if dr.destNode != $5.to_i
        raise "Departure time mismatch for agent: #{id}" if dr.departure.round != $7.to_i
        raise "Arrived before departure for agent: #{id}" if dr.arrival < dr.departure
        t2 = $2.to_i
        raise "Expected boolean, not: #{t2}" unless t2==0 or t2==1
        dr.tempVeh2 = t2==0 ? false : true
        dr.tempVeh3 = $3.to_i
        dr.tempVeh6 = $6.to_i
        dr.tempVeh9 = $9.to_i
        dr.tempVeh10 = $10.to_f
        t11 = $11.to_i
        raise "Expected boolean, not: #{t11}" unless t11==0 or t11==1
        dr.tempVeh11 = t11==0 ? false : true

      else
        raise "Agent ID expected but doesn't exist: #{dr.agentID}"
      end
    else 
      puts "Skipped line: #{line}"
    end
  }
end


def read_convert_file(list, dict)
  File.open('ms_sm_node_convert.txt').each { |line|
    next if line =~ /^#/
    if line =~ /([0-9]+) *=> *([0-9]+)/
      from = $1.to_i
      to = $2.to_i
      if dict.has_key? from
        raise "Comparison error" if dict[from] != to
      else
        dict[from] = to
      end
    else
      puts "Skipped line: #{line}"
    end
  }
end


def final_validate(agents, nodeConv)
  agents.each{|id, dr|
    #Make sure sim mobility node IDs exist
    raise "No Sim Mobility node id for: #{dr.originNode}" unless nodeConv.has_key? dr.originNode
    raise "No Sim Mobility node id for: #{dr.destNode}" unless nodeConv.has_key? dr.destNode
    dr.originNode = nodeConv[dr.originNode]
    dr.destNode = nodeConv[dr.destNode]
  }
end


def run_main()
  #Build up a list of all Driver IDs
  drivers = {}
  min, max = read_dep_file(drivers)

  #Cross-reference with the vehicles file
  read_veh_file(drivers)

  #Compare with sim mobility nodes
  nodeConv = {}
  read_convert_file(drivers, nodeConv)

  #Final validation
  final_validate(drivers, nodeConv)

  #Print the Agents
  puts '-'*20
  drivers.keys.sort.each{|id|
    dr = drivers[id]
    puts "<driver id='#{id}' origin='#{dr.originNode}' dest='#{dr.destNode}' startTime='#{dr.departure}'/>"
  }
  puts '-'*20

  #Print some statistics
  printf("%-13s %5s\n", 'Min agent id:', min.agentID)
  printf("%-13s %5s\n", 'Max agent id:', max.agentID)
  printf("%-13s %5s\n", 'Num agents:', drivers.length)
  raise "Agent IDs must be monotonic increasing from 1..num_agents" unless min.agentID==1 and max.agentID==drivers.length


end




if __FILE__ == $PROGRAM_NAME
  run_main()
end


