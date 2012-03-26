

class Node
  def initialize(nodeID)
    @nodeID = nodeID
    @x = 0
    @y = 0
  end  

  attr_reader :nodeID
  attr_accessor :x
  attr_accessor :y
end

#class Point
#  def initialize(x, y)
#    @x = x
#    @y = y
#  end  
#  attr_accessor :x
#  attr_accessor :y
#end

#Get standard deviation and average
def getStdAvg(list)
  #Sum, average
  sum = 0
  list.each {|item|
    sum += item
  }
  avg = sum / list.length.to_f

  #Sample variance, stdev
  sVar = 0
  list.each {|item|
    sVar += (item-avg)**2
  }
  sVar /= list.length.to_f
  stDev = Math.sqrt(sVar)
  
  return [avg, stDev]
end


class Driver
  def initialize(agentID)
    @agentID = agentID
    @departure = 0
    @arrival = 0
    @completed = false
    @originNode = nil
    @destNode = nil
    @vehicleType = nil

    @firstPos = nil

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

  attr_accessor :firstPos

  attr_accessor :originNode
  attr_accessor :destNode
  attr_accessor :vehicleType

  attr_accessor :tempVeh2
  attr_accessor :tempVeh3
  attr_accessor :tempVeh6
  attr_accessor :tempVeh9
  attr_accessor :tempVeh10
  attr_accessor :tempVeh11
end



#The network file has a complex format. Basically, we are just extracting
#  segments for now.
def read_network_file(segments)
  #First, pre-process and remove comments. Apparently *all* comment types
  #  are valid for mitsim...
  bigstring = ""
  onComment = false
  File.open('network-BUGIS.dat').each { |line|
    line.chomp!
    if onComment
      #Substitute the comment closing and set onComment to false
      if line.sub!(/^((?!\*\/).)*\*\//, '') #C style, closing
        onComment = false
      else
        line = '' #Still in the comment
      end
    end
    unless onComment
      #Single line regexes are easy
      line.sub!(/#.*$/, '')  #Perl style
      line.sub!(/\/\/.*$/, '')  #C++ style
      line.gsub!(/\/\*((?!\*\/).)*\*\//, '') #C style single line

      #Now multi-line C comments
      onComment = true if line.sub!(/\/\*((?!\*\/).)*$/, '')  #C style, line ending reached
    end
    bigstring << line
  }

  #Now, parse unrelated sections out of the "bigstring".
  bigstring.sub!(/^.*(?=\[Links\])/, '')
  bigstring.sub!(/\[(?!Links)[^\]]+\].*/, '')
  bigstring.gsub!(/[ \t]+/, ' ') #Reduce all spaces/tabs to a single space
  bigstring.gsub!(/\[Links\][^{]*{/, '') #Avoid a tiny bit of lookahead. There will be an unmatched } at the end, but it won't matter

  #Links start with:  {1 2 3 4 5   #LinkID LinkType UpNodeID DnNodeID LinkLabelID 
  posInt = '([0-9]+)'
  posDbl = '([0-9\.]+)'
  posSci = '([0-9]+\.[0-9]+e[-+][0-9]+)'
  linkHead = "{ ?#{posInt} #{posInt} #{posInt} #{posInt} #{posInt} ?"
  #After that is at least one segment: {1 2 3 4  #SegmentID DefaultSpeedLimit FreeSpeed Grad 
  segmentHead = "{ ?#{posInt} #{posInt} #{posInt} #{posInt} ?"
  #After that is exactly one descriptor: {1.0e+0 2.0e+0 3.00 4.0e+0 5.0e+0}  #StartingPntX StartingPntY Bulge EndPntX EndPntY
  segmentDesc = "{ ?#{posSci} #{posSci} #{posDbl} #{posSci} #{posSci} ?} ?"
  #After that is at least one lane rules group: {1, 2}
  laneRule = "{ ?#{posInt} #{posInt} ?} ?"
  #Close each segment with }
  segmentStr = "#{segmentHead}#{segmentDesc}((#{laneRule})+}) ?"
  segmentRegex = Regexp.new(segmentStr)
  #Close the Link with }
  linkRegex = Regexp.new("#{linkHead}((#{segmentStr})+}) ?")
  bigstring.scan(linkRegex) {|linkRes|
    linkID = linkRes[0]
    upNodeID = linkRes[2]
    downNodeID = linkRes[3]

    puts '-'*20
    puts "Link ID: #{linkID}"
    puts "UpNode: #{upNodeID}"
    puts "DownNode: #{downNodeID}"

    #Now get segments
    segmentStr = linkRes[5]
    segmentStr.scan(segmentRegex) {|segRes|
      segID = segRes[0]
      segStartX = segRes[4]
      segStartY = segRes[5]
      segEndX = segRes[7]
      segEndY = segRes[8]

      puts "Segment [#{segID}],  (#{segStartX}, #{segStartY}) => (#{segEndX}, #{segEndY})"
    }

    puts '-'*20
  }





  
  
  
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
#  time
#  agentID
#  originNode
#  destNode
#  vehicleType
#  path, but ONLY if the vehicles have a path
def read_dep_file(list)
  min = max = nil
  File.open('dep.out').each { |line|
    if line =~ /([0-9.]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+)/
      dr = Driver.new($2.to_i)
      dr.departure = $1.to_f
      dr.originNode = $3.to_i
      dr.destNode = $4.to_i
      dr.vehicleType = $5.to_i

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
  minDepartureTime = -1
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

        #Calculate min. departure time
        minDepartureTime = dr.departure if minDepartureTime==-1 or dr.departure<minDepartureTime
      else
        raise "Agent ID expected but doesn't exist: #{id}"
      end
    else 
      puts "Skipped line: #{line}"
    end
  }
  return minDepartureTime
end


#For now, this just adds firstPos value for each driver
#Total of 8 values:
#  time 
#  agentID 
#  segmentID
#  laneID 
#  positionInLane 
#  speed 
#  acceleration 
#  vehicleType
def read_traj_file(list)
  File.open('traj_compact.txt').each { |line|  #We use a trimmed version of trajectory.out, but loading the original would work too.
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9.]+) +([0-9e.\-]+) +([0-9]+)/
      id = $2.to_i
      if list.has_key? id
        dr = list[id]
        unless dr.firstPos
          dr.firstPos = $5.to_f
        end
      else
        raise "Agent ID expected but doesn't exist: #{id}"
      end
    else 
      puts "Skipped line: #{line}"
    end
  }

  #Build a lookup of Nodes based on the first location of each driver.
  node_lookup = {} #Array of offsets, indexed by NodeID
  list.each{|id, drv| 
    next unless drv.firstPos
    node_lookup[drv.originNode] = [] unless node_lookup.has_key? drv.originNode
    node_lookup[drv.originNode].push(drv.firstPos)
  }

  #Temp: print
  node_lookup.each {|id, offsets|
    min = max = offsets[0]
    puts "First 'offset' values for agents in Node ID #{id}:"
    offsets.each {|offset|
#      puts "  #{offset}"
      min = offset if offset < min
      max = offset if offset > max
    }
    avg, stddev = getStdAvg(offsets)
    puts "Average:  #{avg} +/- #{stddev}"
    puts "Min/max:  #{min} => #{max}"
    puts '-'*20
  }
end


def read_convert_file(list, dict)
  File.open('ms_sm_node_convert.txt').each { |line|
    next if line =~ /^#/ or line.strip.empty?
    if line =~ /([0-9]+) *=> *([0-9]+)/
      from = $1.to_i
      to = $2.to_i
      if dict.has_key? from
        raise "Comparison error" if dict[from].nodeID != to
      else
        dict[from] = Node.new(to)
      end
    elsif line =~ /([0-9]+) *= *\(([0-9]+),([0-9]+)\)/
      found = false
      dict.each_value{|nd|
        if nd.nodeID == $1.to_i
          nd.x = $2.to_i
          nd.y = $3.to_i
          found = true
        end
      }
      raise "Couldn't find node: #{$1}" unless found
    else
      puts "Skipped line: #{line}"
    end
  }

  #Final check
  dict.each_value{|nd|
    #raise "Node ID not translated: #{nd.nodeID}" if nd.x==0 or nd.y==0
  }
end


def final_validate(agents, nodeConv, minDepTime)
  agents.each{|id, dr|
    #Make sure sim mobility node IDs exist
    raise "No Sim Mobility node id for: #{dr.originNode}" unless nodeConv.has_key? dr.originNode
    raise "No Sim Mobility node id for: #{dr.destNode}" unless nodeConv.has_key? dr.destNode
    dr.originNode = nodeConv[dr.originNode]
    dr.destNode = nodeConv[dr.destNode]

    #Start our departure times at zero, and convert to ms (then int)
    #TODO: We might want to generate a <simulation> tag instead. For now this is easier.
    dr.departure =  ((dr.departure - minDepTime)*1000).to_i
  }
end


def run_main()
  #Read the network file
  segments = {}
  read_network_file(segments)

  #Build up a list of all Driver IDs
  drivers = {}
  min, max = read_dep_file(drivers)

  #Cross-reference with the vehicles file
  minDepTime = read_veh_file(drivers)

  #Parse the trajectory file
  read_traj_file(drivers)

  #Compare with sim mobility nodes
  nodeConv = {}
  read_convert_file(drivers, nodeConv)

  #Final validation
  final_validate(drivers, nodeConv, minDepTime)

  #Print the Agents
  File.open('agents.gen.xml', 'w') {|f|
    f.write("<agents>\n") 
    drivers.keys.sort.each{|id|
      dr = drivers[id]
      f.write("  <driver id='#{id}'")
      f.write(" originPos='(#{dr.originNode.x},#{dr.originNode.y})'")
      f.write(" destPos='(#{dr.destNode.x},#{dr.destNode.y})'")
      f.write(" startTime='#{dr.departure}'/>\n")
    }
    f.write("</agents>") 
  }

  #Some drivers are never started
  drvSkip = 0
  drivers.each {|id, dr|
    drvSkip += 1 unless dr.firstPos
  }

  #Print some statistics
  printf("%-13s %5s\n", 'Min agent id:', min.agentID)
  printf("%-13s %5s\n", 'Max agent id:', max.agentID)
  printf("%-13s %5s\n", 'Num agents:', drivers.length)
  raise "Agent IDs must be monotonic increasing from 1..num_agents" unless min.agentID==1 and max.agentID==drivers.length
  if drvSkip>0
    printf("A total of %d Drivers never started driving (%.2f%%)\n", drvSkip, (100.0*drvSkip/drivers.length))
  end
  puts 'Agents saved to agents.gen.xml'


end




if __FILE__ == $PROGRAM_NAME
  run_main()
end


