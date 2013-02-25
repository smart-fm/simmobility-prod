#!/usr/bin/python

import sys
import math
from lxml import objectify
from lxml import etree

#This program converts a SUMO traffic network to Sim Mobility format (this includes 
# flipping the network for driving on the left).
#Note: Run SUMO like so:
#   ~/sumo/bin/netgenerate --rand -o sumo.net.xml --rand.iterations=200 --random -L 2
#Should work with Python 2 or 3.

#Simple classes. IDs are always strings
class Node:
  def __init__(self, nodeId, xPos, yPos):
    if not (nodeId and xPos and yPos):
      raise Exception('Null parameters in Node constructor')
    self.nodeId = str(nodeId)
    self.pos = Point(float(xPos), float(yPos))

class Edge:
  def __init__(self, edgeId, fromNode, toNode):
    if not (edgeId and fromNode and toNode):
      raise Exception('Null parameters in Edge constructor')
    self.edgeId = str(edgeId)
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



def parse_edge(e, edges, lanes):
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


def parse_junctions(j, nodes):
    #Add a new Node
    res = Node(j.get('id'), j.get('x'), j.get('y'))
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
  #Save the maximum X co-ordinate
  maxX = None

  #Iteraet through Edges; check node IDs
  for e in edges.values():
    if not ((e.fromNode in nodes) and (e.toNode in nodes)):
      raise Exception('Edge references unknown Node ID')

  #Iterate through Nodes, Lanes (Shapes) and check all points here too.
  for n in nodes.values():
    maxX = max((maxX if maxX else n.pos.x),n.pos.x)
  for l in lanes.values():
    for p in l.shape.points:
      maxX = max((maxX if maxX else p.x),p.x)

  #Now invert all x co-ordinates, and scale by 100 (to cm)
  for n in nodes.values():
    n.pos.x = (maxX - n.pos.x) * 100
    n.pos.y *= 100
  for l in lanes.values():
    for p in l.shape.points:
      p.x = (maxX - p.x) * 100
      p.y *= 100


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
  #We need integer-style IDs for sim mobility
  node_ids = {}
  link_ids = {}
  segment_ids = {}
  currLaneId = 1

  #Open, start writing
  f = open('out.txt', 'w')
  f.write('("simulation", 0, 0, {"frame-time-ms":"100",})\n')

  #Nodes
  for n in nodes.values():
    node_ids[n.nodeId] = len(node_ids)+1
    f.write('("multi-node", 0, %d, {"xPos":"%d","yPos":"%d"})\n' % (node_ids[n.nodeId], n.pos.x, n.pos.y))

  #Links (every Edge represents a Link in this case)
  for e in edges.values():
    link_ids[e.edgeId] = len(link_ids)+1
    segment_ids[e.edgeId] = len(segment_ids)+1
    fromId = node_ids[nodes[e.fromNode].nodeId]
    toId = node_ids[nodes[e.toNode].nodeId]
    f.write('("link", 0, %d, {"road-name":"","start-node":"%d","end-node":"%d","fwd-path":"[%d]",})\n' % (link_ids[e.edgeId], fromId, toId, segment_ids[e.edgeId]))
    f.write('("road-segment", 0, %d, {"parent-link":"%d","max-speed":"65","width":"%d","lanes":"%d","from-node":"%d","to-node":"%d"})\n' % (segment_ids[e.edgeId], link_ids[e.edgeId], (250*len(e.lanes)), len(e.lanes), fromId, toId))

    #Lanes are somewhat more messy
    f.write('("lane", 0, %d, {"parent-segment":"%d",' % (currLaneId, segment_ids[e.edgeId]))
    currLaneId+=1

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
    f.write('("lane-connector", 0, %d, {"from-segment":"%d","from-lane":"%d","to-segment":"%d","to-lane":"%d",})\n' % (currLC_id, segment_ids[lc.fromSegId], lc.fromLaneId, segment_ids[lc.toSegId], lc.toLaneId))
    currLC_id += 1

  #Done
  f.close()


def write_xml_multinodes(f, nodes, node_ids, edges, edge_ids, turnings, lane_ids):
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
    f.write('            <nodeID>%d</nodeID>\n' % node_ids[n.nodeId])
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    f.write('            <roadSegmentsAt>\n')
    if (n.nodeId in lookup):
      for lk in lookup[n.nodeId]:
        f.write('              <segmentID>%d</segmentID>\n' % edge_ids[lk.edgeId])
    f.write('            </roadSegmentsAt>\n')

    #Wee need another lookup; this one for lane connectors
    look_lc = {} #fromSegID => [LaneConnector]
    if (n.nodeId in lookup):
      for lc in turnings:
        if (lc.fromSegId in lookup[n.nodeId]) and (lc.toSegId in lookup[n.nodeId]):
          if not (lc.fromSegId in look_lc):
            look_lc[lc.fromSegId] = []
          look_lc[lc.fromSegId].append(lc)

    #Write connectors
    f.write('            <Connectors>\n')
    for rsId in look_lc.keys():
      f.write('              <MultiConnectors>\n')
      f.write('                <RoadSegment>%d</RoadSegment>\n' % edge_ids[rsId])
      f.write('                <Connectors>\n')
      for lc in look_lc[rsId]:
        f.write('                  <Connector>\n')
        f.write('                    <laneFrom>%d</laneFrom>\n' % lane_ids[lc.laneFromOrigId])
        f.write('                    <laneTo>%d</laneTo>\n' % lane_ids[lc.laneToOrigId])
        f.write('                  </Connector>\n')
      f.write('              </MultiConnectors>\n')
    f.write('            </Connectors>\n')

    f.write('          </Intersection>\n')


def write_xml_nodes(f, nodes, node_ids, edges, edge_ids, turnings, lane_ids):
  f.write('      <Nodes>\n')
  f.write('        <UniNodes/>\n')
  f.write('        <Intersections>\n')
  write_xml_multinodes(f, nodes, node_ids, edges, edge_ids, turnings, lane_ids)
  f.write('        </Intersections>\n')
  f.write('      </Nodes>\n')


def print_xml_format(nodes, edges, lanes, turnings):
  #We need integer-style IDs for sim mobility
  node_ids = {}
  link_ids = {}
  edge_ids = {}
  lane_ids = {}

  #Cache node and edge IDs
  for n in nodes.values():
    node_ids[n.nodeId] = len(node_ids)+1
  for e in edges.values():
    edge_ids[e.edgeId] = len(edge_ids)+1
  for l in lanes.values():
    lane_ids[l.laneId] = len(lane_ids)+1

  #Open, start writing
  f = open('simmob.network.xml', 'w')
  f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
  f.write('<geo:SimMobility\n')
  f.write('    xmlns:geo="http://www.smart.mit.edu/geo"\n')
  f.write('    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \n')
  f.write('    xsi:schemaLocation="http://www.smart.mit.edu/geo   ../shared/geospatial/xmlLoader/geo10.xsd">\n\n')

  f.write('    <GeoSpatial>\n')
  f.write('    <RoadNetwork>\n')
  write_xml_nodes(f, nodes, node_ids, edges, edge_ids, turnings, lane_ids)
  f.write('    </RoadNetwork>\n')
  f.write('    </GeoSpatial>\n')

  #Done
  f.close()



def run_main(inFile):
  #Result containers
  edges = {}
  lanes = {}
  nodes = {}
  turnings = []

  #Load, parse
  inFile = open(inFile)
  doc = objectify.parse(inFile)

  #For each edge; ignore "internal"
  edgeTags = doc.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    parse_edge(e, edges, lanes)

  #For each junction
  junctTags = doc.xpath('/net/junction')
  for j in junctTags:
    parse_junctions(j, nodes)

  #Remove junction nodes which aren't referenced by anything else.
  nodesPruned = len(nodes)
  remove_unused_nodes(nodes, edges)
  nodesPruned -= len(nodes)
  print("Pruned unreferenced nodes: %d" % nodesPruned)

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




