#!/usr/bin/ruby

#  
#  This script is a combination of permute.rb and generate.rb. This script would serve larger networks better. If you were to use
#  permute.rb for a larger network (e.g.:-Singapore), it would generate a huge number of trip permutations from which generate.rb 
#  will then try to filter out n random trips, which would take a long time. Alternatively this script builds a list of nodes from
#  a SimMobility output file and generates n trips from randomly selected nodes to other randomly selected nodes. 
#  The <drivers> section of a SimMobility input file is then generated.
#  This script does not check for bad paths, so you might have to use another script in order to filter out the bad paths.
 
#Example usage (combined with generateForSingapore) would be:
#
#   ./generateForSingapore.rb 100000  8:00  9:00 in.txt > all_trips.txt
#   #Manually convert all_trips.txt into all_trips.xml 
#
#Arguments are:
#    1) Number of Agents to generate
#    2,3) Start time, end time of Agent trip generation (note: for now, start time must be the same as the simulation's start time)
#
#Files:
#   in.txt => normal sim mobility output file (normally called out.txt)
#   all_trips.txt => This is a generated file; it will contain a <drivers> section, which can be copied verbatim over the <drivers> section
#                    in the current config file (e.g., test_road_network.xml)

class Point
  def initialize(x,y)  
    @x,@y = x,y  
  end  
  
  attr_reader :x, :y  
end

def read_date(val)
  return -1 unless val =~ /([0-9]?[0-9]):([0-9][0-9])/
  mins = $1.to_i*60 + $2.to_i
  return mins*60*1000
end


def run_main()
  #Requires several args
  if ARGV.length<3 or ARGV.length>4
    print_usage()
    return
  end

  #Read the number of agents
  num_agents = ARGV[0].to_i

  #Parse the dates
  start_ms = read_date ARGV[1]
  end_ms = read_date ARGV[2]
  if start_ms==-1 or end_ms==-1
    puts "Bad time: #{ARGV[1]} or #{ARGV[2]}"
    return
  end

    #Open, read. Sample line:
  #("uni/multi-node", 0, 0xa4dca20, {"xPos":"37289787","yPos":"14381483","aimsun-id":"116724",})
  from_pts = []
  to_pts = []
  File.open(ARGV[3]).each { |line|
#    if line =~ /\("(uni-node|multi-node)"[^{]+\{"xPos":"([^"]+)" *, *"yPos":"([^"]+)" *, *"aimsun-id":"([^"]+)" *,? *\} *\) */
    if line =~ /\("(uni-node|multi-node)"[^{]+\{"xPos":"([^"]+)" *, *"yPos":"([^"]+)" *[^}]*\} *\) */
      unless $4.to_i == 70392
        from_pts.push Point.new($2, $3) if ($1=='multi-node') 
        to_pts.push Point.new($2, $3)
      end
    end
  }

  #Process further?
  if from_pts.empty? or to_pts.empty?
    puts 'Empty file(s)'
    return
  end

  #Some data
  puts "<!-- Num agents: #{num_agents} -->"

  #Generate random agents at random start times
  puts '<drivers>'
  for agID in (1..num_agents) 
      from_pt = from_pts[rand(from_pts.length)]
      to_pt = to_pts[rand(to_pts.length)]
      unless from_pt.x==to_pt.x and from_pt.y==to_pt.y
      	start = rand(end_ms - start_ms)
      	puts "  <driver originPos=\"#{from_pt.x}, #{from_pt.y}\" destPos=\"#{to_pt.x}, #{to_pt.y}\" time=\"#{start}\"/>" 
      end
  end
  puts '</drivers>'
end


def make_key(x1, y1, x2, y2)
  return "[#{x1},#{y1}=>#{x2},#{y2}]"
end


def print_usage()
    puts 'Generate a number of random trips.'
    puts "Usage: \n  #{__FILE__}   <num_agents>  <start_time>  <end_time> <in_file>"
    puts '  <num_agents> is >0 and specifies how many random agents to generate (all will be drivers)'
    puts '  <start_time> is the time to start generating Agents, hh:mm'
    puts '  <end_time> is the time to finish generating Agents, hh:mm'
    puts '  <in_file> is a Sim Mobility network output file'
    puts 'output: A list of <num_agents> drivers, uniformly distributed between all origins and all destinations and all possible start times.'
    puts 'Please see the source file for sample usage.'
end

if __FILE__ == $PROGRAM_NAME
  run_main()
end



