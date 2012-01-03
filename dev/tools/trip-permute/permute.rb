#!/usr/bin/ruby

#This script builds a list of nodes from a Sim Mobility output file and permutes a trip from each 
#  node to each other node. The <drivers> section of a Sim Mobility input file is then generated, 
#  which can be run with check_bad_paths set to true, and output redirected to a file. This output
#  file can then be combined with the original input file via the script generate.rb

class Point
  def initialize(x,y)  
    @x,@y = x,y  
  end  
  
  attr_reader :x, :y  
end


def run_main()
  #Requires one arg
  if ARGV.empty?
    puts "Permute a list of trips. \nUsage: \n  #{__FILE__}   <in_file>\n...where <in_file> is a Sim Mobility output file." 
    return
  end

  #Open, read. Sample line:
  #("uni/multi-node", 0, 0xa4dca20, {"xPos":"37289787","yPos":"14381483","aimsun-id":"116724",})
  allpoints = []
  File.open(ARGV[0]).each { |line|
    if line =~ /\("(uni-node|multi-node)"[^{]+\{"xPos":"([^"]+)" *, *"yPos":"([^"]+)" *, *"aimsun-id":"([^"]+)" *,? *\} *\) */
      allpoints.push Point.new($2, $3)
    end
  }

  #Process further?
  if allpoints.empty?
    puts 'Empty file'
    return
  end

  #Else, generate an Agent for each permutation
  puts '<drivers>'
  for i in 0 ... allpoints.size
    for i2 in 0 ... allpoints.size
      next if i==i2
      puts "  <driver originPos=\"#{allpoints[i].x}, #{allpoints[i].y}\" destPos=\"#{allpoints[i2].x}, #{allpoints[i2].y}\" time=\"0\"/>" 
    end
  end
  puts '</drivers>'

end


if __FILE__ == $PROGRAM_NAME
  run_main()
end



