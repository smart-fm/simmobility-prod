require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'

#This parses an output file from Sim Mobility, taking Node and Segment information in as 
#  required. It operates fastest if you remove all non-network (driver/pedestrian) ticks from the output file.
#
#The temporary Sim Mobility output format is only slightly better than mitsim's mess. We have, basically:
#    ("obj_type", frame_tick, object_id, {"prop":"value",...,})
#...where "frame_tick" is an integer and "object_id" is a (possible hex) integer. Properties and values define their own internal structures.


#We cheat a little and save all the lines we want to print later in the output file.
$SM_Lines_To_Print = []


module SM_ConvertSimMobOutput

#Useful numeric format regexes.
PosStr    = "\"([^\"]+)\"" #Todo, single quotes (later, now it's messing things up)
PosInt    = '([0-9]+)'
PosHexInt = '(0?x?[0-9a-fA-F]+)'  #Todo, this is currently weak; it allows, e.g., "xA". 
PosDbl    = '([0-9\.]+)'

#Our first regex applies to the "header"
HeaderStr = " *[(] *#{PosStr} *, *#{PosInt} *, *#{PosHexInt} *, *{([^}]+)} *[)] *"
HeaderRegex = Regexp.new(HeaderStr)

#Regex for properties
PropStr = "#{PosStr} *: *#{PosStr} *,? *"
PropRegex = Regexp.new(PropStr)

#Regex for polyline points
PointStr = "[(] *#{PosInt} *, *#{PosInt} *[)] *,? *"
PointRegex = Regexp.new(PointStr)


#Various helpers grouped into a class
class Helper

  #Helper; turn a list of "key":"value" pairs into a regular hash
  def parsePropsStr(propsStr)
    res = {}
    propsStr.scan(PropRegex){|m|
      res[m[0]] = m[1]
    }
    return res
  end

  def buildRevLookup(msNodes)
    res = {}
    msNodes.values.each{|msNode|
      res[msNode.sm_node.aimsunID] = msNode if msNode.sm_node
    }
    return res
  end

  def buildSegPairLookup(msSegs)
    res = {}
    msSegs.values.each{|msSeg|
      res[nodePairID_i(msSeg.upNode,msSeg.downNode)] = msSeg
    }
    return res
  end

  def nodePairID(n1, n2)
    return nodePairID_i(n1.nodeID, n2.nodeID)
  end

  def nodePairID_i(str1, str2)
    return "#{str1}::#{str2}"
  end

  def ensureProps(props, required)
    required.each{|prop|
      raise "Missing property: #{prop}" unless props.has_key? prop
    }
  end

  #Parse lane line zero into a series of SimMob::Point objects
  def parsePoints(polyStr)
    res = []
    polyStr.scan(PointRegex) {|pt|
      res.push(SimMob::Point.new(pt[0].to_f, pt[1].to_f))
    }
    return res
  end
end


def self.parse_node(frameID, objID, props, revNodeLookup, simmobNodeLookup)
  help = Helper.new()

  #Make sure we have all the properties we need
  help.ensureProps(props, ['xPos', 'yPos'])
  return unless props.has_key? 'aimsun-id'

  #Get
  xPos = props['xPos']
  yPos = props['yPos']
  aimID = props['aimsun-id']

  #Find
  return unless revNodeLookup.has_key? aimID
  node = revNodeLookup[aimID].sm_node
  return unless node #shouldn't happen
  
  #Set
  raise "Node already set: #{aimID}" if node.pos
  node.pos = SimMob::Point.new(xPos, yPos)
  simmobNodeLookup[objID] = node
end


def self.parse_segment(frameID, objID, props, revNodeLookup, simmobNodeLookup, segPairLookup, simmobSegmentLookup)
  help = Helper.new()
  help.ensureProps(props, ['parent-link', 'max-speed', 'from-node', 'to-node'])
  return unless props.has_key? 'aimsun-id'
  segAimsunID = props['aimsun-id']

  #Try to retrieve the start/end nodes as mitsim nodes.
  startNode = simmobNodeLookup[props['from-node']]
  endNode = simmobNodeLookup[props['to-node']]
  return unless startNode and endNode
  startNode = revNodeLookup[startNode.aimsunID]
  endNode = revNodeLookup[endNode.aimsunID]
  return unless startNode and endNode

  #Now retrieve the segment
  seg = segPairLookup[help.nodePairID(startNode, endNode)]
  return unless seg

  #Add it
  raise "Segment already added: #{segAimsunID}" if seg.sm_segment
  seg.sm_segment = SimMob::Segment.new(segAimsunID)
  seg = seg.sm_segment

  #Now, set the remainder of this segment's (relevant) properties
  seg.startNode = startNode.sm_node
  seg.endNode = endNode.sm_node
  simmobSegmentLookup[objID] = seg
end


def self.parse_lane(frameID, objID, props, revNodeLookup, simmobSegmentLookup)
  help = Helper.new()
  help.ensureProps(props, ['parent-segment', 'line-0'])
  
  #Try to retrieve the relevant segment
  segID = props['parent-segment']
  seg = simmobSegmentLookup[segID]
  return unless seg
  raise "Polyline already set for segment: #{segID}" if seg.polyline

  #Save it
  poly = help.parsePoints(props['line-0'])
  seg.polyline = SimMob::Polyline.new(poly)
end


def self.read_output_file(outputFileName, nw, drivers)
  $SM_Lines_To_Print = []

  #Build a reverse lookup of Sim Mobility Nodes to Mitsim Nodes
  help = Helper.new()
  revNodeLookup = help.buildRevLookup(nw.nodes)
  simmobNodeLookup = {} #Stores SimMob pointer -> SM::Node
  segPairLookup = help.buildSegPairLookup(nw.segments)  #[start,end] => Segment, all mitsim
  simmobSegmentLookup = {} #Stores SimMob pointer -> SM::Segment

  #Parse the file
  File.open(outputFileName).each { |line|
    line.chomp!
    if m = line.match(HeaderRegex)
      #Retrieve
      type = m[1].downcase!
      frameID = m[2]
      objID = m[3]
      props = help.parsePropsStr(m[4])

      #Dispatch
      saveThis = true
      if type=='uni-node' or type=='multi-node'
        parse_node(frameID, objID, props, revNodeLookup, simmobNodeLookup)
      elsif type=='road-segment'
        parse_segment(frameID, objID, props, revNodeLookup, simmobNodeLookup, segPairLookup, simmobSegmentLookup)
      elsif type=='lane'
        parse_lane(frameID, objID, props, revNodeLookup, simmobSegmentLookup)
      elsif type=='link'
        #Maybe later? For now, save it anyway.
      elsif type=='driver' or type=='pedestrian' or type=='signal'
        #We've entered the update portion. Definitely no need to go beyond here.
        break
      else
        saveThis = false
      end
      
      $SM_Lines_To_Print.push(line) if saveThis

    else
#      puts "Skipping line: #{line}"
    end

  }
end

end #module SM_ConvertSimMobOutput
























