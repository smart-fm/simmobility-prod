#This module contains all classes which are specific to Sim Mobility. See "mitsim_classes.rb" for more details.
module SimMob


class RoadNetwork
  def initialize()
    #List of various road components, indexed by ID.
    @nodes = {}
  end

  attr_accessor :nodes

  #Helper methods for making a new item and ensuring it is unique
  #  They also convert the ID explicitly to a string; we've had some trouble with 1 != "1"
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
  end
  attr_accessor :x
  attr_accessor :y
end


class Node
  def initialize(aimsunID)
    @aimsunID = aimsunID
    @pos = nil
  end  

  attr_reader :aimsunID
  attr_accessor :pos
end



class Segment
  def initialize(aimsunID)
    @aimsunID = aimsunID

    #Converted property: only lane line zero matters for now
    @polyline = nil

    #Some properties to enable, later.
    @startNode = nil
    @endNode = nil
    #@parentLink = nil
    #@isFwd = nil  #Probably don't need this
  end  

  attr_reader   :aimsunID
  attr_accessor :polyline
  attr_accessor :startNode
  attr_accessor :endNode
end


class Polyline
  attr_reader   :length

  def initialize(points)
    #Create a list of polyline pairs, counting the length as you go
    @length = 0
    @points = []
    (0...points.length-1).each{|id|
      pt = PolyPoint.new(points[id], points[id+1])
      @points.push(pt)
      @length += pt.length
    }
  end

  #Return the two points (start, end) and the remaining length left after traversing to
  #  "distAlong" length on the polyline. Note that points exactly ON a polypoint are somewhat 
  #  indeterminate, but this shouldn't affect the result of this algorithm.
  def getPolyPoints(distAlong)
    resID = 0
    remDistance = distAlong
    loop do
      #Stopping condition
      break if resID>=@points.length or remDistance <= @points[resID].length

      #Increment
      resID += 1
      remDistance -= @points[resID].length
    end
    raise "Couldn't find polypoint at length #{distAlong} in polyline of length #{@length}" if resID>=@points.length

    return @points[resID].startPt, @points[resID].endPt, remDistance
  end
end

class PolyPoint
  attr_reader :startPt
  attr_reader :endPt
  attr_reader :length

  def initialize(startPt, endPt)
    @startPt = startPt
    @endPt = endPt
    @length = Distance(startPt, endPt)
  end
end



#Later:
#class Link
#  def initialize()
#  end  
#end




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







end #module SimMob

