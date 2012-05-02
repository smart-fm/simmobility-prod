#
#This module contains all classes which are specific to Sim Mobility. 
#
#The only way to go from a "Mitsim" object to a "Sim Mobility" version of that object
#  is to use the "sm_X" property of each object. 
#
#By extension, even if a SimMob.Node has a "Point(x,y)", it will NOT have the 
#  same value as the corresponding Node's Point(x,y).
#
#If you need to access either Sim Mobility or Mitsim components in a high-level way, use the 
#  corresponding "RoadNetwork" class.
#
module Mitsim

class RoadNetwork
  def initialize()
    #List of various road components, indexed by ID.
    @nodes = {}
    @segments = {}
    @links = {}
    @sm_network = nil
  end

  attr_accessor :nodes
  attr_accessor :segments
  attr_accessor :links
  attr_accessor :sm_network

  #Helper methods for making a new item and ensuring it is unique
  #  They also convert the ID explicitly to a string; we've had some trouble with 130 != "130"
  def newLink(id)
    id = id.to_s
    raise "Duplicate Link id: #{id}" if @links.has_key? id
    res = Link.new(id)
    @links[id] = res
    return res
  end
  def newSegment(id)
    id = id.to_s
    raise "Duplicate Segment id: #{id}" if @segments.has_key? id
    res = Segment.new(id)
    @segments[id] = res
    return res
  end
  def newNode(id)
    id = id.to_s
    raise "Duplicate Node id: #{id}" if @nodes.has_key? id
    res = Node.new(id)
    @nodes[id] = res
    return res
  end

  #Helper for getting/adding a Node
  def getOrAddNode(id)
    id = id.to_s
    return newNode(id) unless @nodes.has_key? id
    return @nodes[id]
  end
end


class Point
  def initialize(x, y)
    @x = x
    @y = y
    @sm_point = nil
  end
  attr_accessor :x
  attr_accessor :y
  attr_accessor :sm_point

  def to_s()
    return "(#{x},#{y})"
  end
end


class Node
  def initialize(nodeID)
    @nodeID = nodeID
    @pos = nil
    @sm_node = nil
  end  

  attr_reader :nodeID
  attr_accessor :pos
  attr_accessor :sm_node
end



class Segment
  def initialize(segmentID)
    @segmentID = segmentID
    @startPos = nil
    @endPos = nil
    @upNode = nil
    @downNode = nil
    @parentLink = nil
    @lanes = {}  #<lane id> => <position, from left to right>  --not sure how driving side is implemented.
    @sm_segment = nil
  end  

  attr_reader :segmentID
  attr_accessor :startPos
  attr_accessor :endPos
  attr_accessor :upNode
  attr_accessor :downNode
  attr_accessor :parentLink
  attr_accessor :sm_segment
  attr_accessor :lanes
end



class Link
  def initialize(linkID)
    @linkID = linkID
    @upNode = nil
    @downNode = nil
    @segments = [] #Listed upstream to downstream
    #@sm_link = nil  #Later
  end  

  attr_reader :linkID
  attr_accessor :upNode
  attr_accessor :downNode
  attr_accessor :segments
end


#A driver at a given time tick
class DriverTick
  def initialize(driver)
    @driver = driver
    @pos = nil
    @angle = nil
    @sm_angle = nil
  end

  attr_reader   :driver #The actual driver object
  attr_accessor :pos #Translated position of that driver
  attr_accessor :angle
  attr_accessor :sm_angle  #Angle in SimMob. We break encapsulation a bit here because I don't want to add properties to integers.
end


#A driver in the mitsim output
class Driver
  def initialize(agentID)
    @agentID = agentID
    @departure = 0
    @arrival = 0
    @completed = false
    @originNode = nil
    @destNode = nil
    @vehicleType = nil
    @hasAtLeastOneTick = nil

    @tempVeh2 = nil #Something boolean
    @tempVeh6 = nil #Usually equals the destination node except in very rare cases (~38 of them)

    @tempVeh3 = nil #Might be distance in meters
    @tempVeh9 = nil #Same

    @tempVeh10 = nil #Almost definitely speed in m/s
    @tempVeh11 = nil #Always zero
  end  

  attr_reader :agentID
  attr_accessor :departure
  attr_accessor :arrival
  attr_accessor :completed

  attr_accessor :originNode
  attr_accessor :destNode
  attr_accessor :vehicleType

  attr_accessor :hasAtLeastOneTick

  attr_accessor :tempVeh2
  attr_accessor :tempVeh3
  attr_accessor :tempVeh6
  attr_accessor :tempVeh9
  attr_accessor :tempVeh10
  attr_accessor :tempVeh11
end







end #module Mitsim

