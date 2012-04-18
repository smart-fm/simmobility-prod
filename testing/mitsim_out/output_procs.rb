require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'


#Methods for output.
module Output


#Various helpers grouped into a class
class Helper
#Generate something that the visualizer can parse.
  def fakeNodeID(nodeID)
    ind = nodeID.index(':')
    return nodeID.to_i unless ind
    prefix = nodeID.slice(0,ind).to_i
    suffix = nodeID.slice(ind+1, nodeID.length).to_i
    return prefix*1000 + suffix
  end
end #class Helper



def self.print_network(nw, timeticks)
  help = Helper.new

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
    #puts "Found: #{link.upNode} => #{link.downNode}" if numFound==2
  }
  File.open('output_network.txt', 'w') {|f|
#    knownNodeIDs.uniq.each{|nodeID|
    allNodeIDs.each{|nodeID|
      nd = nw.nodes[nodeID]
      next unless nd.pos
      name = 'multi'
      name = 'uni' if nodeID.include? ':'
      f.write("(\"#{name}-node\", 0, #{help.fakeNodeID(nodeID)}, {")  #Header
      f.write("\"xPos\":\"#{(nd.pos.x*100).to_i}\",\"yPos\":\"#{(nd.pos.y*100).to_i}\",") #Guaranteed
      f.write("\"mitsim-id\":\"#{nodeID}\",") if nw.nodes.has_key? nodeID  #Optional (now it's guaranteed though)
      f.write("\"aimsun-id\":\"#{nw.nodes[nodeID].sm_node}\",") if nw.nodes[nodeID].sm_node  #Optional
      f.write("})\n") #Footer
    }

    #Now write all Links
    possibleLinks.each{|link|
      f.write("(\"link\", 0, #{link.linkID}, {")  #Header
      f.write("\"road-name\":\"\",\"start-node\":\"#{help.fakeNodeID(link.upNode)}\",") #Guaranteed
      f.write("\"end-node\":\"#{help.fakeNodeID(link.downNode)}\",") #Also guaranteed
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
        f.write("\"lanes\":\"#{segment.lanes.length}\",") 
        f.write("\"from-node\":\"#{help.fakeNodeID(segment.upNode)}\",") #Not hooked up yet
        f.write("\"to-node\":\"#{help.fakeNodeID(segment.downNode)}\",") #Not hooked up yet
        f.write("})\n") #Footer

        #Create a "start" and "end" DynamicVector, rotate, and scale to the size of a lane line
        startVec = DynamicVector.new(segment.startPos, segment.endPos)
        startVec.flipNormal($FlipRight)
        startVec.scaleTo($LaneWidth)
        endVec = DynamicVector.new(segment.endPos, segment.startPos)
        endVec.flipNormal(!$FlipRight)
        endVec.scaleTo($LaneWidth)

        #Write lanes
        f.write("(\"lane\", 0, #{segment.lanes.object_id}, {")  #Header; IDs merely have to be unique.
        f.write("\"parent-segment\":\"#{segment.segmentID}\",") #Guaranteed
        (0..segment.lanes.length).each{|lineID|
          f.write("\"line-#{lineID}\":\"[(#{(startVec.getX()*100).to_i},#{(startVec.getY()*100).to_i}),(#{(endVec.getX()*100).to_i},#{(endVec.getY()*100).to_i}),]\",")
          startVec.translate()
          endVec.translate()
        }
        f.write("})\n") #Footer
      }
    }

    #Write drivers
    timeticks.keys.sort.each{|tick|
      timeticks[tick].each_key{|driverID|
        driverTick = timeticks[tick][driverID]
        f.write("(\"Driver\", #{tick}, #{driverID}, {")  #Header
        f.write("\"xPos\":\"#{driverTick.pos.x*100}\",") #Guaranteed
        f.write("\"yPos\":\"#{driverTick.pos.y*100}\",") #Guaranteed
        f.write("\"angle\":\"#{driverTick.angle*180/Math::PI}\",")
        f.write("\"length\":\"400\",") #Not hooked up yet
        f.write("\"width\":\"200\",") #Not hooked up yet
        f.write("})\n") #Footer
      }
    }
  }
end


def self.print_agents(nw, timeticks, drivers, min, max)
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
    drvSkip += 1 unless dr.hasAtLeastOneTick
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




end #module Output

