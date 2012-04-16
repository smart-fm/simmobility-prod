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
end


class Node
  def initialize(nodeID)
    @aimsunID = nodeID
    @pos = Point.new(0, 0)
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
    @sm_segment = nil
  end  

  attr_reader :segmentID
  attr_accessor :startPos
  attr_accessor :endPos
  attr_accessor :upNode
  attr_accessor :downNode
  attr_accessor :parentLink
  attr_accessor :sm_segment
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

    @firstPos = nil

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

  attr_accessor :firstPos

  attr_accessor :originNode
  attr_accessor :destNode
  attr_accessor :vehicleType

  attr_accessor :tempVeh2
  attr_accessor :tempVeh3
  attr_accessor :tempVeh6
  attr_accessor :tempVeh9
  attr_accessor :tempVeh10
  attr_accessor :tempVeh11
end







end #module Mitsim

