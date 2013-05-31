from geo.point import Point
from geo.helper import assert_non_null


#Our container class
class RoadNetwork:
  '''The primary container class for Sim Mobility road networks.
     Note that, like all the "format" classes, and "id" is always
     a string (i.e., "123" not 123). Anything *except* the id is
     usually a reference (e.g., "link.fromNode"). Dictionaries usually
     use IDs as keys to make things simpler.
  '''

  def __init__(self):
    #Contains all Multi-Nodes and Uni-Nodes
    self.nodes = {}    #{nodeId => Node}

    #Contains all Links
    self.links = {}    #{linkId => Link}


class RNIndex:
  '''(Optional) index on a RoadNetwork'''

  def __init__(self, rn:'RoadNetwork'):
    #List of all Lane connectors in the network.
    self.lane_connectors = {} #{nodeId => [LaneConnector]}
    self.lanes = {}    #{laneId => Lane}

    #Now construct them.
    self.__build_index(rn)

  def __build_index(self, rn):
    #Get to the segment level.
    for lk in rn.links.values():
      for sg in lk.segments:
        #Index all Lanes
        for ln in sg.lanes:
          lanes[ln.laneId] = ln

        #Index all Connectors
        for lcGrp in sg.lane_connectors.values():
          for lc in lcGrp:
            if not (sg.toNode in lane_connectors):
              lane_connectors[sg.toNode] = []
            lane_connectors[sg.toNode].append(lc)


class Node:
  '''A Node. All Nodes are UniNodes unless subclassed (e.g., as Intersection).'''

  def __init__(self, nodeId, xPos, yPos):
    assert_non_null("Null args", nodeId, xPos, yPos)
    self.nodeId = str(nodeId)
    self.pos = Point(float(xPos), float(yPos))


class Intersection(Node):
  ''' An Intersection is more flexible than a Node, but this flexibility is
      expressed in its usage (not its data) so for now it's just a simple subclass.
  '''

  def __init__(self, nodeId, xPos, yPos):
    super(Inersection,self).__init__(nodeId, xPos, yPos)


class Link:
  def __init__(self, linkId, fromNode, toNode):
    assert_non_null("Null args", linkId, fromNode, toNode)
    self.linkId = str(linkId)
    self.fromNode = fromNode
    self.toNode = toNode
    self.segments = [] #List of segments in order

class Segment:
  def __init__(self, segId, fromNode, toNode):
    assert_non_null("Null args", edgeId, fromNode, toNode)
    self.segId = str(segId)
    self.fromNode = fromNode
    self.toNode = toNode
    self.lanes = []  #Lanes in order (closest-to-median first)
    self.lane_edges = [] #Lane edges in order (median first)
    self.lane_connectors = {} #{toSegId => [LaneConnector]}

class LaneConnector:
  '''A lane connector *from* a given Lane in a given Segment *to* a given Lane in a given Segment.'''

  def __init__(self, fromSegment, toSegment, fromLane, toLane):
    self.fromSegment = fromSegment
    self.toSegment = toSegment
    self.fromLane = fromLane
    self.toLane = toLane

class Lane:
  '''A Lane. Drivers will drive on Lanes.'''

  def __init__(self, laneId, laneNumber, parentSeg, polyline):
    assert_non_null("Null args", laneId, laneNumber, parentSeg, polyline)
    self.laneId = str(laneId)
    self.laneNumber = laneNumber  #The number of that lane; 0 is closest to the median.
    self.parentSeg = parentSeg
    self.polyline = polyline #An array of points


class LaneEdge:
  '''A Lane edge. Drivers will drive *between* two LaneEdges.'''

  def __init__(self, laneEdgeNumber, parentSeg, polyline):
    assert_non_null("Null args", laneEdgeNumber, parentSeg, polyline)
    self.laneEdgeNumber = laneEdgeNumber #The number of that edge line; 0 represents the median.
    self.parentSeg = parentSeg
    self.polyline = polyline #An array of points


