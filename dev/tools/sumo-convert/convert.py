#!/usr/bin/python

import sys
import math
from lxml import objectify
from lxml import etree

#This program converts a SUMO traffic network to Sim Mobility format (this includes 
# flipping the network for driving on the left).
#Note: Run SUMO like so:
#   ~/sumo/bin/netgenerate --rand -o sumo.net.xml --rand.iterations=200 --random -L 2

#Note that this program runs about 10x faster on Python3 for some reason

#Simple classes. IDs are always strings
class Node:
  def __init__(self, nodeId, xPos, yPos):
    if not (nodeId and xPos and yPos):
      raise Exception('Null parameters in Node constructor')
    self.nodeId = str(nodeId)
    self.guid = None
    self.pos = Point(float(xPos), float(yPos))

class Edge:
  def __init__(self, edgeId, fromNode, toNode):
    if not (edgeId and fromNode and toNode):
      raise Exception('Null parameters in Edge constructor')
    self.edgeId = str(edgeId)
    self.guid = None
    self.guidLink = None  #Edges and Links are 1-to-1 for now.
    self.fromNode = str(fromNode)
    self.toNode = str(toNode)
    self.lanes = []
    self.lane_edges = []

#Note that "from/toLaneId" are zero-numbered, not actual lane IDs
class LaneConnector:
  def __init__(self, fromSegId, toSegId, fromLaneId, toLaneId, laneFromOrigId, laneToOrigId):
    self.fromSegId = fromSegId
    self.toSegId = toSegId
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

#A basic vector (in the geometrical sense)
class DynVect:
  def __init__(self, start, end):
    self.pos = Point(float(start.x), float(start.y))
    self.mag = Point(float(end.x-start.x), float(end.y-start.y))

  def getPos(self):
    return self.pos

  def translate(self):
    self.pos.x += self.mag.x
    self.pos.y += self.mag.y
    return self

  def scaleVectTo(self, amount):
    self.makeUnit()
    self.scaleVect(amount)
    return self

  def makeUnit(self):
    self.scaleVect(1.0/self.getMagnitude())

  def getMagnitude(self):
    return math.sqrt(self.mag.x*self.mag.x + self.mag.y*self.mag.y)

  #NOTE: Be careful calling this: you almost always want "scaleVectTo()"
  def scaleVect(self, amount):
    self.mag.x *= amount
    self.mag.y *= amount

  def flipNormal(self, clockwise):
    sign =  1 if clockwise else -1
    newX = self.mag.y*sign
    newY = -self.mag.x*sign
    self.mag.x = newX
    self.mag.y = newY
    return self

  def rotateRight(self): #Flip this vector 90 degrees clockwise around the origin.
    return self.flipNormal(True)

  def rotateLeft(self):  #Flip this vector 90 degrees counter-clockwise around the origin.
    return self.flipNormal(False)


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


class Point:
  def __init__(self, x, y):
    self.x = x
    self.y = y

  def __repr__(self):
    return "Point(%d,%d)" % (self.x, self.y)

  def __str__(self):
    return "(%d,%d)" % (self.x, self.y)



def parse_edge_sumo(e, edges, lanes):
    #Sanity check
    if e.get('function')=='internal':
      raise Exception('Node with from/to should not be internal')

    #Add a new edge
    res = Edge(e.get('id'), e.get('from'), e.get('to'))
    edges[res.edgeId] = res

    #Add child Lanes
    laneTags = e.xpath("lane")
    for l in laneTags:
      newLane = Lane(l.get('id'), l.get('shape'))
      res.lanes.append(newLane)
      lanes[newLane.laneId] = newLane


def project_coords(wgsRev, utmZone, lat, lon):
  from LatLongUTMconversion import LLtoUTM

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
    raise Exception('Resultant UTM zone (%s) does not match expected zone (%s).' % (utmZone, resZone))

  #All is good; return a Point
  return Point(x,y)


def parse_junctions_sumo(j, nodes):
    #Add a new Node
    res = Node(j.get('id'), j.get('x'), j.get('y'))
    nodes[res.nodeId] = res

def parse_nodes_osm(n, nodes):
    #Nodes are slightly complicated by the fact that they use lat/long.
    projected = project_coords('WGS 84', 'UTM 48N', n.get('lat'), n.get('lon'))  #Returns a point

    #Add a new Node
    res = Node(n.get('id'), projected.x, projected.y)
    nodes[res.nodeId] = res


def remove_unused_nodes(nodes, edges):
  #Just rebuild the hash using the edges as a guide
  res_nodes = {}
  for e in edges.values():
    res_nodes[e.fromNode] = nodes[e.fromNode]
    res_nodes[e.toNode] = nodes[e.toNode]

  #Now flip it
  nodes.clear()
  nodes.update(res_nodes)


class InOut:
  def __init__(self):
    self.incoming = []
    self.outgoing = []

def make_lane_connectors(nodes, edges, turnings):
  #First, make a list of all "incoming" and "outgoing" edges at a given node
  lookup = {}  #nodeId => InOut
  for e in edges.values():
    #Give it an entry
    if not (e.fromNode in lookup):
      lookup[e.fromNode] = InOut()
    if not (e.toNode in lookup):
      lookup[e.toNode] = InOut()

    #Append
    lookup[e.toNode].incoming.append(e)
    lookup[e.fromNode].outgoing.append(e)

  #Now make a set of lane connectors from all "incoming" to all "outgoing" (except U-turns) at a Node
  for n in nodes.values():
    for fromEdge in lookup[n.nodeId].incoming:
      for toEdge in lookup[n.nodeId].outgoing:
        if (fromEdge.fromNode==toEdge.toNode and fromEdge.toNode==toEdge.fromNode):
          continue

        #The looping gets even deeper!
        for fromLaneID in range(len(fromEdge.lanes)):
          for toLaneID in range(len(toEdge.lanes)):
            turnings.append(LaneConnector(fromEdge.edgeId, toEdge.edgeId, fromLaneID, toLaneID, fromEdge.lanes[fromLaneID].laneId, toEdge.lanes[toLaneID].laneId))


def check_and_flip_and_scale(nodes, edges, lanes):
  #Save the maximum X co-ordinate, minimum Y
  maxX = None

  #Currently SimMobility can't take negative y coords
  minX = 0
  minY = 0

  #Iteraet through Edges; check node IDs
  for e in edges.values():
    if not ((e.fromNode in nodes) and (e.toNode in nodes)):
      raise Exception('Edge references unknown Node ID')

  #Iterate through Nodes, Lanes (Shapes) and check all points here too.
  for n in nodes.values():
    maxX = max((maxX if maxX else n.pos.x),n.pos.x)
    minX = min(minX, n.pos.x)
    minY = min(minY, n.pos.y)
  for l in lanes.values():
    for p in l.shape.points:
      maxX = max((maxX if maxX else p.x),p.x)
      minX = min(minX, p.x)
      minY = min(minY, p.y)

  #Add some buffer space to the y-value (for lane edge lines)
  minX -= 10
  minY -= 10

  #Now invert all x co-ordinates, and scale by 100 (to cm)
  for n in nodes.values():
    n.pos.x = (-minX + maxX - n.pos.x) * 100
    n.pos.y = (-minY + n.pos.y) * 100
  for l in lanes.values():
    for p in l.shape.points:
      p.x = (-minX + maxX - p.x) * 100
      p.y = (-minY + p.y) * 100


def mostly_parallel(first, second):
  #Both/one vertical?
  fDx= first[-1].x-first[0].x
  sDx = second[-1].x-second[0].x
  if (fDx==0 or sDx==0):
    return fDx==sDx

  #Calculate slope
  mFirst = float(first[-1].y-first[0].y) / float(fDx)
  mSecond = (second[-1].y-second[0].y) / float(sDx)
  return abs(mFirst-mSecond) < 3.0  #You can increase this as your lines skew, but don't go too high.



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



def make_lane_edges(nodes, edges, lanes):
  for e in edges.values():
    #All lanes are relative to our Segment line
    segLine = [nodes[e.fromNode].pos, nodes[e.toNode].pos]

    #We need the lane widths. To do this geometrically, first take lane line one (-1) and compare the slopes:
    zeroLine = [e.lanes[-1].shape.points[0],e.lanes[-1].shape.points[-1]]
    if not mostly_parallel(segLine, zeroLine):
      raise Exception("Can't convert; lines are not parallel")

    #Now that we know the lines are parallel, get the distance between them. This should be half the lane width
    halfW = get_line_dist(segLine, zeroLine)

    #Add lane line 1 (actually -1, since sumo lists are in reverse order) shifted RIGHT to give us line zero
    zeroStart = DynVect(zeroLine[0], zeroLine[1])
    zeroStart.rotateRight().scaleVectTo(halfW).translate()
    zeroEnd = DynVect(zeroLine[1], zeroLine[0])
    zeroEnd.rotateLeft().scaleVectTo(halfW).translate()
    e.lane_edges.append(LaneEdge([zeroStart.getPos(), zeroEnd.getPos()]))

    #Now add each remaining lane (including 1, again) shifted LEFT to give us their expected location.
    for i in reversed(range(len(e.lanes))):
      currLine = [e.lanes[i].shape.points[0],e.lanes[i].shape.points[-1]]
      currStart = DynVect(currLine[0], currLine[1])
      currStart.rotateLeft().scaleVectTo(halfW).translate()
      currEnd = DynVect(currLine[1], currLine[0])
      currEnd.rotateRight().scaleVectTo(halfW).translate()
      e.lane_edges.append(LaneEdge([currStart.getPos(), currEnd.getPos()]))



def print_old_format(nodes, edges, lanes, turnings):
  #Open, start writing
  f = open('out.txt', 'w')
  f.write('("simulation", 0, 0, {"frame-time-ms":"100",})\n')

  #Nodes
  for n in nodes.values():
    f.write('("multi-node", 0, %d, {"xPos":"%d","yPos":"%d"})\n' % (n.guid, n.pos.x, n.pos.y))

  #Links (every Edge represents a Link in this case)
  for e in edges.values():
    fromId = nodes[e.fromNode].guid
    toId = nodes[e.toNode].guid
    f.write('("link", 0, %d, {"road-name":"","start-node":"%d","end-node":"%d","fwd-path":"[%d]",})\n' % (e.guidLink, fromId, toId, e.guid))
    f.write('("road-segment", 0, %d, {"parent-link":"%d","max-speed":"65","width":"%d","lanes":"%d","from-node":"%d","to-node":"%d"})\n' % (e.guid, e.guidLink, (250*len(e.lanes)), len(e.lanes), fromId, toId))

    #Lanes are somewhat more messy
    #Note that "lane_id" here just refers to lane line 0, since lanes are grouped by edges in this output format. (Confusingly)
    f.write('("lane", 0, %d, {"parent-segment":"%d",' % (e.lanes[0].guid, e.guid))

    #Each lane component
    i = 0
    for l in e.lane_edges:
      f.write('"lane-%d":"[' % (i))

      #Lane lines are mildly more complicated, since we need lane *edge* lines.
      for p in l.points:
        f.write('(%d,%d),' % (p.x, p.y))
      f.write(']",')
      i+=1

    #And finally
    f.write('})\n')

  #Write all Lane Connectors
  currLC_id = 1
  for lc in turnings:
    f.write('("lane-connector", 0, %d, {"from-segment":"%d","from-lane":"%d","to-segment":"%d","to-lane":"%d",})\n' % (currLC_id, edges[lc.fromSegId].guid, lc.fromLaneId, edges[lc.toSegId].guid, lc.toLaneId))
    currLC_id += 1

  #Done
  f.close()


def write_xml_multinodes(f, nodes, edges, lanes, turnings):
  #Build a lookup of "edges at nodes"
  lookup = {} #nodeID => [edgeIDs]
  for e in edges.values():
    if not (e.fromNode in lookup):
      lookup[e.fromNode] = []
    if not (e.toNode in lookup):
      lookup[e.toNode] = []
    lookup[e.fromNode].append(e)
    lookup[e.toNode].append(e)

  #Now write it.
  for n in nodes.values():
    f.write('          <Intersection>\n')
    f.write('            <nodeID>%d</nodeID>\n' % n.guid)
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    f.write('            <roadSegmentsAt>\n')
    if (n.nodeId in lookup):
      for lk in lookup[n.nodeId]:
        f.write('              <segmentID>%d</segmentID>\n' % edges[lk.edgeId].guid)
    f.write('            </roadSegmentsAt>\n')

    #Wee need another lookup; this one for lane connectors
    look_lc = {} #fromSegID => [LaneConnector]
    if (n.nodeId in lookup):
      for lc in turnings:
        if (edges[lc.fromSegId] in lookup[n.nodeId]) and (edges[lc.toSegId] in lookup[n.nodeId]):
          if not (lc.fromSegId in look_lc):
            look_lc[lc.fromSegId] = []
          look_lc[lc.fromSegId].append(lc)

    #Write connectors
    f.write('            <Connectors>\n')
    for rsId in look_lc.keys():
      f.write('              <MultiConnectors>\n')
      f.write('                <RoadSegment>%d</RoadSegment>\n' % edges[rsId].guid)
      f.write('                <Connectors>\n')
      for lc in look_lc[rsId]:
        f.write('                  <Connector>\n')
        f.write('                    <laneFrom>%d</laneFrom>\n' % lanes[lc.laneFromOrigId].guid)
        f.write('                    <laneTo>%d</laneTo>\n' % lanes[lc.laneToOrigId].guid)
        f.write('                  </Connector>\n')
      f.write('                </Connectors>\n')
      f.write('              </MultiConnectors>\n')
    f.write('            </Connectors>\n')

    f.write('          </Intersection>\n')


def write_xml_nodes(f, nodes, edges, lanes, turnings):
  #Write Nodes
  f.write('      <Nodes>\n')
  f.write('        <UniNodes/>\n')
  f.write('        <Intersections>\n')
  write_xml_multinodes(f, nodes, edges, lanes, turnings)
  f.write('        </Intersections>\n')
  f.write('      </Nodes>\n')


#Distance between 2 nodes/points
def dist(m, n):
  #Convert to Point
  if isinstance(m, Node):
    m = m.pos
  if isinstance(n, Node):
    n = n.pos

  #Calc distance
  dx = n.x - m.x;
  dy = n.y - m.y;
  return math.sqrt(dx**2 + dy**2);



def write_xml_lane_edge_polylines(f, lane_edges):
  #Relatively simple layout
  curr_id = 0
  for le in lane_edges:
    f.write('                <laneEdgePolyline_cached>\n')
    f.write('                  <laneNumber>%d</laneNumber>\n' % curr_id)
    curr_id += 1
    f.write('                  <polyline>\n')
    pt_id = 0
    for p in le.points:
      f.write('                    <PolyPoint>\n')
      f.write('                      <pointID>%d</pointID>\n' % pt_id)
      pt_id += 1
      f.write('                      <location>\n')
      f.write('                        <xPos>%d</xPos>\n' % p.x)
      f.write('                        <yPos>%d</yPos>\n' % p.y)
      f.write('                      </location>\n')
      f.write('                    </PolyPoint>\n')
    f.write('                  </polyline>\n')
    f.write('                </laneEdgePolyline_cached>\n')


def write_xml_lanes(f, lanes, est_width):
  #Lanes just have a lot of properties; we fake nearly all of them.
  for l in lanes:
    f.write('                <Lane>\n')
    f.write('                  <laneID>%d</laneID>\n' % l.guid)
    f.write('                  <width>%d</width>\n' % est_width)
    f.write('                  <can_go_straight>true</can_go_straight>\n')
    f.write('                  <can_turn_left>true</can_turn_left>\n')
    f.write('                  <can_turn_right>true</can_turn_right>\n')
    f.write('                  <can_turn_on_red_signal>false</can_turn_on_red_signal>\n')
    f.write('                  <can_change_lane_left>true</can_change_lane_left>\n')
    f.write('                  <can_change_lane_right>true</can_change_lane_right>\n')
    f.write('                  <is_road_shoulder>false</is_road_shoulder>\n')
    f.write('                  <is_bicycle_lane>false</is_bicycle_lane>\n')
    f.write('                  <is_pedestrian_lane>false</is_pedestrian_lane>\n')
    f.write('                  <is_vehicle_lane>true</is_vehicle_lane>\n')
    f.write('                  <is_standard_bus_lane>false</is_standard_bus_lane>\n')
    f.write('                  <is_whole_day_bus_lane>false</is_whole_day_bus_lane>\n')
    f.write('                  <is_high_occupancy_vehicle_lane>false</is_high_occupancy_vehicle_lane>\n')
    f.write('                  <can_freely_park_here>false</can_freely_park_here>\n')
    f.write('                  <can_stop_here>false</can_stop_here>\n')
    f.write('                  <is_u_turn_allowed>false</is_u_turn_allowed>\n')
    f.write('                  <PolyLine>\n')
    pt_id = 0
    for p in l.shape.points:
      f.write('                    <PolyPoint>\n')
      f.write('                      <pointID>%d</pointID>\n' % pt_id)
      pt_id += 1
      f.write('                      <location>\n')
      f.write('                        <xPos>%d</xPos>\n' % p.x)
      f.write('                        <yPos>%d</yPos>\n' % p.y)
      f.write('                      </location>\n')
      f.write('                    </PolyPoint>\n')
    f.write('                  </PolyLine>\n')
    f.write('                </Lane>\n')


def write_xml_segment(f, e, nodes, edges, turnings):
  #Each Link has only 1 segment
  seg_width = dist(e.lane_edges[0].points[0],e.lane_edges[-1].points[0])
  f.write('            <Segment>\n')
  f.write('              <segmentID>%d</segmentID>\n' % e.guid)
  f.write('              <startingNode>%d</startingNode>\n' % nodes[e.fromNode].guid)
  f.write('              <endingNode>%d</endingNode>\n' % nodes[e.toNode].guid)
  f.write('              <maxSpeed>60</maxSpeed>\n')
  f.write('              <Length>%d</Length>\n' % dist(nodes[e.fromNode],nodes[e.toNode]))
  f.write('              <Width>%d</Width>\n' % seg_width)
  f.write('              <polyline>\n')
  f.write('                <PolyPoint>\n')
  f.write('                  <pointID>0</pointID>\n')
  f.write('                  <location>\n')
  f.write('                    <xPos>%d</xPos>\n' % nodes[e.fromNode].pos.x)
  f.write('                    <yPos>%d</yPos>\n' % nodes[e.fromNode].pos.y)
  f.write('                  </location>\n')
  f.write('                </PolyPoint>\n')
  f.write('                <PolyPoint>\n')
  f.write('                  <pointID>1</pointID>\n')
  f.write('                  <location>\n')
  f.write('                    <xPos>%d</xPos>\n' % nodes[e.toNode].pos.x)
  f.write('                    <yPos>%d</yPos>\n' % nodes[e.toNode].pos.y)
  f.write('                  </location>\n')
  f.write('                </PolyPoint>\n')
  f.write('              </polyline>\n')
  f.write('              <laneEdgePolylines_cached>\n')
  write_xml_lane_edge_polylines(f, e.lane_edges)
  f.write('              </laneEdgePolylines_cached>\n')
  f.write('              <Lanes>\n')
  write_xml_lanes(f, e.lanes, seg_width/float(len(e.lanes)))
  f.write('              </Lanes>\n')
  f.write('              <Obstacles>\n')  #No obstacles for now, but we might generate pedestrian crossings later.
  f.write('              </Obstacles>\n')
  f.write('            </Segment>\n')

def write_xml_links(f, nodes, edges, turnings):
  #Write Links
  f.write('      <Links>\n')
  for e in edges.values():
    f.write('        <Link>\n')
    f.write('          <linkID>%d</linkID>\n' % e.guidLink)
    f.write('          <roadName/>\n')
    f.write('          <StartingNode>%d</StartingNode>\n' % nodes[e.fromNode].guid)
    f.write('          <EndingNode>%d</EndingNode>\n' % nodes[e.toNode].guid)
    f.write('          <Segments>\n')
    write_xml_segment(f, e, nodes, edges, turnings)
    f.write('          </Segments>\n')
    f.write('        </Link>\n')
  f.write('      </Links>\n')


def print_xml_format(nodes, edges, lanes, turnings):
  #Open, start writing
  f = open('simmob.network.xml', 'w')
  f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
  f.write('<geo:SimMobility\n')
  f.write('    xmlns:geo="http://www.smart.mit.edu/geo"\n')
  f.write('    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \n')
  f.write('    xsi:schemaLocation="http://www.smart.mit.edu/geo   ../../Basic/shared/geospatial/xmlLoader/geo10.xsd">\n\n')

  f.write('    <GeoSpatial>\n')
  f.write('    <RoadNetwork>\n')
  write_xml_nodes(f, nodes, edges, lanes, turnings)
  write_xml_links(f, nodes, edges, turnings)
  f.write('    </RoadNetwork>\n')
  f.write('    </GeoSpatial>\n')
  f.write('</geo:SimMobility>\n')

  #Done
  f.close()


#Sim Mobility doesn't strictly require globally unique IDs, but they make debugging easier
#  (and may be related to a glitch in the StreetDirectory).
def assign_unique_ids(nodes, edges, lanes, currId):
  for n in nodes.values():
    n.guid = currId
    currId += 1
  for e in edges.values():
    e.guid = currId
    e.guidLink = currId+1
    currId += 2
  for l in lanes.values():
    l.guid = currId
    currId += 1




def parse_all_sumo(rootNode, nodes, edges, lanes):
  #For each edge; ignore "internal"
  edgeTags = rootNode.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    parse_edge_sumo(e, edges, lanes)

  #For each junction
  junctTags = rootNode.xpath('/net/junction')
  for j in junctTags:
    parse_junctions_sumo(j, nodes)



def parse_all_osm(rootNode, nodes, edges, lanes):
  #All edges
#  edgeTags = rootNode.xpath("/net/edge[(@from)and(@to)]")
#  for e in edgeTags:
#    parse_edge_sumo(e, edges, lanes)

  #All nodes
  nodeTags = rootNode.xpath('/osm/node')
  for n in nodeTags:
    parse_nodes_osm(n, nodes)



def run_main(inFileName):
  #Result containers
  edges = {}
  lanes = {}
  nodes = {}
  turnings = []

  #Load, parse
  inFile = open(inFileName)
  doc = objectify.parse(inFile)

  #What kind of file is this?
  format = 'U'  #U[nknown], [S]umo, [O]penStreetMap
  if len(doc.xpath("/net"))>0:
    format = 'S'
  elif len(doc.xpath("/osm"))>0:
    format = 'O'

  #Parse edges, lanes, nodes
  if format=='S':
    parse_all_sumo(doc, nodes, edges, lanes)
  elif format=='O':
    parse_all_osm(doc, nodes, edges, lanes)
  else:
    raise Exception('Unknown road network format: ' + inFileName)


  #Remove junction nodes which aren't referenced by anything else.
  nodesPruned = len(nodes)
  remove_unused_nodes(nodes, edges)
  nodesPruned -= len(nodes)
  print("Pruned unreferenced nodes: %d" % nodesPruned)

  #Give these unique output IDs
  assign_unique_ids(nodes, edges, lanes, 1000)

  #Check network properties; flip X coordinates
  check_and_flip_and_scale(nodes, edges, lanes)

  #Create N+1 lane edges from N lanes
  make_lane_edges(nodes, edges, lanes)

  #Create lane connectors from/to every lane *except* going backwards. This is an 
  # oversimplification, but it applies to all generated SUMO networks.
  make_lane_connectors(nodes, edges, turnings)

  #Before printing the XML network, we should print an "out.txt" file for 
  #  easier visual verification with our old GUI.
  print_old_format(nodes, edges, lanes, turnings)

  #Now print the network in XML format, for use with the actual software.
  print_xml_format(nodes, edges, lanes, turnings)


if __name__ == "__main__":
  if len(sys.argv) < 2:
    print ('Usage:\n' , sys.argv[0] , '<in_file.net.xml>')
    sys.exit(0)

  run_main(sys.argv[1])
  print("Done")




