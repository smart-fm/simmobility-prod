require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'



#Helper; make sure all nodes exist
def attemptToGenerateID(nw, linkID, pos, nodeID, currSubID)
  #If no Node ID exists, generate a node and add it to the node list
  unless nodeID
    #Somewhat fragile, but should be good enough
    raise "Too many segments" if currSubID[0]>999

    #Set the ID (string)
    nodeID = "#{linkID}:#{currSubID[0]}"
    currSubID[0] += 1

    #Make the node
    nd = nw.newNode(nodeID)
    nd.pos = Mitsim::Point.new(pos.x, pos.y)
  end

  #Return the (new?) ID
  return nodeID
end


#Parse a file, removing C, C++, and Perl-style comments.
def parse_file_remove_comments(fileName)
  stream = ''
  onComment = false
  File.open(fileName).each { |line|
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
    stream << line
  }

  #Now, parse unrelated sections out of the stream.
  stream.sub!(/^.*(?=\[Links\])/, '')
  stream.sub!(/\[(?!Links)[^\]]+\].*/, '')
  stream.gsub!(/[ \t]+/, ' ') #Reduce all spaces/tabs to a single space
  stream.gsub!(/\[Links\][^{]*{/, '') #Avoid a tiny bit of lookahead. There will be an unmatched } at the end, but it won't matter

  return stream
end


#Either add a Node (by position) or, if it exists, ensure that it
# hasn't moved much.
def addOrCheckNode(nw, id, currentPos)
  nd = nw.getOrAddNode(id)
  unless nd.pos
    nd.pos = Mitsim::Point.new(currentPos.x, currentPos.y)
  else
    #Ensure the position hasn't changed.
    errorVal = Distance(nd.pos, currentPos)
    puts "Node ID/position mismatch[#{id}],\n  prev: #{nd.pos},\n  curr: #{currentPos}\n  error: #{errorVal})" if errorVal >= 10.0
    puts "Node ID/position mismatch[#{id}], off by #{errorVal} meters" if errorVal >= 1.0
  end
end



#The network file has a complex format. Basically, we are just extracting
#  segments for now.
def read_network_file(nwFileName, nw)
  #First, pre-process and remove comments. Apparently *all* comment types
  #  are valid for mitsim...
  bigstring = parse_file_remove_comments(nwFileName)

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
    link = nw.newLink(linkID)
    link.upNode = upNodeID
    link.downNode = downNodeID

    #Now get segments
    #Segments are listed from upstream to downstream
    segmentStr = linkRes[5]
    prevEndPoint = nil
    segmentStr.scan(segmentRegex) {|segRes|
      segID = segRes[0]
      segStartX = ParseScientificToFloat(segRes[4])
      segStartY = ParseScientificToFloat(segRes[5])
      segEndX = ParseScientificToFloat(segRes[7])
      segEndY = ParseScientificToFloat(segRes[8])

      #Save it temporarily
      seg = nw.newSegment(segID)
      seg.startPos = Mitsim::Point.new(segStartX, segStartY)
      seg.endPos = Mitsim::Point.new(segEndX, segEndY)
      seg.parentLink = link
      link.segments.push(seg)

      #Double-check consistency
      if prevEndPoint
        unless seg.startPos.x==prevEndPoint.x and seg.startPos.y==prevEndPoint.y
          #Skip errors less than 1 m
          errorVal = Distance(seg.startPos, prevEndPoint)
          raise "Segment consistency error on Segment: #{segID} Distance: #{errorVal}" if errorVal >= 1.0 
        end
      end
      prevEndPoint = seg.endPos
    }


    #Now set each segment's node ids and go from there
    currSubID = [1] #Node ID for segments.
    link.segments[0].upNode = upNodeID
    link.segments[-1].downNode = downNodeID

    #Force add Link start/end node
    addOrCheckNode(nw, link.segments[0].upNode, link.segments[0].startPos)
    addOrCheckNode(nw, link.segments[-1].downNode, link.segments[-1].endPos)

    #Add UniNodes for each segment. These will have generated names, with ":1" appended for the 
    # Node closest to upstream, and so forth.
    link.segments.each{|segment|
      #Attempt to generate an ID. Even if this fails, save this ID
      segment.upNode = attemptToGenerateID(nw, linkID, segment.startPos, segment.upNode, currSubID)
      segment.downNode = attemptToGenerateID(nw, linkID, segment.endPos, segment.downNode, currSubID)
    }
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
def read_dep_file(depFileName, nw, drivers)
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


#The vehicle.out file contains, for vehicles that have reached their destinations, data pertaining to:
#    origin,
#    destination,
#    departure time,
#    arrival time,
#    mileage
#    speed
#(Actually: 11 values +5)
def read_veh_file(vehFileName, drivers)
  minDepartureTime = -1
  File.open(vehFileName).each { |line|
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9]+)/
      #Retrieve and check the ID
      id = $1.to_i
      raise "Agent ID expected but doesn't exist: #{id}" unless drivers.has_key? id

      #Retrieve and check the driver.
      dr = drivers[id]
      raise "Agent completed twice: #{id}" if dr.completed
      dr.arrival = $8.to_i
      dr.completed = true
      raise "Origin mismatch for agent: #{id} => #{$4}" if dr.originNode.nodeID != $4
      raise "Dest mismatch for agent: #{id} => #{$5}" if dr.destNode.nodeID != $5
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
def read_traj_file(trajFileName, nw, drivers)
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


#outputSMNodeID is just for show
#def compute_error(smNodeLookup, msNode, nodeID, outputSMNodeID, offset)
#  smNode = smNodeLookup[nodeID]
#  return if smNode.x==0 and smNode.y==0 #Not in our lookup file
#
#  expected = Point.new(msNode.x*100+offset.x, msNode.y*100+offset.y)
#  diff = Point.new(expected.x-smNode.x, expected.y-smNode.y)
#  #puts "Mitsim(#{nodeID}) to Sim Mobility(#{outputSMNodeID}) has error of: (#{diff.x},#{diff.y})"
#  #puts "   mitsim: (#{msNode.x*100},#{msNode.y*100})"
#  #puts "   simmob: (#{smNode.x},#{smNode.y})"
#end



def ms_sm_nodeset(nw, mitsim, simmob)
  raise "Unknown mitsim node: #{mitsim}" unless nw.nodes.has_key? mitsim
  mitsim = nw.nodes[mitsim]

  #Set or check
  unless mitsim.sm_node
    mitsim.sm_node = nw.sm_network.getOrAddNode(simmob)
  else
    raise "Comparison error" if mitsim.sm_node.aimsunID != simmob
  end
end


#Read our (manually built) Node conversion file.
def read_convert_file(convFileName, nw, drivers)
  File.open(convFileName).each { |line|
    #Skip comments
    next if line =~ /^#/ or line.strip.empty?

    #Lines of the form "38 => 98088" specify that Mitsim node (38) is mapped to
    #  Sim Mobility node (98088)
    if line =~ /([0-9]+) *=> *([0-9]+)/
      ms_sm_nodeset(nw, $1, $2)

    #It's also common for Mitsim to "break up" Sim Mobility nodes into several pieces:
    #  {111,1111} => 98226
    elsif line =~ /{((?:[0-9:]+[, ]*)+)} *=> *([0-9]+)/
      mitsim_ids = $1
      simmob = $2
      mitsim_ids.scan(/[0-9:]+/) {|line|
        ms_sm_nodeset(nw, line, simmob)
      }

    #We also (temporarily) require Sim Mobility nodes to specify their own positions.
    #  48732 = (37232859,14308003)
    #This will be removed as soon as we start parsing Sim Mobility output files.
    elsif line =~ /([0-9]+) *= *\(([0-9]+),([0-9]+)\)/
      nodeid = $1
      pos = SimMob::Point.new($2.to_i, $3.to_i)
      raise "Couldn't find node: #{nodeid}" unless nw.sm_network.nodes.has_key? nodeid
      node = nw.sm_network.nodes[nodeid]

      unless node.pos
        node.pos = pos
      else
        raise "Position mismatch" unless Distance(node.pos, pos)==0
      end      

    #Anything else is reported as unexpected
    else
      puts "Skipped line: #{line}"
    end
  }

  #Final check
  nw.sm_network.nodes.each_value{|nd|
    puts "Node ID not translated: #{nd.aimsunID}" unless nd.pos
  }
end


def final_validate(network, drivers, minDepTime)
  #Check the offset from sim mobility nodes to mitsim ones
  #sampleNodePos_SM = nodeConv['122'] #122 => 60896, the lower-left most node
  #sampleNodePos_MS = nil
  #segments.each{|key, seg|
  #  if seg.upNode=='122'
  #    sampleNodePos_MS = seg.startPos
  #    break
  #  elsif seg.downNode=='122'
  #    sampleNodePos_MS = seg.endPos
  #    break
  #  end
  #}
  #raise "Couldn't find sample node!" unless sampleNodePos_MS
  #
  #Offset FROM mitsim points TO sim mobility ones. (Add this to mitsim points)
  #NOTE: This doesn't work. We'll need to use the Sim Mobility coordinates directly; there's
  #      far too much error if we just scale by one of the MITSIM points. 
  #offset = Point.new(sampleNodePos_SM.x-sampleNodePos_MS.x*100, sampleNodePos_SM.y-sampleNodePos_MS.y*100)
  #puts "Mitsim node location: (#{sampleNodePos_MS.x*100},#{sampleNodePos_MS.y*100})"
  #puts "Sim Mobility node location: (#{sampleNodePos_SM.x},#{sampleNodePos_SM.y})"
  #puts "Offset (ms+): (#{offset.x},#{offset.y})"
  #
  #Now check how much error we get versus other node IDs:
  #segments.each{|key, seg|
  #  unless seg.upNode.include? ':'
  #    if nodes.has_key? seg.upNode and nodeConv.has_key? seg.upNode
  #      compute_error(nodeConv, seg.startPos, seg.upNode, nodeConv[seg.upNode].nodeID, offset)
  #    end
  #  end
  #  unless seg.downNode.include? ':'
  #    if nodes.has_key? seg.downNode and nodeConv.has_key? seg.downNode
  #      compute_error(nodeConv, seg.endPos, seg.downNode, nodeConv[seg.downNode].nodeID, offset)
  #    end
  #  end
  #}


  drivers.each{|id, dr|
    #Make sure sim mobility node IDs exist
    raise "No Sim Mobility node id for: #{dr.originNode}" unless dr.originNode.sm_node
    raise "No Sim Mobility node id for: #{dr.destNode}" unless dr.destNode.sm_node
#    dr.originNode = nodeConv[dr.originNode.to_s]
#    dr.destNode = nodeConv[dr.destNode.to_s]

    #Start our departure times at zero, and convert to ms (then int)
    #TODO: We might want to generate a <simulation> tag instead. For now this is easier.
    dr.departure =  ((dr.departure - minDepTime)*1000).to_i
  }
end


#Generate something that the visualizer can parse.
def fakeNodeID(nodeID)
  ind = nodeID.index(':')
  return nodeID.to_i unless ind
  prefix = nodeID.slice(0,ind).to_i
  suffix = nodeID.slice(ind+1, nodeID.length).to_i
  return prefix*1000 + suffix
end




def print_network(nw)
  #Try printing the MITSIM node network
  knownNodeIDs = []
  allNodeIDs = []
  possibleLinks = []
  nw.segments.each{|key, seg|
    numFound = 0
    unless seg.upNode.include? ':'
      if nw.nodes.has_key? seg.upNode and nw.nodes[seg.upNode].sm_node
        knownNodeIDs.push(seg.upNode)
        numFound += 1
      end
    end
    unless seg.downNode.include? ':'
      if nw.nodes.has_key? seg.downNode and nw.nodes[seg.downNode].sm_node
        knownNodeIDs.push(seg.downNode)
        numFound += 1
      end
    end

    #Hmm
    if nw.nodes.has_key?(seg.upNode) and not allNodeIDs.include? seg.upNode
      allNodeIDs.push(seg.upNode)
    end
    if nw.nodes.has_key?(seg.downNode) and not allNodeIDs.include? seg.downNode
      allNodeIDs.push(seg.downNode)
    end

    #Mark this link for later
    #if numFound>0
    possibleLinks.push(seg.parentLink) unless possibleLinks.include? seg.parentLink
    #end
  }
  possibleLinks.each{|link|
    numFound = 0
    #puts "Checking: #{link.linkID} => (#{link.upNode},#{link.downNode})"
    unless link.upNode.include? ':'
      #puts " up:"
      if nw.nodes.has_key? link.upNode and nw.nodes[link.upNode].sm_node
        knownNodeIDs.push(link.upNode)
        numFound += 1
        #puts " +1A"
      end
    end
    unless link.downNode.include? ':'
      #puts " down:"
      if nw.nodes.has_key? link.downNode and nw.nodes[link.downNode].sm_node
        knownNodeIDs.push(link.downNode)
        numFound += 1
        #puts " +1B"
      end
    end

    #Do we know about this link?
    puts "Found: #{link.upNode} => #{link.downNode}" if numFound==2
  }
  File.open('output_network.txt', 'w') {|f|
#    knownNodeIDs.uniq.each{|nodeID|
    allNodeIDs.each{|nodeID|
      nd = nw.nodes[nodeID]
      next unless nd.pos
      name = 'multi'
      name = 'uni' if nodeID.include? ':'
      f.write("(\"#{name}-node\", 0, #{fakeNodeID(nodeID)}, {")  #Header
      f.write("\"xPos\":\"#{(nd.pos.x*100).to_i}\",\"yPos\":\"#{(nd.pos.y*100).to_i}\",") #Guaranteed
      f.write("\"mitsim-id\":\"#{nodeID}\",") if nw.nodes.has_key? nodeID  #Optional (now it's guaranteed though)
      f.write("\"aimsun-id\":\"#{nw.nodes[nodeID].sm_node}\",") if nw.nodes[nodeID].sm_node  #Optional
      f.write("})\n") #Footer
    }

    #Now write all Links
    possibleLinks.each{|link|
      f.write("(\"link\", 0, #{link.linkID}, {")  #Header
      f.write("\"road-name\":\"\",\"start-node\":\"#{fakeNodeID(link.upNode)}\",") #Guaranteed
      f.write("\"end-node\":\"#{fakeNodeID(link.downNode)}\",") #Also guaranteed
      f.write("\"fwd-path\":\"[")
      link.segments.each{|segment|
        f.write("#{segment.segmentID},")
      }
      f.write("]\",") #Close fwd-path
      f.write("\"rev-path\":\"[")
      #TODO: Currently there seems to be no way to do this.
      f.write("]\",") #Close rev-path
      f.write("})\n") #Footer
    }

    #Now write all Segments
    possibleLinks.each{|link|
      link.segments.each{|segment|
        f.write("(\"road-segment\", 0, #{segment.segmentID}, {")  #Header
        f.write("\"parent-link\":\"#{link.linkID}\",") #Guaranteed
        f.write("\"max-speed\":\"0\",") #Not hooked up yet
        f.write("\"lanes\":\"1\",") #Not hooked up yet
        f.write("\"from-node\":\"#{fakeNodeID(segment.upNode)}\",") #Not hooked up yet
        f.write("\"to-node\":\"#{fakeNodeID(segment.downNode)}\",") #Not hooked up yet
        f.write("})\n") #Footer
      }
    }
  }
end


def print_agents(nw, drivers, min, max)
  #Print the Agents
  skipped = 0
  total = 0
  File.open('agents.gen.xml', 'w') {|f|
    f.write("<agents>\n") 
    drivers.keys.sort.each{|id|
      skipped += 1
      total += 1
      dr = drivers[id]
      next unless dr.originNode.sm_node and dr.destNode.sm_node
      next unless dr.originNode.sm_node.pos and dr.destNode.sm_node.pos
      f.write("  <driver id='#{id}'")
      f.write(" originPos='(#{dr.originNode.sm_node.pos.x},#{dr.originNode.sm_node.pos.y})'")
      f.write(" destPos='(#{dr.destNode.sm_node.pos.x},#{dr.destNode.sm_node.pos.y})'")
      f.write(" startTime='#{dr.departure}'/>\n")
      skipped -= 1
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
  unless skipped == 0
    puts '-'*40
    printf("WARNING: %d agents (%.2f%%) were SKIPPED\n", skipped, (100.0*skipped/total))
    puts 'Your simulation will not be accurate unless these Agents are eventually added back in.'
    puts '-'*40
  end
end



def run_main()
  #Internal data structures
  network = Mitsim::RoadNetwork.new()
  network.sm_network = SimMob::RoadNetwork.new()
  drivers = {}

  #Read the network file
  read_network_file('network-BUGIS.dat', network)

  #Build up a list of all Driver IDs
  min, max = read_dep_file('dep.out', network, drivers)

  #Cross-reference with the vehicles file
  minDepTime = read_veh_file('vehicle.out', drivers)

  #Compare with sim mobility nodes
  read_convert_file('ms_sm_node_convert.txt', network, drivers)

  #Parse the trajectory file
  #We use a trimmed version of trajectory.out, but loading the original would work too.
  read_traj_file('traj_compact.txt', network, drivers)

  #Final validation
  final_validate(network, drivers, minDepTime)

  #Print 
  print_network(network)
  print_agents(network, drivers, min, max)

end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




