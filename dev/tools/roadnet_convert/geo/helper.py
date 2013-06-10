from geo.LatLongUTMconversion import LLtoUTM
from geo.LatLongUTMconversion import UTMtoLL
from geo.position import Point
from geo.position import DynVect
from geo.formats import simmob
import math

#Assert that all values are non-null
def assert_non_null(msg, *args):
  for arg in args:
    if not arg:
      raise Exception(msg)

#Convert a dictionary's key/value pairs to lowercase
def dict_to_lower(orig):
  res = {}
  for k,v in orig:
    k = str(k).lower()
    v = str(v).lower()
    if k in orig:
      raise Exception("Duplicate key (by case) in map: "+k)
    res[k] = v
  return res


#Distance between 2 nodes/points
def dist(m, n):
  #Convert the various 'Node' types to Point
  if hasattr(m, 'pos'):
    m = m.pos
  if hasattr(n, 'pos'):
    n = n.pos

  #Calc distance
  dx = n.x - m.x;
  dy = n.y - m.y;
  return math.sqrt(dx**2 + dy**2)


#Assumes first/second are parallel lines
def get_line_dist(first, second):
  #Slope is vertical?
  dx= first[-1].x-first[0].x
  if (dx==0):
    return abs(first[0].y-second[0].y)

  #Otherwise, get y-intercept for each line.
  m1 = (first[-1].y-first[0].y) / float(dx)
  m2 = (second[-1].y-second[0].y) / float(dx)
  m = (m1+m2)/2.0  #We use the average slope just in case the lines aren't *quite* parallel
  fB = first[0].y - m * first[0].x
  sB = second[0].y - m * second[0].x

  #And then we have a magic formula:
  return abs(sB-fB) / math.sqrt(m**2 + 1)


#Project coordinates (TODO: currently very rigid)
def project_coords(wgsRev, utmZone, lat, lon):
  #Make sure we have both params, convert to float.
  if not (lat and lon):
    raise Exception('lat/lon required in project_coords')
  lat = float(lat)
  lon = float(lon)

  #Make sure they are using the latest standard
  if (wgsRev.replace(' ', '') != 'WGS84'):
    raise Exception('Deprecated WGS specification (only WGS 84 supported)')

  #Now, perform the projection. Make sure our result matches our expectations.
  (resZone, x, y) = LLtoUTM(23, lat, lon)
  if (utmZone.replace(' ', '') != "UTM"+resZone.replace(' ', '')):
    raise Exception('Resultant UTM zone (%s) does not match expected zone (%s).' % (resZone,utmZone))

  #All is good; return a Point
  return Point(x,y)


def remove_unused_nodes(nodes, segments):
  '''This function counts the number of RoadSegments which reference a Node. If it's zero, that 
     Node is pruned.
  '''

  nodesAt = {} #node => [segment, segment,]
  for s in segments.values():
    #Add it if it doesn't exist.
    if not s.fromNode in nodesAt:
      nodesAt[s.fromNode] = []
    if not s.toNode in nodesAt:
      nodesAt[s.toNode] = []

    #Increment
    nodesAt[e.fromNode].append(e)
    nodesAt[e.toNode].append(e)

  #We can cheat a little here: Nodes with no references won't even be in our result set.
  nodes.clear()
  for n in nodesAt.keys():
    nodes[n.nodeId] = n


def __mostly_parallel(first, second):
  #Cutoff point for considering lines *not* parallel
  #You can increase this as your lines skew, but don't go too high.
  #Cutoff = 3.0
  Cutoff = 50.0  #TODO: Temp, while we work on OSM data.

  #Both/one vertical?
  fDx= first[-1].x-first[0].x
  sDx = second[-1].x-second[0].x
  if (fDx==0 or sDx==0):
    return (fDx-sDx) < Cutoff

  #Calculate slope
  mFirst = float(first[-1].y-first[0].y) / float(fDx)
  mSecond = (second[-1].y-second[0].y) / float(sDx)
  return abs(mFirst-mSecond) < Cutoff


#Expand lanes halfway in each direction to make lane edges.
#NOTE: This function assumes that the Segment's line is roughly 1/2 a lane's 
#      distance from lane line 0, and is (somewhat) parallel.
#      If this is not the case we will need another way of estimating lane width.
#TODO: We might want to make the lane width a parameter, since our assumption of 
#      the Segment's placement only makes sense for SUMO (so move that into a SUMO-specific function).
def make_lane_edges_from_lane_lines(seg, global_id): 
  #All lanes are relative to our Segment line
  refLine = [seg.fromNode.pos, seg.toNode.pos]

  #We need the lane widths. To do this geometrically, first take lane line 0 and compare the slopes:
  zeroLine = [seg.lanes[0].polyline[0], seg.lanes[0].polyline[-1]]
  if not __mostly_parallel(refLine, zeroLine):
    raise Exception("Can't convert segment %s; lines are not parallel: [%s=>%s] and [%s=>%s]" % (seg.segId, refLine[0], refLine[-1], zeroLine[0], zeroLine[-1]))

  #Now that we know the lines are parallel, get the distance between them. This should be half the lane width
  halfW = get_line_dist(refLine, zeroLine)

  #Add lane 0 shifted RIGHT to give us lane edge zero
  zeroStart = DynVect(zeroLine[0], zeroLine[1])
  zeroStart.rotateRight().scaleVectTo(halfW).translate()
  zeroEnd = DynVect(zeroLine[1], zeroLine[0])
  zeroEnd.rotateLeft().scaleVectTo(halfW).translate()
  seg.lane_edges.append(simmob.LaneEdge(global_id.next(), 0, seg, [zeroStart.getPos(), zeroEnd.getPos()]))

  #Now just shift each lane left to make our lane edges.
  lnEdNum = 1
  for i in range(len(lanes)):
    currLine = [e.lanes[i].shape.points[0],e.lanes[i].shape.points[-1]]
    currStart = DynVect(currLine[0], currLine[1])
    currStart.rotateLeft().scaleVectTo(halfW).translate()
    currEnd = DynVect(currLine[1], currLine[0])
    currEnd.rotateRight().scaleVectTo(halfW).translate()
    seg.lane_edges.append(simmob.LaneEdge(global_id.next(), lnEdNum, seg, [currStart.getPos(), currEnd.getPos()]))
    lnEdNum += 1


#Generate lane connectors from/to all lanes the might possibly meet (e.g., at the same Node).
def make_lane_connectors(rn):
  #First, make a list of all "incoming" and "outgoing" edges at a given node
  lookup = {}  #nodeId => InOut
  for lk in rn.links.values():
    for e in lk.segments:
      #Give it an entry
      if not (e.fromNode in lookup):
        lookup[e.fromNode] = InOut()
      if not (e.toNode in lookup):
        lookup[e.toNode] = InOut()

    #Append
    lookup[e.toNode].incoming.append(e)
    lookup[e.fromNode].outgoing.append(e)

  #Now make a set of lane connectors from all "incoming" to all "outgoing" (except U-turns) at a Node
  for n in rn.nodes.values():
    for fromEdge in lookup[n.nodeId].incoming:
      for toEdge in lookup[n.nodeId].outgoing:
        if (fromEdge.fromNode==toEdge.toNode and fromEdge.toNode==toEdge.fromNode):
          continue

        #The looping gets even deeper!
        for fromLaneID in range(len(fromEdge.lanes)):
          for toLaneID in range(len(toEdge.lanes)):
            rn.turnings.append(simmob.LaneConnector(fromEdge, toEdge, fromLaneID, toLaneID, fromEdge.lanes[fromLaneID].laneId, toEdge.lanes[toLaneID].laneId))


#Helper class for generating unique IDs
class IdGenerator:
  def __init__(self, initVal): 
    self.currVal = initVal-1 #Because of how we increment

  #Get the next assignable ID (and increment).
  def next(self) => str:
    self.currVal += 1
    return str(self.currVal)


#Helper class for UTM scaling
class ScaleHelper:
  def __init__(self):
    self.bounds = [None, None, None, None] #minX,minY,maxX,maxY
    self.center = project_coords('WGS 84', 'UTM 48N', 1.305, 103.851)  #Singapore, roughly

  #Add a point to the bounds
  def add_point(self, pt):
    self.bounds[0] = min(x for x in [pt.x, self.bounds[0]] if x is not None)
    self.bounds[1] = min(y for y in [pt.y, self.bounds[1]] if y is not None)
    self.bounds[2] = max(x for x in [pt.x, self.bounds[2]] if x is not None)
    self.bounds[3] = max(y for y in [pt.y, self.bounds[3]] if y is not None)

  #Convert a point to lat/long
  def convert(self, pt):
    #Convert this point to an offset "relative" to the center of the bounds, then add this to our UTM center.
    offset = Point(self.bounds[0]+(self.bounds[2]-self.bounds[0])/2,self.bounds[1]+(self.bounds[3]-self.bounds[1])/2)
    newPt = Point(self.center.x+(pt.x-offset.x), self.center.y+(pt.y-offset.y))

    #Finally, convert back to lat/lng
    return UTMtoLL(23, newPt.y, newPt.x, '48N')

  #Get the minimum/maximum points
  def min_pt(self):
    return Point(self.bounds[0], self.bounds[1])
  def max_pt(self):
    return Point(self.bounds[2], self.bounds[3])


#A basic vector (in the geometrical sense)
class DynVect:
  def __init__(self, start, end):
    #Ensure that we copy.
    self.pos = Point(float(start.x), float(start.y))
    self.mag = Point(float(end.x)-start.x, float(end.y)-start.y)

  def getPos(self):
    return self.pos

  def translate(self):
    self.pos.x += self.mag.x
    self.pos.y += self.mag.y
    return self

  def scaleVectTo(self, amount):
    #Factoring in the unit vector by dividing early is more accurate.
    factor = amount/self.getMagnitude()
    self.mag.x = factor * self.mag.x
    self.mag.y = factor * self.mag.y
    return self

  def getMagnitude(self):
    return math.sqrt(self.mag.x**2.0 + self.mag.y**2.0)

  def flipNormal(self, clockwise):
    sign =  1.0 if clockwise else -1.0
    newX = self.mag.y*sign
    newY = -self.mag.x*sign
    self.mag.x = newX
    self.mag.y = newY
    return self

  def rotateRight(self): #Flip this vector 90 degrees clockwise around the origin.
    return self.flipNormal(True)

  def rotateLeft(self):  #Flip this vector 90 degrees counter-clockwise around the origin.
    return self.flipNormal(False)
