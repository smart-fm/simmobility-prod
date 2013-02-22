#!/usr/bin/python

import sys
from lxml import objectify
from lxml import etree

#This program converts a SUMO traffic network to Sim Mobility format (this includes 
# flipping the network for driving on the left).

#Simple classes
class Edge:
  def __init__(self, edgeId, fromNode, toNode):
    self.edgeId = int(edgeId)
    self.fromNode = int(fromNode)
    self.toNode = int(toNode)




def parse_edge(e, edges):
    #Sanity check
    if e.get('function')=='internal':
      raise Exception('Node with from/to should not be internal')

    #Add a new edge
    res = Edge(e.get('id'), e.get('from'), e.get('to'))
    edges[res.edgeId] = res



def run_main(inFile):
  #Result containers
  edges = {}

  #Load, parse
  inFile = open(inFile)
  doc = objectify.parse(inFile)

  #For each edge; ignore "internal"
  edgeTags = doc.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    parse_edge(e, edges)



if __name__ == "__main__":
  if len(sys.argv) < 2:
    print 'Usage:\n' , sys.argv[0] , '<in_file.net.xml>'
    sys.exit(0)

  run_main(sys.argv[1])





