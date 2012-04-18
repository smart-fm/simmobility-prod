require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'


#Mitsim has a somewhat convoluted, quasi-json format which it uses for storing the network. 
#Basically, it looks something like this:
#
#[Nodes] : 32
#{ # {NodeID Type 'Name'} 
#        {641 1 "641"}
#        {701 1 "701"}
#...(more nodes)
#}
#
#[Links] : 42 : 76 : 266
#{ # Link: //Links Are listed in arbitrary order 
## {LinkID LinkType UpNodeID DnNodeID LinkLabelID 
## 
## Segment: //Segments are listed from upstrm to dnstrm within their link  
##      {SegmentID DefaultSpeedLimit FreeSpeed Grad 
##        {StartingPntX StartingPntY Bulge EndPntX EndPntY } 
## Lane: // Lanes are listed from left to right 
##            {LaneID Rules} 
##      } // end of segment 
##    } // end of link 
#
#	{1 3 9000 72 0
#		{10 35 35 0
#			{3.03304e+04 3.12078e+04 0.00 3.03592e+04 3.11839e+04}
#			{101 0}
#			{102 0}
#			{103 1}
#		} //end of segment
#		{11 35 35 0
#			{3.03592e+04 3.11839e+04 0.00 3.04189e+04 3.11229e+04}
#			{111 0}
#			{112 0}
#			{113 1}
#			{114 1}
#		} //end of segment
#	} //end of link
#...(more links/segments)
#
#A few issues complicate easy parsing of this format:
#  1) C, C++ and Perl-style comments are allowed, including multi-line C-style. 
#  2) Segments flow from Node to Node, but they list their own (x,y) positions
#     rather than storing them in the Nodes. This may lead to subtle inconsistencies.
#  3) Segments have no "UniNode" ids, which makes identifying them outside of a simulation run difficult.
#
#To solve these issues, we attempt the following:
#  1) We perform two-phase parsing of the input text. The first phase strips all comments from the source,
#     and the second phase operates on this pruned string. 
#  2) We perform consistency checks, and display various warnings depending on how severe the detected 
#     inconsistencies are.
#  3) We use an intermediate labelling format for each UniNode; essentially, "<LinkID>:<UniNodePosition>".
#     For example, Link 106 would have Uni Nodes labeled "106:1", "106:2", etc.
#


module MS_NetworkParser

#Useful numeric format regexes.
PosInt = '([0-9]+)'
PosDbl = '([0-9\.]+)'
PosSci = '([0-9]+\.[0-9]+e[-+][0-9]+)'

#Links start with:  {1 2 3 4 5   #LinkID LinkType UpNodeID DnNodeID LinkLabelID 
LinkHead = "{ ?#{PosInt} #{PosInt} #{PosInt} #{PosInt} #{PosInt} ?"

#After that is at least one segment: {1 2 3 4  #SegmentID DefaultSpeedLimit FreeSpeed Grad 
SegmentHead = "{ ?#{PosInt} #{PosInt} #{PosInt} #{PosInt} ?"

#After that is exactly one descriptor: {1.0e+0 2.0e+0 3.00 4.0e+0 5.0e+0}  #StartingPntX StartingPntY Bulge EndPntX EndPntY
SegmentDesc = "{ ?#{PosSci} #{PosSci} #{PosDbl} #{PosSci} #{PosSci} ?} ?"

#After that is at least one lane rules group: {1, 2}
LaneRule = "{ ?#{PosInt} #{PosInt} ?} ?"

#Close each segment with }
SegmentStr = "#{SegmentHead}#{SegmentDesc}((#{LaneRule})+}) ?"
SegmentRegex = Regexp.new(SegmentStr)

#Close the Link with }
LinkRegex = Regexp.new("#{LinkHead}((#{SegmentStr})+}) ?")


#Various helpers grouped into a class
class Helper

  #Helper; make sure all nodes are given unique, sensible IDs.
  def attemptToGenerateID(nw, linkID, pos, nodeID, currSubID)
    #If no Node ID exists, generate a node and add it to the node list
    unless nodeID
      #Somewhat fragile, but should be good enough
      raise "Too many segments" if currSubID[0]>999
  
      #Set the ID (string)
      nodeID = "#{linkID}:#{currSubID[0]}"
      currSubID[0] += 1
  
      #Make the node
      nd = nw.newNode(nodeID)
      nd.pos = Mitsim::Point.new(pos.x, pos.y)
    end
  
    #Return the (new?) ID
    return nodeID
  end


  #Helper: parse a file, removing C, C++, and Perl-style comments.
  def parse_file_remove_comments(fileName)
    stream = ''
    onComment = false
    File.open(fileName).each { |line|
      line.chomp!
      if onComment
        #Substitute the comment closing and set onComment to false
        if line.sub!(/^((?!\*\/).)*\*\//, '') #C style, closing
          onComment = false
        else
          line = '' #Still in the comment
        end
      end
      unless onComment
        #Single line regexes are easy
        line.sub!(/#.*$/, '')  #Perl style
        line.sub!(/\/\/.*$/, '')  #C++ style
        line.gsub!(/\/\*((?!\*\/).)*\*\//, '') #C style single line
  
        #Now multi-line C comments
        onComment = true if line.sub!(/\/\*((?!\*\/).)*$/, '')  #C style, line ending reached
      end
      stream << line
    }

    #Now, parse unrelated sections out of the stream.
    stream.sub!(/^.*(?=\[Links\])/, '')
    stream.sub!(/\[(?!Links)[^\]]+\].*/, '')
    stream.gsub!(/[ \t]+/, ' ') #Reduce all spaces/tabs to a single space
    stream.gsub!(/\[Links\][^{]*{/, '') #Avoid a tiny bit of lookahead. There will be an unmatched } at the end, but it won't matter
 
    return stream
  end

  #Either add a Node (by position) or, if it exists, ensure that it
  # hasn't moved much.
  def addOrCheckNode(nw, id, currentPos)
    nd = nw.getOrAddNode(id)
    unless nd.pos
      nd.pos = Mitsim::Point.new(currentPos.x, currentPos.y)
    else
      #Ensure the position hasn't changed.
      errorVal = Distance(nd.pos, currentPos)
      puts "Node ID/position mismatch[#{id}],\n  prev: #{nd.pos},\n  curr: #{currentPos}\n  error: #{errorVal})" if errorVal >= 10.0
      puts "Node ID/position mismatch[#{id}], off by #{errorVal} meters" if errorVal >= 1.0
    end
  end

end #class Helper



#Main method: parse a network file.
def self.read_network_file(nwFileName, nw)
  help = Helper.new

  #First, pre-process and remove comments. Apparently *all* comment types
  #  are valid for mitsim...
  bigstring = help.parse_file_remove_comments(nwFileName)
  bigstring.scan(LinkRegex) {|linkRes|
    linkID = linkRes[0]

    upNodeID = linkRes[2]
    downNodeID = linkRes[3]
    link = nw.newLink(linkID)
    link.upNode = upNodeID
    link.downNode = downNodeID

    #Now get segments
    #Segments are listed from upstream to downstream
    segmentStr = linkRes[5]
    prevEndPoint = nil
    segmentStr.scan(SegmentRegex) {|segRes|
      segID = segRes[0]
      segStartX = ParseScientificToFloat(segRes[4])
      segStartY = ParseScientificToFloat(segRes[5])
      segEndX = ParseScientificToFloat(segRes[7])
      segEndY = ParseScientificToFloat(segRes[8])

      #Save it temporarily
      seg = nw.newSegment(segID)
      seg.startPos = Mitsim::Point.new(segStartX, segStartY)
      seg.endPos = Mitsim::Point.new(segEndX, segEndY)
      seg.parentLink = link
      link.segments.push(seg)

      #Double-check consistency
      if prevEndPoint
        unless seg.startPos.x==prevEndPoint.x and seg.startPos.y==prevEndPoint.y
          #Skip errors less than 1 m
          errorVal = Distance(seg.startPos, prevEndPoint)
          raise "Segment consistency error on Segment: #{segID} Distance: #{errorVal}" if errorVal >= 1.0 
        end
      end
      prevEndPoint = seg.endPos
    }


    #Now set each segment's node ids and go from there
    currSubID = [1] #Node ID for segments.
    link.segments[0].upNode = upNodeID
    link.segments[-1].downNode = downNodeID

    #Force add Link start/end node
    help.addOrCheckNode(nw, link.segments[0].upNode, link.segments[0].startPos)
    help.addOrCheckNode(nw, link.segments[-1].downNode, link.segments[-1].endPos)

    #Add UniNodes for each segment. These will have generated names, with ":1" appended for the 
    # Node closest to upstream, and so forth.
    link.segments.each{|segment|
      #Attempt to generate an ID. Even if this fails, save this ID
      segment.upNode = help.attemptToGenerateID(nw, linkID, segment.startPos, segment.upNode, currSubID)
      segment.downNode = help.attemptToGenerateID(nw, linkID, segment.endPos, segment.downNode, currSubID)
    }
  }

end


end #module MS_NetworkParser

