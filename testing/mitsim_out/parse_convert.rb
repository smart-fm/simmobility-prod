require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'



module MS_ConvertParser


#Various helpers grouped into a class
class Helper

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

end #class Helper



#Read our (manually built) Node conversion file.
def self.read_convert_file(convFileName, nw, drivers)
  help = Helper.new
  File.open(convFileName).each { |line|
    #Skip comments
    next if line =~ /^#/ or line.strip.empty?

    #Lines of the form "38 => 98088" specify that Mitsim node (38) is mapped to
    #  Sim Mobility node (98088)
    if line =~ /([0-9]+) *=> *([0-9]+)/
      help.ms_sm_nodeset(nw, $1, $2)

    #It's also common for Mitsim to "break up" Sim Mobility nodes into several pieces:
    #  {111,1111} => 98226
    elsif line =~ /{((?:[0-9:]+[, ]*)+)} *=> *([0-9]+)/
      mitsim_ids = $1
      simmob = $2
      mitsim_ids.scan(/[0-9:]+/) {|line|
        help.ms_sm_nodeset(nw, line, simmob)
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
    #TODO: This could go somewhere else, like in the "final validate" step.
    #puts "Node ID not translated: #{nd.aimsunID}" unless nd.pos
  }
end


end #module MS_ConvertParser

