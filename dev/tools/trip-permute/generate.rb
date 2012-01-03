#!/usr/bin/ruby

#This script takes a list of all possible trips, combined with a blacklist, and generates X random trips within a given
#  time frame from these lists and a random seed.
#Example usage (combined with permute) would be:
#
#   ./permute.rb in.txt >all_trips.txt
#   #Manually convert all_trips.txt into all_trips.xml
#   ./SimMobility all_trips.xml /dev/null >blacklist.txt
#   ./generate.rb all_trips.txt blacklist.txt 4000 8:00 10:00 0
#
#Arguments after "blacklist" are:
#    1) Number of Agents to generate
#    2,3) Start time, end time of Agent trip generation
#    4) Random seed (optional, uses current system time otherwise)


class Trip
  def initialize(fromX,fromY,toX,toY)
    @fromX,@fromY,@toX,@toY = fromX,fromY,toX,toY
  end  
  
  attr_reader :fromX,:fromY,:toX,:toY
end


def run_main()
  #Requires several args
  if ARGV.length<5 or ARGV.length>6
    print_usage()
    return
  end

  #Build our blacklist, indexed by strings a la: "[x1,y1=>x2,y2]"
  #Blacklist entry sample: Skipping agent; (some text) {(37232859, 14308003)=>(37273805, 14362442)}
  blacklist = {}
  File.open(ARGV[1]).each { |line|
    if line =~ /Skipping agent;[^{]*\{\(([0-9]+), *([0-9]+)\)=>\(([0-9]+), *([0-9]+)\)\}/
      blacklist[make_key($1, $2, $3, $4)] = true
    end
  }

  #Stop early?
  if blacklist.empty?
    puts 'Empty blacklist file.'
    return
  end

  #Build our list of allowed routes, skipping blacklisted entry.
  #Sample:   <driver originPos="37232859, 14308003" destPos="37273805, 14362442" time="0"/>
  trips = []
  skipped = 0
  File.open(ARGV[0]).each { |line|
    if line =~ /<driver +originPos="([0-9]+), *([0-9]+)" +destPos="([0-9]+), *([0-9]+)" +time="[0-9]+" *\/>/
      tripKey = make_key($1, $2, $3, $4)
      unless blacklist.has_key? tripKey
        trips.push(Trip.new($1, $2, $3, $4))
      else 
        skipped += 1
      end
    end
  }

  #Some data
  puts "Trips: #{trips.length}"
  puts "Skipped: #{skipped}"
  

end


def make_key(x1, y1, x2, y2)
  return "[#{x1},#{y1}=>#{x2},#{y2}]"
end


def print_usage()
    puts 'Generate a number of random trips.'
    puts "Usage: \n  #{__FILE__}   <trips_file>  <blacklist_file>  <num_agents>  <start_time>  <end_time>  <rand_seed>"
    puts "where: \n  <trips_file> is a list of all possible trips"
    puts '  <blacklist_file> is a list of invalid trips'
    puts '  <num_agents> is >0 and specifies how many random agents to generate (all will be drivers)'
    puts '  <start_time> is the time to start generating Agents, hh:mm'
    puts '  <end_time> is the time to finish generating Agents, hh:mm'
    puts '  <rand_seed> is some seed value that allows the generation to be repeated. Defaults to the current system time'
    puts 'output: A list of <num_agents> drivers, uniformly distributed between all valid trips and all possible start times.'
    puts 'Please see the source file for sample usage.'
end

if __FILE__ == $PROGRAM_NAME
  run_main()
end



