from geo.formats import sumo
from lxml import objectify


def parse(inFileName :str) -> sumo.RoadNetwork:
  '''Parse a SUMO network file.'''
  rn = sumo.RoadNetwork()

  #Open, parse, 
  inFile = open(inFileName)
  rootNode = objectify.parse(inFile)

  #For each junction
  junctTags = rootNode.xpath('/net/junction')
  for j in junctTags:
    __parse_junction(j, rn.junctions)

  #For each edge; ignore "internal"
  edgeTags = rootNode.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    if e.get('function')=='internal':
      raise Exception('Node with from/to should not be internal')
    __parse_edge(e, rn.edges)

  #Done
  inFile.close()
  return rn


def __parse_junction(j, junctions):
    #Add a new Junction
    res = sumo.Junction(j.get('id'), j.get('x'), j.get('y'))
    junctions[res.jnctId] = res


def __parse_edge(e, edges):
    #Add a new edge
    res = sumo.Edge(e.get('id'), e.get('from'), e.get('to'))
    edges[res.edgeId] = res

    #Add child Lanes
    laneTags = e.xpath("lane")
    for l in laneTags:
      newLane = sumo.Lane(l.get('id'), sumo.Shape(l.get('shape')))
      res.lanes.append(newLane)


