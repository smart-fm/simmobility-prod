require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'

#This parses an output file from Sim Mobility, taking Node and Segment information in as 
#  required. It operates fastest if you remove all non-network (driver/pedestrian) ticks from the output file.
#
#The temporary Sim Mobility output format is only slightly better than mitsim's mess. We have, basically:
#    ("obj_type", frame_tick, object_id, {"prop":"value",...,})
#...where "frame_tick" is an integer and "object_id" is a (possible hex) integer. Properties and values define their own internal structures.

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

  def ensureProps(props, required)
    required.each{|prop|
      raise "Missing property: #{prop}" unless props.has_key? prop
    }
  end
end


def self.parse_node(frameID, objID, props, revNodeLookup)
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
end


def self.read_output_file(outputFileName, nw, drivers)
  #Try not to flood the console with warnings.
  alreadyWarned = false

  #Build a reverse lookup of Sim Mobility Nodes to Mitsim Nodes
  help = Helper.new()
  revNodeLookup = help.buildRevLookup(nw.nodes)

  #Parse the file
  File.open(outputFileName).each { |line|
    line.chomp!
    if m = line.match(HeaderRegex)
      #Retrieve
      type = m[1]
      frameID = m[2]
      objID = m[3]
      props = help.parsePropsStr(m[4])

      #Dispatch
      if type=='uni-node' or type=='multi-node'
        parse_node(frameID, objID, props, revNodeLookup)
      elsif type=='road-segment'
        parse_segment(frameID, objID, props, revNodeLookup)
      elsif type=='lane'
        parse_lane(frameID, objID, props, revNodeLookup)
      elsif type=='driver' or type=='pedestrian' or type=='signal'
        unless alreadyWarned
          puts "NOTE: Your Sim Mobility output class contains agent tick data."
          puts "      You can significantly speed up reading of this log file by"
          puts "      removing everything except the network specification."
          alreadyWarned = true
        end
      end
      


    else
      puts "Skipping line: #{line}"
    end

  }
end

end #module SM_ConvertSimMobOutput
























