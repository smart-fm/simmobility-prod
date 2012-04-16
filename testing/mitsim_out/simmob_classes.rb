#This module contains all classes which are specific to Sim Mobility. See "mitsim_classes.rb" for more details.
module SimMob


class RoadNetwork
  def initialize()
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
    @pos = Point.new(0, 0)
  end  

  attr_reader :aimsunID
  attr_accessor :pos
end



class Segment
  def initialize(aimsunID)
    @aimsunID = aimsunID

    #Some properties to enable, later.
    #@startNode = nil
    #@endNode = nil
    #@isFwd = nil
    #@parentLink = nil
  end  

  attr_reader :aimsunID
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

