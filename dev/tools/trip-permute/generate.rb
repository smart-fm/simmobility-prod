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
#    2,3) Start time, end time of Agent trip generation (note: for now, start time must be the same as the simulation's start time)
#    4) Random seed (optional, uses current system time otherwise)
#
#Files:
#   in.txt => normal sim mobility output file (normally called out.txt)


class Trip
  def initialize(fromX,fromY,toX,toY)
    @fromX,@fromY,@toX,@toY = fromX,fromY,toX,toY
  end  
  
  attr_reader :fromX,:fromY,:toX,:toY
end


def read_date(val)
  return -1 unless val =~ /([0-9]?[0-9]):([0-9][0-9])/
  mins = $1.to_i*60 + $2.to_i
  return mins*60*1000
end


def run_main()
  #Requires several args
  if ARGV.length<5 or ARGV.length>6
    print_usage()
    return
  end

  #Read the number of agents
  num_agents = ARGV[2].to_i

  #Parse the dates
  start_ms = read_date ARGV[3]
  end_ms = read_date ARGV[4]
  if start_ms==-1 or end_ms==-1
    puts "Bad time: #{ARGV[3]} or #{ARGV[4]}"
    return
  end

  #Random seed
  seed = if ARGV.length>5 then ARGV[5].to_i else Time.new.to_i end
  srand seed

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
  puts "<!-- Trips: #{trips.length} -->"
  puts "<!-- Skipped: #{skipped} -->"
  puts "<!-- Random seed: #{seed} -->"
  puts "<!-- Num agents: #{num_agents} -->"

  #Generate random agents at random start times
  puts '<drivers>'
  for agID in (1..num_agents) 
      trip = trips[rand(trips.length)]
      start = rand(end_ms - start_ms)
      puts "  <driver originPos=\"#{trip.fromX}, #{trip.fromY}\" destPos=\"#{trip.toX}, #{trip.toY}\" time=\"#{start}\"/>" 
  end
  puts '</drivers>'

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



