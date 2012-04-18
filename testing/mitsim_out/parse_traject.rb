require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'


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


module MS_TrajectoryParser


def self.read_traj_file(trajFileName, nw, drivers)
  lastKnownTime = 0
  unknownNodes = []
  File.open(trajFileName).each { |line|
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9.]+) +([0-9e.\-]+) +([0-9]+)/
      id = $2.to_i
      if drivers.has_key? id
        dr = drivers[id]

        #Saving firstPos
        unless dr.firstPos
          dr.firstPos = $5.to_f
        end

        #Time
        time = $1.to_i
        raise "Error: time must be strictly increasing" if time<lastKnownTime

        #segmentID, laneID, positionInLane
        segmentID = $3
        raise "Segment ID not known: " + segmentID.to_s unless nw.segments.has_key? segmentID
        laneID = $4
        posInLane = $5.to_f
        
        #Get the upstream node
        segment = nw.segments[segmentID]
        unless segment.upNode.include? ':'  #We don't consider uni-nodes for now.
          unless nw.nodes.has_key? segment.upNode
            unknownNodes.push(segment.upNode)
          end
        end

        #Increment
        lastKnownTime = time
      else
        raise "Agent ID expected but doesn't exist: #{id}"
      end
    else 
      puts "Skipped line: #{line}"
    end
  }


  puts "Unknown node IDs: #{unknownNodes.uniq}" unless unknownNodes.empty?

  #Build a lookup of Nodes based on the first location of each driver.
  node_lookup = {} #Array of offsets, indexed by NodeID
  drivers.each{|id, drv| 
    next unless drv.firstPos
    node_lookup[drv.originNode] = [] unless node_lookup.has_key? drv.originNode
    node_lookup[drv.originNode].push(drv.firstPos)
  }

  #Temp: print
  node_lookup.each {|id, offsets|
    next  #Skip for now
    min = max = offsets[0]
    puts "First 'offset' values for agents in Node ID #{id}:"
    offsets.each {|offset|
#      puts "  #{offset}"
      min = offset if offset < min
      max = offset if offset > max
    }
    avg, stddev = GetStdDevAndAverage(offsets)
    puts "Average:  #{avg} +/- #{stddev}"
    puts "Min/max:  #{min} => #{max}"
    puts '-'*20
  }
end

end #module MS_TrajectoryParser

