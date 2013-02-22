#!/usr/bin/python

import sys
from lxml import objectify
from lxml import etree

#This program converts a SUMO traffic network to Sim Mobility format (this includes 
# flipping the network for driving on the left).

#Simple classes. IDs are always strings
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



def run_main(inFile):
  #Result containers
  edges = {}
  lanes = {}

  #Load, parse
  inFile = open(inFile)
  doc = objectify.parse(inFile)

  #For each edge; ignore "internal"
  edgeTags = doc.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    parse_edge(e, edges, lanes)



if __name__ == "__main__":
  if len(sys.argv) < 2:
    print 'Usage:\n' , sys.argv[0] , '<in_file.net.xml>'
    sys.exit(0)

  run_main(sys.argv[1])





