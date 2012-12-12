
def print_usage()
  puts 'Use this script to update an "old" (fwd/back) XML road network to the new version:'
  puts '  ruby split.rb  Input.xml'
  puts 'This is a somewhat fragile comparison method: it is based heavily on the exact'
  puts 'text structure of the XML files. Use at your own risk.'
end

#Simple Link class
class Link
  def initialize()
    @id = nil
    @start = nil
    @endN = nil
    @name = nil
  end

  def reset()
    @id = nil
    @start = nil
    @endN = nil
    @name = nil
  end

  attr_accessor :id
  attr_accessor :start
  attr_accessor :endN
  attr_accessor :name
end


def scan_line(outfile, line, currLink)
  print_line = true

  #New Link?
  if line =~ /<Link>/
    currLink.reset
  elsif line =~ /<linkID>1([0-9]+)<\/linkID>/
    currLink.id = "2#{$1}" #Reverse IDs start with a 2
  elsif line =~ /<roadName>([^<]+)<\/roadName>/
    currLink.name = $1 #No change here
  elsif line =~ /<StartingNode>([0-9]+)<\/StartingNode>/
    currLink.endN = $1 #Flip end and start
  elsif line =~ /<EndingNode>([0-9]+)<\/EndingNode>/
    currLink.start = $1 #Flip end and start
  elsif line =~ /<\/Segments>/ or line =~ /<\/Link>/
    #We can't rely on this, since BKDseg may not exist.
    print_line = false
  elsif line =~ /<FWDSegments>/ || line =~ /<\/FWDSegments>/ || line =~ /<BKDSegments>/ || line =~ /<\/BKDSegments>/
    #Don't print this line
    print_line = false

    #Make sure we have all our data
    raise "Incomplete data" unless currLink.id and currLink.start and currLink.endN and currLink.name

    #Possible additional printings
    if line =~ /<\/FWDSegments>/ or line =~ /<\/BKDSegments>/
      #Close up this link
      outfile.puts "#{' '*20}</Segments>"
      outfile.puts "#{' '*18}</Link>"
    elsif line =~ /<BKDSegments>/
      #Start up a new Link
      outfile.puts "#{' '*18}<Link>"
      outfile.puts "#{' '*20}<linkID>#{currLink.id}</linkID>"
      outfile.puts "#{' '*20}<roadName>#{currLink.name}</roadName>"
      outfile.puts "#{' '*20}<StartingNode>#{currLink.start}</StartingNode>"
      outfile.puts "#{' '*20}<EndingNode>#{currLink.endN}</EndingNode>"
      outfile.puts "#{' '*20}<Segments>"
    end
  end

  #Print this line?
  outfile.puts(line) if print_line
end


def copy_and_split(infile, outfile) 
  #Are we tracking links?
  tracking = false

  #info about the current link
  currLink = Link.new

  #For each line...
  infile.each{|line|
    if tracking
      #Stop tracking?
      if line =~ /<\/Links>/
        tracking = false
        outfile.puts(line)
      else
        scan_line(outfile, line, currLink)
      end
    else
      #Begin tracking?
      tracking = true if line =~ /<Links>/
      outfile.puts(line)  #Copy regardless
    end
  }
end


def run_main()
  #Simple
  if ARGV.length < 1
    print_usage()
    exit
  end

  #Open a file for output
  File.open("output.xml", "w"){ |outfile| 
    copy_and_split(File.open(ARGV[0]), outfile)
  }

  puts 'Done'
end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




