from geo.formats import osm
from geo.formats import simmob
from geo.helper import IdGenerator
from geo.helper import make_lane_edges_from_lane_lines
from geo.helper import make_lane_connectors


def convert(rn :osm.RoadNetwork) -> simmob.RoadNetwork:
  '''Convert an OSM Road Network to a Sim Mobility network.'''
  res = simmob.RoadNetwork()

  #Simple GUID
  global_id = IdGenerator(1000)

  #Map between old and new Nodes
  ndLookup = {} #OSM Node ID => (generated) Node

  #Every Node can be converted by projecting, 
  # mirroring its x component and scaling by a factor of 100.
  #TODO: We *can* preserve sumo IDs later, with the "orig-id" tag (or something similar)
  zone = None
  for nd in rn.nodes.values():
    projected,zone = project_wgs84(nd.loc.lat, nd.loc.lng, zone)
    newNode = simmob.Node(global_id.next(), -100*projected.x, 100*projected.y)
    res.nodes[newNode.nodeId] = newNode
    ndLookup[nd.nodeId] = newNode

  #Now add one segment per node pair. (For now Sim Mobility does not handle polylines that well).
  #TODO: We might not be handling UniNodes that well here.
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


  #Generate lane connectors from/to all Lanes.
  #TODO: Will this work?
  #make_lane_connectors(res)

  #Some consistency checks
  __check_multi_uni(res.links)

  return res


def __check_multi_uni(links):
  #Check if Multi/Uni nodes are assigned properly.
  #TODO: This is a temporary function; we need to incorporate it later.
  segmentsAt = {} #nodeID => [segment,segment,...]
  nodeType = {} #nodeID => Uni(true)/Multi(false)
  for lk in links.values():
    #Save time later:
    nodeType[lk.segments[0].fromNode.nodeId] = False
    nodeType[lk.segments[-1].toNode.nodeId] = False

    for sg in lk.segments:
      #Add it if it doesn't exist.
      if not sg.fromNode in segmentsAt:
        segmentsAt[sg.fromNode] = []
      if not sg.toNode in segmentsAt:
        segmentsAt[sg.toNode] = []

      #Increment
      segmentsAt[sg.fromNode].append(e)
      segmentsAt[sg.toNode].append(e)

  #Now double-check the Multi/Uni values.
  for n in segmentsAt.keys():
    segs = segmentsAt[n]
    if len(segs)==0:
      raise Exception('Empty segment accidentally added.')

    #Don't set it unless it's still unset.
    if not n.nodeId in nodeType:
      #This should fail:
      if len(segs)==1:
        raise Exception('Node should be a MultiNode...')

      #Simple. 
      #TODO: What if len(segs)==4? We can share Nodes, right?
      elif len(segs)>2:
        nodeType[n.nodeId] = False

      #Somewhat harder:
      else:
        #The segments here should share one node (this node) and then go to different destinations.
        n1 = segs[0].fromNode if segs[0].toNode==n.nodeId else segs[0].toNode
        n2 = segs[1].fromNode if segs[1].toNode==n.nodeId else segs[1].toNode
        nodeType[n.nodeId] = (n1 != n2)

  #Finally, do our consistency checks.
  for lk in links.values():
    seg_nodes = [nodes[lk.segments[0].fromNode]]
    for e in lk.segments:
      seg_nodes.append(nodes[e.toNode])

    #Begins and ends at a MultiNode? (This shouldn't fail)
    if nodeType[seg_nodes[0].nodeId]:
      raise Exception('UniNode found (%s) at start of Segment where a MultiNode was expected.' % seg_nodes[0].nodeId)
    if nodeType[seg_nodes[-1].nodeId]:
      raise Exception('UniNode found (%s) at end of Segment where a MultiNode was expected.' % seg_nodes[-1].nodeId)

    #Ensure middle nodes are UniNodes. Note that this is not strictly required; we'll just
    #   have to false-segment the nodes that trigger this. 
    #NOTE: This currently affects 10% of all Nodes. 
    #Example: Node 74389907 is a legitimate candidate for Node splitting.
    for i in range(1, len(seg_nodes)-2):
      if not nodeType[seg_nodes[i].nodeId]:
        raise Exception('Non-uni node (%s) in middle of a Segment.' % seg_nodes[i].nodeId)



