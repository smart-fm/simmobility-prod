#!/usr/bin/python

import sys
from lxml import objectify
from lxml import etree

#This program converts a SUMO traffic network to Sim Mobility format (this includes 
# flipping the network for driving on the left).

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

class Lane:
  def __init__(self, laneId, shape):
    if not (laneId and shape):
      raise Exception('Null parameters in Lane constructor')
    self.laneId = str(laneId)
    self.shape = Shape(shape)


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


def check_and_flip_and_scale(nodes, edges, lanes):
  #Save the maximum X co-ordinate
  maxX = None

  #Iteraet through Edges; check node IDs
  for e in edges.values():
    if not (nodes.has_key(e.fromNode) and nodes.has_key(e.toNode)):
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



def print_old_format(nodes, edges, lanes):
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
    for l in e.lanes:
      f.write('"lane-%d":"[' % (i))

      #Lane lines also contain the start/end Node
      all_points = [nodes[e.fromNode].pos] + l.shape.points + [nodes[e.toNode].pos]
      for p in all_points:
        f.write('(%d,%d),' % (p.x, p.y))
      f.write(']",')
      i+=1

    #And finally
    f.write('})\n')

  #Done
  f.close()



def run_main(inFile):
  #Result containers
  edges = {}
  lanes = {}
  nodes = {}

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

  #Check network properties; flip X coordinates
  check_and_flip_and_scale(nodes, edges, lanes)

  #Before printing the XML network, we should print an "out.txt" file for 
  #  easier visual verification with our old GUI.
  print_old_format(nodes, edges, lanes)



if __name__ == "__main__":
  if len(sys.argv) < 2:
    print 'Usage:\n' , sys.argv[0] , '<in_file.net.xml>'
    sys.exit(0)

  run_main(sys.argv[1])





