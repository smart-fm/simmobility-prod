import geo.formats.sumo
from lxml import objectify


def parse(inFileName :str) -> geo.formats.sumo.RoadNetwork:
  '''Parse a SUMO network file.'''
  rn = geo.formats.sumo.RoadNetwork()

  #Open, parse, 
  inFile = open(inFileName)
  rootNode = objectify.parse(inFile)

  #For each junction
  junctTags = rootNode.xpath('/net/junction')
  for j in junctTags:
    __parse_junction(j, rn)

  #For each edge; ignore "internal"
  edgeTags = rootNode.xpath("/net/edge[(@from)and(@to)]")
  for e in edgeTags:
    if e.get('function')=='internal':
      raise Exception('Node with from/to should not be internal')
    __parse_edge(e, rn)

  #Done
  inFile.close()
  return rn


def __parse_junction(j, rn :geo.formats.sumo.RoadNetwork):
    #Add a new Junction
    res = geo.formats.sumo.Junction(j.get('id'), j.get('x'), j.get('y'))
    rn.junctions[res.jnctId] = res


def __parse_edge(e, rn :geo.formats.sumo.RoadNetwork):
    #Add a new edge
    res = geo.formats.sumo.Edge(e.get('id'), rn.junctions[e.get('from')], rn.junctions[e.get('to')])
    rn.edges[res.edgeId] = res

    #Add child Lanes
    laneTags = e.xpath("lane")
    for l in laneTags:
      newLane = geo.formats.sumo.Lane(l.get('id'), geo.formats.sumo.Shape(l.get('shape')))
      res.lanes.append(newLane)


