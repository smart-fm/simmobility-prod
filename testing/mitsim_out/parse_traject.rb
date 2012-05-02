require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'

#This parses a trajectory file, which contains the following 8 values:
#  time 
#  agentID 
#  segmentID
#  laneID 
#  positionInLane 
#  speed 
#  acceleration 
#  vehicleType
#
#The goal is to convert this to a series of time ticks (only one, if simple parsing is enabled)
# which contain actual (mitsim) x,y positions for each driver at that tick.
#


module MS_TrajectoryParser


def self.read_traj_file(trajFileName, nw, timeticks, drivers)
  lastKnownTime = 0
  unknownNodes = []
  unknownSegments = []
  File.open(trajFileName).each { |line|
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9.]+) +([0-9e.\-]+) +([0-9]+)/
      #Save temporaries
      timeS = $1.to_i
      driverID = $2.to_i
      segmentID = $3
      laneID = $4
      posInLane = $5.to_f

      #Ensure the timestep is increasing
      raise "Error: time must be strictly increasing" if timeS<lastKnownTime
      lastKnownTime = timeS

      #Convert this time (s) to a "tick" value. 
      #TODO: For now, this is hardcoded.
      timeTick = timeS * 5    #ticks are 200ms, times are listed in seconds

      #Ensure this driver/segment/lane exist; ensure time tick is ready; ensure no duplicates
      raise "Unknown segment: #{segmentID}" unless nw.segments.has_key? segmentID
      raise "Unknown lane: #{laneID}" unless nw.segments[segmentID].lanes.has_key? laneID
      raise "Unknown driver id: #{driverID}" unless drivers.has_key? driverID
      timeticks[timeTick] = {} unless timeticks.has_key? timeTick
      raise "Driver (#{driverID}) already set for time: #{timeS}ms" if timeticks[timeTick].has_key? driverID

      #Set a new driver tick entry
      drivers[driverID].hasAtLeastOneTick = true
      newDrvTick = Mitsim::DriverTick.new(drivers[driverID])
      timeticks[timeTick][driverID] = newDrvTick

      #Now, determine its position at that time tick.
      #NOTE: This assumes that segment.startPos/endPos are consistent across all Node definitions
      #      (the parser will output a warning if this is not true).
      segment = nw.segments[segmentID]
      pos = DynamicVector.new(segment.startPos, segment.endPos)
      pos.scaleTo(posInLane)
      pos.translate()
      pos.flipNormal($FlipRight)
      pos.scaleTo(segment.lanes[laneID]*$LaneWidthMS + $LaneWidthMS/2.0)
      pos.translate()
      newDrvTick.pos = Mitsim::Point.new(pos.getX(), pos.getY())
      newDrvTick.angle = pos.getAngle()

      #Determine its comparable position in Sim Mobility
      smSeg = segment.sm_segment
      if smSeg
        #Get the total "percent" traveled in the mitsim segment, convert to SM
        smPolyDist = (posInLane*smSeg.polyline.length)/Distance(segment.startPos, segment.endPos)
        startPoly,endPoly,remDist = smSeg.polyline.getPolyPoints(smPolyDist)
        pos = DynamicVector.new(startPoly, endPoly)
        pos.scaleTo(smPolyDist)
        pos.translate()
        pos.flipNormal($FlipRight)
        pos.scaleTo(segment.lanes[laneID]*$LaneWidthSM + $LaneWidthSM/2.0)
        pos.translate()
        newDrvTick.pos.sm_point = SimMob::Point.new(pos.getX(), pos.getY())
        newDrvTick.sm_angle = pos.getAngle()
      else
        unknownSegments.push(segmentID) unless unknownSegments.include? segmentID
      end
      
      #Consider unknown nodes
      unless segment.upNode.include? ':'  #We don't consider uni-nodes for now.
        unless nw.nodes.has_key? segment.upNode
          unknownNodes.push(segment.upNode)
        end
      end
    else 
      puts "Skipped line: #{line}"
    end
  }

  puts "Unknown node IDs: #{unknownNodes.uniq}" unless unknownNodes.empty?
  puts "Segment IDs missing conversion: #{unknownSegments}" unless unknownSegments.empty?

end

end #module MS_TrajectoryParser

