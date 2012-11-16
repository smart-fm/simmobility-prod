
def print_usage()
  puts 'This script can be used to compare two output files from sim Mobility:'
  puts '  ruby compare.rb  out_orig.txt  out_new.txt'
  puts 'Output items are compared by "virtual" ID, in as general a manner as possible'
  puts '(although it is not perfect; you may need to change this file if you'
  puts 'add a new output type). Following that, each addition, removal, or change is listed.'
end

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

#Simple regex for "possible IDs"
PosHexIntStrong = '0x[0-9a-f]+' #For now we take advantage of the fact that IDs are always hex; thus, we avoid potential scanning loops
PotentialID = Regexp.new(PosHexIntStrong)



#Class to hold our results as we build them.
class LogRes
  def initialize()
  end
end


#Helper: Remove parent-link from props
def fixProps(line)
  return line.gsub(/"parent-link":"[^"]+"/, '"parent-link":"<removed>"')
end


#Attepmt to create a virtual ID
#Will be of the form: 
#  {type=>propStr}
#...with each 0xID in propStry replaced with the corresponding virtual ID
def getVirtId(knownIDs, type, propsStr)
  newPropStr = propsStr
  offset = 0
  scanAgain = true

  #Find/replace IDs one by one
  while (scanAgain)
    scanAgain = false
    PotentialID.match(newPropStr, offset){|m|
      #Skip IDs we can never know about
      if knownIDs.has_key? m[0]
        #Easy case: Can't get the ID
        repKey = knownIDs[m[0]]
        return nil unless repKey

        #Slightly harder case: replace this and keep searching
        newPropStr = "#{m.pre_match}#{repKey}#{m.post_match}"
        offset = 0
        scanAgain = true
      else
        offset = m.pre_match.length + m[0].length
        scanAgain = true
      end
    }
  end

  #Success
  return "{#{type}=>#{newPropStr}}" 
end


#If buildRes is true, then logRes is updated with the Log lines. Otherwise, it is checked.
def read_file(logRes, fileName, buildRes)
  #Our list of "unparsed" lines
  unprocessed = []

  #Parse the file once, looking for IDs
  knownIDs = {}
  origLogLines = {}
  File.open(fileName).each { |line|
    line.chomp!
    if m = line.match(HeaderRegex)
      #Skip "sd-vertex" and "sd-edge" for now
      next if m[1].downcase == "sd-vertex"
      next if m[1].downcase == "sd-edge"

      #Save the ID
      knownIDs[m[3]] = nil

      #Save the line
      unprocessed.push(line)
    end
  }

  #Parse each line file again, this time attempting to generate a virtual ID for each real ID.
  until unprocessed.empty? do
    remaining = []
    unprocessed.each{ |line|
      #Retrieve
      raise "Unexpected regex mismatch" unless m = line.match(HeaderRegex)
      type = m[1].downcase
      frameID = m[2]
      objID = m[3]
      props = fixProps(m[4])

      #Ensure no ID, frame is zero
      raise "Non-zero frame number" unless frameID == '0'
      raise "Duplicate ID (should be unique) (#{objID})" if knownIDs[objID]

      #Attempt to get an ID for this object
      virtObjID = getVirtId(knownIDs, type, props)
      if (virtObjID)
        knownIDs[objID] = virtObjID
        origLogLines[objID] = line
      else
        remaining.push(line)
      end
    }

    #Ensure we aren't looping forever
    raise "Infinite loop" if remaining.length >= unprocessed.length
    unprocessed = remaining
  end

  #Now import (or check) these results
  if buildRes
  else
  end
end


def run_main()
  #Simple
  if ARGV.length < 2
    print_usage()
    exit
  end

  #Create a data structure to store results, and build (then compare) it.
  res = LogRes.new()
  read_file(res, ARGV[0], true)
  read_file(res, ARGV[1], false)

  puts 'Done'
end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




