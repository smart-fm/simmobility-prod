from geo.position import Point
import geo.helper

#Our container class
class RoadNetwork:
  '''The primary container class for SUMO road networks. (See: simmob.py)
     At the moment, we only support simple (multi-node only) SUMO networks.
  '''

  def __init__(self):
    self.junctions = {}    #{jnctId => Junction}
    self.edges = {}    #{edgeId => Edge}


class Junction:
  '''A Junction fulfils a similar purpose to a simmob.Node'''

  def __init__(self, jnctId, xPos, yPos):
    geo.helper.assert_non_null("Null args", jnctId, xPos, yPos)
    self.jnctId = str(jnctId)
    self.pos = Point(float(xPos), float(yPos))


class Edge:
  def __init__(self, edgeId, fromJnct, toJnct):
    geo.helper.assert_non_null("Null args", edgeId, fromJnct, toJnct)
    self.edgeId = str(edgeId)
    self.fromJnct = fromJnct
    self.toJnct = toJnct
    self.lanes = []  #Lanes in order (closest-to-median first ---I think.)


class Lane:
  def __init__(self, laneId, shape):
    geo.helper.assert_non_null("Null args", laneId, shape)
    self.laneId = str(laneId)
    self.shape = shape


#NOTE on Shapes, from the SUMO user's guide
#The start and end node are omitted from the shape definition; an example: 
#    <edge id="e1" from="0" to="1" shape="0,0 0,100"/> 
#    describes an edge that after starting at node 0, first visits position 0,0 
#    than goes one hundred meters to the right before finally reaching the position of node 1
#The start and end Nodes are not usually part of the polyline anyway, since lane edges start a 
#    few meters out from the Intersection.
class Shape:
  def __init__(self, pts):
    geo.helper.assert_non_null("Null args", pts)
    pts = pts.split(' ')

    self.points = []
    for pair in pts:
      pair = pair.split(',')
      self.points.append(Point(float(pair[0]), float(pair[1])))

