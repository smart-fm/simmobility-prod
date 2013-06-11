import geo.formats.osm
import geo.formats.simmob
from geo.helper import IdGenerator
from geo.helper import DynVect
from geo.position import Point
import geo.helper
import sys


#Some minimal helper classes to help with conversion.
class __RoadNetwork:
  def __init__(self):
    self.nodes = {}  #osm Node ID->__Node
    self.ways = []

class __Node:
  def __init__(self, x, y):
    self.x = x
    self.y = y
    self.inter = False #Definitely an intersection? (start/end)
    self.count = 0     #WaysAt

  def is_uni(self):
    return self.count==1 and not self.inter

class __Way:
  def __init__(self):
    self.nodes = [] #List of Nodes in order. First and last will be multi; rest will be uni.
    self.numLanes = 1 #Number of lanes (default "guess" is >0)
    self.DEBUG_ITEM = ""  #Original Way ID (don't rely on this).


def convert(rn :geo.formats.osm.RoadNetwork) -> geo.formats.simmob.RoadNetwork:
  '''Convert an OSM Road Network to a Sim Mobility network.'''
  
  #The first step is to rebuild the osm.RoadNetwork as __RoadNetwork, such that:
  #  1) Every __Node's (x,y) position is already projected/reversed from lat/lng
  #  2) Every __Node is tagged with "uni" if it should be a UniNode.
  #  3) Every __Way maps between two nodes that are not "uni" --any nodes between the first and last are "uni", of course.
  rn = __preprocess_osm(rn)

  #Now we start to build the actual road network.
  res = geo.formats.simmob.RoadNetwork()

  #Simple GUID
  global_id = IdGenerator(1000)

  #Map between old and new Nodes
  ndLookup = {} #__Node => (generated) Node

  #Thanks to our pre-processing step, __Nodes are now trivial.
  for nodeId,node in rn.nodes.items():
    newNode = None
    if node.is_uni():
      newNode = geo.formats.simmob.Node(global_id.next(), node.x, node.y)
    else:
      newNode = geo.formats.simmob.Intersection(global_id.next(), node.x, node.y)
    res.nodes[newNode.nodeId] = newNode
    ndLookup[node] = newNode

  #__Ways define Links, with each UniNode representing a Segment (We can't handle polylines that well currently).
  for wy in rn.ways:
    #Sanity check.
    if len(wy.nodes)<2:
      raise Exception("OSM Ways must have >=2 Nodes listed.")

    #Make the Link first.
    lnk = geo.formats.simmob.Link(global_id.next(), ndLookup[wy.nodes[0]], ndLookup[wy.nodes[-1]])
    valid = True

    #Now, make a Segment for each pair of Nodes
    prevNode = None
    for nd in wy.nodes:
      if prevNode:
        #TODO: This catches errors which should be caught earlier but aren't (mysteriously).
        if geo.helper.dist(ndLookup[prevNode], ndLookup[nd])<=sys.float_info.epsilon:
          print("Warning, Way", wy.DEBUG_ITEM, "contains a pair of Nodes which are too close. This Link will be skipped")
          valid = False
          break
        seg = geo.formats.simmob.Segment(global_id.next(), ndLookup[prevNode], ndLookup[nd])
        lnk.segments.append(seg)
      prevNode = nd

    #If there was some error, we skip this Link.
    if not valid:
      continue
    res.links[lnk.linkId] = lnk

    #We need Lane polylines for each Segment.
    for seg in lnk.segments:
      #We create two vectors, one at the fromNode and one at the toNode. 
      LaneWidth = 3.4
      startVec = DynVect(seg.fromNode.pos, seg.toNode.pos)
      endVec = DynVect(seg.toNode.pos, seg.fromNode.pos)

      #We can provide a simple "intersection" buffer if the total lane length allows it.
      #TODO: This is not strictly correct; a 6m lane with a UniNode will fail when it could have succeded.
      BufferSize = 10.0
      if (startVec.getMagnitude() > 1.1*BufferSize):
        if isinstance(seg.fromNode, geo.formats.simmob.Intersection):
          startVec.scaleVectTo(BufferSize/2.0).translate()
        if isinstance(seg.toNode, geo.formats.simmob.Intersection):
          endVec.scaleVectTo(BufferSize/2.0).translate()

      #Point both away from the median line.
      #These are incremented by a faux-lane-width in order to generate basic lane sizes.
      startVec.scaleVectTo(LaneWidth/2.0).rotateLeft().translate().scaleVectTo(LaneWidth)
      endVec.scaleVectTo(LaneWidth/2.0).rotateRight().translate().scaleVectTo(LaneWidth)

      #Add child Lanes for each Segment
      laneNum = 1
      for lIt in range(wy.numLanes):
        #Make a polyline array
        poly = [startVec.getPos(), endVec.getPos()]

        #Make the new lane.
        #Lane IDs (here) probably don't matter, since they are only used internally.
        newLane = geo.formats.simmob.Lane(global_id.next(), laneNum, seg, poly)
        seg.lanes.append(newLane)
        laneNum += 1

      #Create and assign lane edges
      geo.helper.make_lane_edges_from_lane_lines(seg, global_id)

  #Generate lane connectors from/to all Lanes.
  geo.helper.make_lane_connectors(res)

  #At this point, we need to fix any negative components (x,y)  ---the Y component will certainly be negative.
  minMax = __get_or_fix_points(res, None)
  __get_or_fix_points(res, minMax)

  #Some consistency checks
  __check_multi_uni(res, res.links)

  return res



def __preprocess_osm(rn :geo.formats.osm.RoadNetwork) ->__RoadNetwork:
  res = __RoadNetwork()

  #Every Node can be converted by projecting, then scaling by a factor of 100.
  #We also mirror the Y-component, since latitude increases going North.
  #TODO: We *can* preserve osm IDs later, with the "orig-id" tag (or something similar)
  zone = None
  for nd in rn.nodes.values():
    projected,zone = geo.helper.project_wgs84(nd.loc.lat, nd.loc.lng, zone)
    res.nodes[nd.nodeId] = __Node(100*projected.x, -100*projected.y)

  #Before moving on, we scan through the list of Ways and remove all Nodes which are directly on top of each other.
  # This will avoid divide-by-zero errors later.
  #TODO: This doesn't actually seem to catch any errors! (NOTE: It is still necessary for *actual* errors).
  pruned_nodes = []
  for wy in rn.ways.values():
    prevNode = None
    new_node_list = []
    for nd in wy.nodes:
      if prevNode:
        if geo.helper.dist(res.nodes[prevNode.nodeId], res.nodes[nd.nodeId])<=sys.float_info.epsilon:
          pruned_nodes.append(nd.nodeId)
          continue
      new_node_list.append(nd)
      prevNode = nd
    if len(new_node_list) != len(wy.nodes):
      wy.nodes = new_node_list

  if len(pruned_nodes)>0:
    print("Warning: The following Nodes were pruned (too close to other Nodes) :",pruned_nodes)


  #We now iterate through every Way from the original OSM network, marking start and end nodes
  #  in addition to counting the number of Ways at each Node.
  for wy in rn.ways.values():
    if len(wy.nodes)<2:
      raise Exception("Ways in an OSM network can't have <2 Nodes")

    #First/last Nodes are always MultiNodes
    res.nodes[wy.nodes[0].nodeId].inter = True
    res.nodes[wy.nodes[-1].nodeId].inter = True

    #And count
    for nd in wy.nodes:
      res.nodes[nd.nodeId].count += 1

  #We iterate through each Way again, this time splitting it into actual __Ways based on is_uni()'s return value.
  for wy in rn.ways.values():
    #Retrieve the number of lanes.
    numLanes = 1 #Guess: 1
    if "lanes" in wy.props:
      numLanes = int(wy.props["lanes"])
      if numLanes<1:
        print("Error, cannot have",numLanes,"lanes.")

    currWay = None
    for nd in wy.nodes:
      #Always count the current Node in the current Way
      currNode = res.nodes[nd.nodeId]
      if currWay:
        currWay.nodes.append(currNode)

      #Done with the current Way?
      if not currNode.is_uni():
        #Finalize the current (old) way.
        if currWay:
          res.ways.append(currWay)

        #Start a new (current) way and add the current Node to it.
        #NOTE: If this is the last Node, then "currWay" will just be useless (this is normal!)
        currWay = __Way()
        currWay.nodes.append(currNode)
        currWay.numLanes = numLanes
        currWay.DEBUG_ITEM = wy.wayId

  return res


def __get_or_fix_points(rn :geo.formats.simmob.RoadNetwork, minMax :'[minX,maxX,minY,maxY]'):
  #Are we fixing or measuring?
  fix = True if minMax else False
  
  #Find every "point" and either measure or fix it.
  for nd in rn.nodes.values():
    minMax = __get_or_fix(nd.pos, minMax, fix)
  for lnk in rn.links.values():
    for sg in lnk.segments:
      for ln in sg.lanes:
        for pt in ln.polyline:
          minMax = __get_or_fix(pt, minMax, fix)
      for le in sg.lane_edges:
        for pt in le.polyline:
          minMax = __get_or_fix(pt, minMax, fix)


def __get_or_fix(pt :Point, minMax :'[minX,maxX,minY,maxY]', fix :bool):
  if fix:
    #Cheat a bit (add 1m to each minimum)
    #TODO: Check sumo2simmob; we need to consolidate these.
    pt.x = minMax[1] - (pt.x-minMax[0]) + 100
    pt.y = minMax[3] - (pt.y-minMax[2]) + 100
  else:
    if minMax:
      minMax[0] = min(minMax[0], pt.x)
      minMax[1] = max(minMax[1], pt.x)
      minMax[2] = min(minMax[2], pt.y)
      minMax[3] = max(minMax[3], pt.y)
    else:
      minMax = [pt.x,pt.x, pt.y,pt.y]


def __check_multi_uni(rn, links):
  #Check if Multi/Uni nodes are assigned properly.
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
      segmentsAt[sg.fromNode].append(sg)
      segmentsAt[sg.toNode].append(sg)

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
    seg_nodes = [rn.nodes[lk.segments[0].fromNode.nodeId]]
    for e in lk.segments:
      seg_nodes.append(rn.nodes[e.toNode.nodeId])

    #Begins and ends at a MultiNode? (This shouldn't fail)
    if nodeType[seg_nodes[0].nodeId]:
      raise Exception('UniNode found (%s) at start of Segment where a MultiNode was expected.' % seg_nodes[0].nodeId)
    if nodeType[seg_nodes[-1].nodeId]:
      raise Exception('UniNode found (%s) at end of Segment where a MultiNode was expected.' % seg_nodes[-1].nodeId)

    #Ensure middle nodes are UniNodes.
    for i in range(1, len(seg_nodes)-2):
      if not nodeType[seg_nodes[i].nodeId]:
        raise Exception('Non-uni node (%s) in middle of a Segment.' % seg_nodes[i].nodeId)



