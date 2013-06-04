from geo.formats import osm
from lxml import objectify
from helper import IdGenerator


def parse(inFileName :str) -> sumo.RoadNetwork:
  '''Parse an OSM network file.'''
  rn = osm.RoadNetwork()

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
    res = osm.Node(n.get('id'), n.get('lat'), n.get('lon'))
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

    #First, add a new Link
    res = osm.Way(wy.get('id'), nodeIds[0], nodeIds[-1])
    rn.ways[res.wayId] = res


##################################################################################################
# TODO: This belongs on the "convert" function.
    raise Exception("not yet")
    #Now add one segment per node pair. (For now Sim Mobility does not handle polylines that well).
    for i in range(len(nodeIds)-1):
      #Retrieve the node IDs
      fromNode = nodeIds[i]
      toNode = nodeIds[i+1]

      #Make an edge
      e = temp.Edge(global_id.next(), fromNode, toNode)
      res.segments.append(e)

      #Sanity check
      if not (fromNode in nodes and toNode in nodes):
        raise Exception('Way references unknown Nodes')

      #We create two vectors, one at the fromNode and one at the toNode. 
      LaneWidth = 3.4
      startVec = DynVect(nodes[fromNode].pos, nodes[toNode].pos)
      endVec = DynVect(nodes[toNode].pos, nodes[fromNode].pos)

      #We can provide a simple "intersection" buffer if the total lane length allows it.
      BufferSize = 10.0
      if (startVec.getMagnitude() > 1.1*BufferSize):
         startVec.scaleVectTo(BufferSize/2.0).translate()
         endVec.scaleVectTo(BufferSize/2.0).translate()

      #Point both away from the median line.
      #These are incremented by a faux-lane-width in order to generate basic lane sizes.
      startVec.scaleVectTo(LaneWidth/2.0).rotateLeft().translate().scaleVectTo(LaneWidth)
      endVec.scaleVectTo(LaneWidth/2.0).rotateRight().translate().scaleVectTo(LaneWidth)

      #Add child Lanes for each Segment
      for lIt in range(numLanes):
        #Fake the shape; we'll have to translate it right back unfortunately.
        shapeStr = '%f,%f %f,%f' % (startVec.getPos().x, startVec.getPos().y, endVec.getPos().x, endVec.getPos().y)
        startVec.translate()
        endVec.translate()

        #Make the new lane.
        #Lane IDs (here) probably don't matter, since they are only used internally.
        newLane = temp.Lane(len(lanes)+100, shapeStr)
        e.lanes.append(newLane)
        lanes[newLane.laneId] = newLane

      #SUMO expects lanes in reverse; we'll do the same here (for now).
      e.lanes.reverse()
##################################################################################################




