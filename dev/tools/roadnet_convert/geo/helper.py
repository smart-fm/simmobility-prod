from geo.LatLongUTMconversion import LLtoUTM
from geo.LatLongUTMconversion import UTMtoLL
from geo.position import Point
from geo.position import Location
import geo.formats.simmob
import math

#Assert that all values are non-null
def assert_non_null(*args, msg="Unexpected null value"):
  for arg in args:
    if arg is None:
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
  dx = float(n.x) - m.x;
  dy = float(n.y) - m.y;
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


#Project coordinates using WGS 84.
# Pass a non-nil "zone" to ensure that this point was transformed using that Zone (this is also returned)
# It should be obvious that transforming points that reside in different zones is a bad idea.
def project_wgs84(lat, lng, zone):
  #Convenience constant
  WGS_Id = 23 #WGS 84

  #Make sure we have both params, convert to float. 
  assert_non_null(lat, lng, msg="Lat/Lng required in project_coords")

  #Now, perform the projection, and make sure it maches our expected zone.
  (resZone, x, y) = LLtoUTM(WGS_Id, float(lat), float(lng))
  if zone and (resZone != zone):
    raise Exception('Resultant UTM zone (%s) does not match expected zone (%s).' % (resZone,zone))

  #All is good; return a Point and the zone is resides in.
  return (Point(x,y), resZone)


#Get the minimum Point (x,y) in this network.
def get_global_min(rn :geo.formats.simmob.RoadNetwork) ->Point:
  globMin = None
  
  #Find every "point" and test it.
  for nd in rn.nodes.values():
    if not globMin:
      globMin = Point(nd.pos.x, nd.pos.y)
    globMin.x = min(globMin.x, nd.pos.x)
    globMin.y = min(globMin.y, nd.pos.y)
  for lnk in rn.links.values():
    for sg in lnk.segments:
      for ln in sg.lanes:
        for pt in ln.polyline:
          globMin.x = min(globMin.x, pt.x)
          globMin.y = min(globMin.y, pt.y)
      for le in sg.lane_edges:
        for pt in le.polyline:
          globMin.x = min(globMin.x, pt.x)
          globMin.y = min(globMin.y, pt.y)
  return globMin


#Translate every point by (x,y), ignoring x or y if either is >=0
def translate_network(rn :geo.formats.simmob.RoadNetwork, transPt :Point):
  #Mutate our input value.
  if transPt.x>0:
    transPt.x = 0
  if transPt.y>0:
    transPt.y = 0
  if (transPt.x==0) and (transPt.y==0):
    return

  #Find every "point" and translate it.
  for nd in rn.nodes.values():
    nd.pos.x -= transPt.x
    nd.pos.y -= transPt.y
  for lnk in rn.links.values():
    for sg in lnk.segments:
      for ln in sg.lanes:
        for pt in ln.polyline:
          pt.x -= transPt.x
          pt.y -= transPt.y
      for le in sg.lane_edges:
        for pt in le.polyline:
          pt.x -= transPt.x
          pt.y -= transPt.y



def remove_unused_nodes(nodes, links):
  '''This function counts the number of RoadSegments which reference a Node. If it's zero, that 
     Node is pruned.
  '''

  #Note: I *think* that using "node" (not ID) as a key will work here.
  nodesAt = {} #node => [segment, segment,]
  for lnk in links.values():
    for s in lnk.segments:
      #Add it if it doesn't exist.
      if not s.fromNode in nodesAt:
        nodesAt[s.fromNode] = []
      if not s.toNode in nodesAt:
        nodesAt[s.toNode] = []

    #Increment
    nodesAt[s.fromNode].append(s)
    nodesAt[s.toNode].append(s)

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


def get_lane_widths(lane_edges):
  '''Retrieve the width of each lane based on lane edges.'''
  res = []
  prevLE = None
  for le in lane_edges:
    if prevLE:
      #Just the starting points should be sufficient.
      res.append(geo.helper.dist(prevLE.polyline[0], le.polyline[0]))
    prevLE = le
  return res


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
  halfW = 3.5*100 / 2 #Temp; we're getting very narrow lanes otherwise.

  #Add lane 0 shifted RIGHT to give us lane edge zero
  zeroStart = DynVect(zeroLine[0], zeroLine[1])
  zeroStart.rotateRight().scaleVectTo(halfW).translate()
  zeroEnd = DynVect(zeroLine[1], zeroLine[0])
  zeroEnd.rotateLeft().scaleVectTo(halfW).translate()
  seg.lane_edges.append(geo.formats.simmob.LaneEdge(0, seg, [zeroStart.getPos(), zeroEnd.getPos()]))

  #Now just shift each lane left to make our lane edges.
  lnEdNum = 1
  for i in range(len(seg.lanes)):
    currLine = [seg.lanes[i].polyline[0],seg.lanes[i].polyline[-1]]
    currStart = DynVect(currLine[0], currLine[1])
    currStart.rotateLeft().scaleVectTo(halfW).translate()
    currEnd = DynVect(currLine[1], currLine[0])
    currEnd.rotateRight().scaleVectTo(halfW).translate()
    seg.lane_edges.append(geo.formats.simmob.LaneEdge(lnEdNum, seg, [currStart.getPos(), currEnd.getPos()]))
    lnEdNum += 1


#Generate lane connectors from/to all lanes the might possibly meet (e.g., at the same Node).
def make_lane_connectors(rn):
  #First, make a list of all "incoming" and "outgoing" edges at a given node
  lookup = {}  #nodeId => InOut
  for lk in rn.links.values():
    for seg in lk.segments:
      #Give it an entry
      if not (seg.fromNode.nodeId in lookup):
        lookup[seg.fromNode.nodeId] = InOut()
      if not (seg.toNode.nodeId in lookup):
        lookup[seg.toNode.nodeId] = InOut()

      #Append
      lookup[seg.toNode.nodeId].incoming.append(seg)
      lookup[seg.fromNode.nodeId].outgoing.append(seg)

  #Now make a set of lane connectors from all "incoming" to all "outgoing" (except U-turns) at a Node
  for n in rn.nodes.values():
    if not n.nodeId in lookup:
      continue
    for fromEdge in lookup[n.nodeId].incoming:
      for toEdge in lookup[n.nodeId].outgoing:
        #No U-turns
        if (fromEdge.fromNode==toEdge.toNode and fromEdge.toNode==toEdge.fromNode):
          continue

        if not toEdge.segId in fromEdge.lane_connectors:
          fromEdge.lane_connectors[toEdge.segId] = []

        #The looping gets even deeper!
        for fromLane in fromEdge.lanes:
          for toLane in toEdge.lanes:
            #Sanity check.
            if fromEdge.toNode.nodeId != toEdge.fromNode.nodeId:
              raise Exception("Error: Lane Connector malformed (1)")
            if fromEdge.toNode.nodeId != n.nodeId:
              raise Exception("Error: Lane Connector malformed (2)")

            fromEdge.lane_connectors[toEdge.segId].append(geo.formats.simmob.LaneConnector(fromEdge, toEdge, fromLane, toLane))


#Helper for remembering incoming and outgoing Segments at a given Node.
class InOut:
  def __init__(self):
    self.incoming = []
    self.outgoing = []


#Helper class for generating unique IDs
class IdGenerator:
  def __init__(self, initVal): 
    self.currVal = initVal-1 #Because of how we increment

  #Get the next assignable ID (and increment).
  def next(self) -> str:
    self.currVal += 1
    return str(self.currVal)


#Helper class for UTM scaling
class ScaleHelper:
  def __init__(self, outBounds):
    self.outBounds = outBounds #minLocation, maxLocation of output
    self.inBounds = [None, None] #minPoint, maxPoint of input
#    self.center = project_coords('WGS 84', 'UTM 48N', 1.305, 103.851)  #Singapore, roughly

  #Add a point to the bounds
  def add_point(self, pt):
    if not self.inBounds[0]:
      self.inBounds[0] = Point(pt.x, pt.y)
    if not self.inBounds[1]:
      self.inBounds[1] = Point(pt.x, pt.y)

    #Update
    self.inBounds[0].x = min(self.inBounds[0].x, pt.x)
    self.inBounds[0].y = min(self.inBounds[0].y, pt.y)
    self.inBounds[1].x = max(self.inBounds[1].x, pt.x)
    self.inBounds[1].y = max(self.inBounds[1].y, pt.y)

  #Convert a point to lat/long
  def convert(self, pt :Point) -> Location:
    #Get the normalized components
    norm = Point( \
      (pt.x-self.inBounds[0].x)/(self.inBounds[1].x-self.inBounds[0].x), \
      (pt.y-self.inBounds[0].y)/(self.inBounds[1].y-self.inBounds[0].y)  \
    )

    #Convert to lat/lng. Note that Latitude scales up (but this should be handled by the same formula).
    #NOTE: Linear scaling will introduce artifacts, but for now a reverse-transformation
    #      would be even more problematic.
    newLoc = Location( \
      (norm.y*(self.outBounds[1].lat-self.outBounds[0].lat)+self.outBounds[0].lat), \
      (norm.x*(self.outBounds[1].lng-self.outBounds[0].lng)+self.outBounds[0].lng)  \
    )

    #Done
    return newLoc

  #Get the minimum/maximum points
#  def min_pt(self):
#    return Point(self.bounds[0], self.bounds[1])
#  def max_pt(self):
#    return Point(self.bounds[2], self.bounds[3])


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
