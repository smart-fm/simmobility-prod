#!/usr/bin/ruby

#This script takes a list of all possible trips, combined with a blacklist, and generates X random trips within a given
#  time frame from these lists and a random seed.
#Example usage (combined with permute) would be:
#
#   ./permute.rb in.txt >all_trips.txt
#   #Manually convert all_trips.txt into all_trips.xml
#   ./SimMobility all_trips.txt /dev/null >blacklist.txt
#   ./generate.rb all_trips.txt blacklist.txt 4000 8:00 10:00 0
#
#Arguments after "blacklist" are:
#    1) Number of Agents to generate
#    2,3) Start time, end time of Agent trip generation
#    4) Random seed (optional, uses current system time otherwise)


def run_main()
end


if __FILE__ == $PROGRAM_NAME
  run_main()
end



