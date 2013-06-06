from geo.formats import simmob


def serialize(rn :simmob.RoadNetwork, outFilePath :str):
  '''Serialize a Road Network to Sim Mobility's old "out.txt" format.'''
  print("Saving file:", outFilePath)
  out = open(outFilePath, 'w')
  out.write('("simulation", 0, 0, {"frame-time-ms":"100",})\n')  #frame-time doesn't matter, but it's required.

  #Nodes
  for n in rn.nodes.values():
    if isinstance(n, simmob.Intersection):
      out.write('("multi-node", 0, %s, {"xPos":"%d","yPos":"%d"})\n' % (n.nodeId, n.pos.x, n.pos.y))
    else:
      out.write('("uni-node", 0, %s, {"xPos":"%d","yPos":"%d"})\n' % (n.nodeId, n.pos.x, n.pos.y))

  #Links, edges
  for lk in rn.links.values():
    #Write the link (and its forward segments)
    fromId = lk.fromNode.nodeId
    toId = lk.toNode.nodeId
    out.write('("link", 0, %s, {"road-name":"","start-node":"%s","end-node":"%s","fwd-path":"[' % (lk.linkId, fromId, toId))
    for seg in lk.segments:
      out.write('%s,' % seg.segId)
    out.write(']",})\n')

    #Write the segments one-by-one
    for seg in lk.segments:
      fromId = seg.fromNode.nodeId
      toId = seg.toNode.nodeId
      out.write('("road-segment", 0, %s, {"parent-link":"%s","max-speed":"65","width":"%d","lanes":"%d","from-node":"%s","to-node":"%s"})\n' % (seg.segId, lk.linkId,(250*len(seg.lanes)), len(seg.lanes), fromId, toId))

      #Lanes are somewhat more messy
      #Note that "lane_id" here just refers to lane line 0, since lanes are grouped by edges in this output format. (Confusingly)
      out.write('("lane", 0, %s, {"parent-segment":"%s",' % (seg.lanes[0].laneId, seg.segId))

      #Each lane component
      i = 0
      for l in seg.lane_edges:
        out.write('"lane-%d":"[' % (i))

        #Lane lines are mildly more complicated, since we need lane *edge* lines.
        for p in l.polyline:
          out.write('(%d,%d),' % (p.x, p.y))
        out.write(']",')
        i+=1

      #And finally
      out.write('})\n')

  #We'll need an Index for Lane Connectors
  rnIndex = simmob.RNIndex(rn)

  #Write all Lane Connectors
  currLC_id = 1
  for lc in rnIndex.lane_connectors.values():
    out.write('("lane-connector", 0, %d, {"from-segment":"%d","from-lane":"%d","to-segment":"%d","to-lane":"%d",})\n' % (currLC_id, lc.fromSegment.segId, lc.fromLane.laneNumber, lc.toSegment.segId, lc.toLane.laneNumber))
    currLC_id += 1

  #Done
  out.close()


