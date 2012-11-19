
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
    @origItems = {}    #physID=>LogItem, origFile
    @newItems = {}     #physID=>LogItem, newFile
    @compareItems = {} #virtID=>LogLitemPair; nil = no comparison
  end

  attr_accessor :origItems
  attr_accessor :newItems
  attr_accessor :compareItems
end

#A "log item", indexed by ID
class LogItem
  def initialize(physID, virtID, origLine)
    @physID = physID      #The {type=>props string}
    @virtID = virtID      #The physID, with all IDs inside that replaced by their virtual IDs.
    @origLine = origLine  #The line used to build this 
  end

  attr_reader :physID
  attr_reader :virtID
  attr_reader :origLine
end

#A pair of log items
LogItemPair = Struct.new(:origItem, :newItem)


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
        newPropStr = "#{m.pre_match}#{repKey.virtID}#{m.post_match}"
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


#origFile specifies if we are building the "original" or the "new" file's results.
def read_file(logRes, fileName, origFile)
  #Our list of "unparsed" lines
  unprocessed = []

  #Which array do we populate?
  items = origFile ? logRes.origItems : logRes.newItems

  #Parse the file once, looking for IDs
  File.open(fileName).each { |line|
    line.chomp!
    if m = line.match(HeaderRegex)
      #Skip "sd-vertex" and "sd-edge" for now
      next if m[1].downcase == "sd-vertex"
      next if m[1].downcase == "sd-edge"

      #Save the ID
      items[m[3]] = nil

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
      raise "Duplicate ID (should be unique) (#{objID})" if items[objID]

      #Attempt to get an ID for this object
      virtObjID = getVirtId(items, type, props)
      if (virtObjID)
        items[objID] = LogItem.new(objID, virtObjID, line)
      else
        remaining.push(line)
      end
    }

    #Ensure we aren't looping forever
    raise "Infinite loop" if remaining.length >= unprocessed.length
    unprocessed = remaining
  end

  #Now build our half of the LogRes array
  items.each{|key, val|
    raise "Unexpected leftover null value" unless val
    unless logRes.compareItems.has_key? val.virtID
      logRes.compareItems[val.virtID] = LogItemPair.new
    end

    raise "Duplicate key (1)" if origFile and logRes.compareItems[val.virtID].origItem
    raise "Duplicate key (2)" if !origFile and logRes.compareItems[val.virtID].newItem

    if origFile
      logRes.compareItems[val.virtID].origItem = val
    else
      logRes.compareItems[val.virtID].newItem = val
    end
  }
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

  #Scan the results
  origHas = []
  newHas = []
  res.compareItems.each{|key, value|
    next if value.origItem and value.newItem
    raise 'Unexpected double-null' unless value.origItem or value.newItem

    origHas.push(value.origItem) if value.origItem
    origHas.push(value.newItem) if value.newItem
  }

  #Display results
  puts "Missing in new file: (#{origHas.size}/#{res.compareItems.size})"
  origHas.each{|item| puts item.origLine }
  puts "Extra lines in new file: (#{newHas.size}/#{res.compareItems.size})"
  newHas.each{|item| puts item.origLine }

  puts 'Done'
end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




