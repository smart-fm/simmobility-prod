from lxml import objectify
import geo.formats.osm
from geo.helper import IdGenerator


def parse(inFileName :str) -> geo.formats.osm.RoadNetwork:
  '''Parse an OSM network file.'''
  rn = geo.formats.osm.RoadNetwork()

  #Open, parse, 
  inFile = open(inFileName)
  rootNode = objectify.parse(inFile)

  #All Nodes
  #zone = None #Ensure we stay in the same zone for all points.
  nodeTags = rootNode.xpath('/osm/node')
  for n in nodeTags:
    __parse_nodes_osm(n, rn)

  #All Ways
  wayTags = rootNode.xpath("/osm/way")
  global_id = IdGenerator(1000) #For edges
  for wy in wayTags:
    __parse_way_osm(wy, rn, global_id)

  #Done
  inFile.close()
  return rn


def __parse_node_osm(n, rn):
    #Nodes are slightly complicated by the fact that they use lat/long.
    #NOTE: This is done in "convert"
    #projected,zone = project_wgs84(n.get('lat'), n.get('lon'), zone)  #Returns a point

    #Add a new Node
    res = geo.formats.osm.Node(n.get('id'), n.get('lat'), n.get('lon'))
    rn.nodes[res.nodeId] = res

    #Return the zone, for reference
    #return zone


def __parse_way_osm(wy, rn, global_id):
    #Skip everything except primary highways
    #TODO: This is a temporary fix; there are better ways to handle this.
    tag = wy.xpath("tag[@k='highway']")
    if len(tag) == 0:
      return
    if (tag[0].get('v')!='primary'):
      return

    #
    #TODO: These properties might be useful, but we have to make them optional.
    #
    #Skip footways.
    #tag = wy.xpath("tag[@k='highway']")
    #if len(tag) > 0:
    #  if (tag[0].get('v')=='footway'):
    #    return
    #
    #Skip anything with a post code (likely a building), or a "residential" land use tag.
    #if len(wy.xpath("tag[@k='addr:postcode']"))>0:
    #  return
    #tag = wy.xpath("tag[@k='landuse']")
    #if len(tag) > 0:
    #  if (tag[0].get('v')=='residential'):
    #    return
    #
    #Remove service highways.
    #tag = wy.xpath("tag[@k='highway']")
    #if len(tag) > 0:
    #  if (tag[0].get('v')=='service'):
    #    return

    #First, build up a series of Node IDs
    nodeIds = []
    for ndItem in wy.iter("nd"):
      nId = ndItem.get('ref')
      if not nId:
        raise Exception('No "ref" for "nd" in "way"')
      nodeIds.append(nId)

    #Ensure we have at least two points.
    if len(nodeIds) < 2:
      print('Warning: Skipping an OSM Way, as it does not have at least 2 node refs: %s' % wy.get('id'))
      return

    #Attempt to extract the lane count; default to 1
    numLanes = 1
    numLanesTag = wy.xpath("tag[@k='lanes']")
    if len(numLanesTag) > 0:
      numLanes = int(numLanesTag[0].get('v'))

    #Add a new Way
    res = geo.formats.osm.Way(wy.get('id'), nodeIds[0], nodeIds[-1])
    rn.ways[res.wayId] = res






