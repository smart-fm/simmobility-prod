from geo.point import Point

#TODO: This is currently the same as temp.py

#Our container class
class RoadNetwork:
  def __init__(self):
    self.nodes = {}    #origId => Node
    self.links = {}    #origId => Link
    self.lanes = {}    #origId => Lane
    self.turnings = [] #LaneConnector


#Simple classes. IDs are always strings
class Node:
  def __init__(self, nodeId, xPos, yPos):
    if not (nodeId and xPos and yPos):
      raise Exception('Null parameters in Node constructor')
    self.nodeId = str(nodeId)
    self.guid = None
    self.pos = Point(float(xPos), float(yPos))
    self.is_uni = None

  def isUni(self):
    if self.is_uni is None:
      raise Exception('isUni not defined for Node')
    return self.is_uni

class Link:
  def __init__(self, linkId, fromNode, toNode):
    if not (linkId and fromNode and toNode):
      raise Exception('Null parameters in Link constructor')
    self.linkId = str(linkId)
    self.guid = None
    self.fromNode = str(fromNode)
    self.toNode = str(toNode)
    self.segments = [] #List of segment IDs

class Edge:
  def __init__(self, edgeId, fromNode, toNode):
    if not (edgeId and fromNode and toNode):
      raise Exception('Null parameters in Edge constructor')
    self.edgeId = str(edgeId)
    self.guid = None
    self.fromNode = str(fromNode)
    self.toNode = str(toNode)
    self.lanes = []
    self.lane_edges = []

#Note that "from/toLaneId" are zero-numbered, not actual lane IDs
class LaneConnector:
  def __init__(self, fromSegment, toSegment, fromLaneId, toLaneId, laneFromOrigId, laneToOrigId):
    self.fromSegment = fromSegment  #These are references
    self.toSegment = toSegment      #These are references
    self.fromLaneId = fromLaneId
    self.toLaneId = toLaneId
    self.laneFromOrigId = laneFromOrigId
    self.laneToOrigId = laneToOrigId

class Lane:
  def __init__(self, laneId, shape):
    if not (laneId and shape):
      raise Exception('Null parameters in Lane constructor')
    self.laneId = str(laneId)
    self.guid = None
    self.shape = Shape(shape)

class LaneEdge:
  def __init__(self, points):
    self.points = points  #Just an array of Points


#NOTE on Shapes, from the SUMO user's guide
#The start and end node are omitted from the shape definition; an example: 
#    <edge id="e1" from="0" to="1" shape="0,0 0,100"/> 
#    describes an edge that after starting at node 0, first visits position 0,0 
#    than goes one hundred meters to the right before finally reaching the position of node 1
class Shape:
  def __init__(self, pts):
    pts = pts.split(' ')

    self.points = []
    for pair in pts:
      pair = pair.split(',')
      self.points.append(Point(float(pair[0]), float(pair[1])))

