#!/usr/bin/ruby

#This script builds a list of nodes from a Sim Mobility output file and permutes a trip from each 
#  node to each other node. The <drivers> section of a Sim Mobility input file is then generated, 
#  which can be run with check_bad_paths set to true, and output redirected to a file. This output
#  file can then be combined with the original input file via the script generate.rb
#
#NOTE: check_bad_paths doesn't really work any more. We'll need a different way of handling this; 
#      perhaps by scanning for Agent errors.

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
  from_pts = []
  to_pts = []
  File.open(ARGV[0]).each { |line|
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

  #Else, generate an Agent for each permutation
  puts '<drivers>'
  for i in 0 ... from_pts.size
    for i2 in 0 ... to_pts.size
      next if from_pts[i].x==to_pts[i2].x and from_pts[i].y==to_pts[i2].y
      puts "  <driver originPos=\"#{from_pts[i].x}, #{from_pts[i].y}\" destPos=\"#{to_pts[i2].x}, #{to_pts[i2].y}\" time=\"0\"/>" 
    end
  end
  puts '</drivers>'

end


if __FILE__ == $PROGRAM_NAME
  run_main()
end



