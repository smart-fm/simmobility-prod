from geo.formats import sumo
from geo.formats import simmob
from geo.helper import IdGenerator
from geo.helper import make_lane_edges_from_lane_lines
from geo.helper import make_lane_connectors


def convert(rn :sumo.RoadNetwork) -> simmob.RoadNetwork:
  '''Convert a SUMO Road Network to a Sim Mobility network.'''
  res = simmob.RoadNetwork()

  #Simple GUID
  global_id = IdGenerator(1000)

  #Map between old and new Junctions
  jnctLookup = {} #Junction ID => (generated) Node

  #Every Junction can be converted by mirroring its x component and 
  # scaling by a factor of 100.
  #TODO: Currently we do not consider UniNodes for SUMO networks.
  #TODO: We *can* preserve sumo IDs later, with the "orig-id" tag (or something similar)
  for jn in rn.junctions.values():
    newNode = simmob.Node(global_id.next(), -100*jn.pos.x, 100*jn.pos.y)
    res[newNode.nodeId] = newNode
    jnctLookup[jn.jnctId] = newNode

  #Every Edge can be converted by reversing its lanes, generating
  # lane edges, and creating a Link encapsulating it.
  #TODO: UniNode support would also require Links longer than 1 segment.
  #TODO: We *can* preserve sumo IDs later, with the "orig-id" tag (or something similar)
  for ed in rn.edges.values():
    newSeg = simmob.Segment(global_id.next(), jnctLookup[ed.fromJnct], jnctLookup[ed.toJnct])
    newLink = simmob.Link(global_id.next(), newSeg.fromNode, newSeg.toNode)
    newLink.segments.append(newSeg)
    res.links[newLink.linkId] = newLink

    #Reverse and assign lanes
    lnNum = 0
    for ln in reversed(ed.lanes):
      #TODO: For now polylines don't work that well in Sim Mobility, so we just use the first/last points.
      if len(ln.shape.points)<2:
        raise Exception("Can't convert SUMO shape to SIMMOB polyline; not enough points")

      #Don't forget to reverse/scale out polyline points
      p1 = ln.shape.points[0]
      p2 = ln.shape.points[-1]
      poly = [Point(-100*p1.x, 100*p1.y), Point(-100*p2.x, 100*p2.y)]

      #Create it.
      newSeg.lanes.append(simmob.Lane(global_id.next(), lnNum, newSeg, poly))
      lnNum += 1

    #Create and assign lane edges
    make_lane_edges_from_lane_lines(newSeg, global_id)

  #Generate lane connectors from/to all Lanes.
  make_lane_connectors(res)

  return res







