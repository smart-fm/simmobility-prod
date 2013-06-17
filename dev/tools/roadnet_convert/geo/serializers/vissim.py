from geo.formats import simmob
import geo.helper


def serialize(rn :simmob.RoadNetwork, outFilePath :str):
  '''Serialize a Road Network to VISSIM's format.'''
  print("Saving file:", outFilePath)
  out = open(outFilePath, 'w')

  out.write('#\n##Note: This file must be incorporated into an existing .inp file.\n#\n\n')

  max_id = __write_vissim_links(out, rn)
  __write_vissim_connectors(out, rn, max_id+10)

  #Done
  out.close()


def __write_vissim_links(out :file, rn :simmob.RoadNetwork):
  '''A VISSIM Link is more like a Sim Mobility Segment'''
  out.write('-- Links: --\n')
  out.write('------------\n\n')
  max_id = 0
  for ln in rn.links.values():
    for seg in ln.segments:
      #Basic stuff.
      out.write('LINK      %s NAME "%s" LABEL  %s %s\n' % (seg.segId, "", 0.0, 0.0)) 
      out.write('  BEHAVIORTYPE     %s   DISPLAYTYPE     %s \n' % (1, 1))
      out.write('  LENGTH  %s LANES  %s ' % (geo.helper.dist(seg.fromNode, seg.toNode)/100.0, len(seg.lanes))
      if len(seg.lanes)==0:
        raise Exception('Segment must have at least 1 lane.')
      max_id = max(max_id, seg.segId)

      #Lane widths
      lw = geo.helper.get_lane_widths(seg.lane_edges)
      if len(lw) != len(seg.lanes):
        raise Exception('Lane widths mismatches number of lanes.')
      out.write('LANE_WIDTH')
      for w in lw:
        out.write('  %s' % w/100.0)

      #More simple properties.
      seg_len = 10.00 #No idea what this is.
      out.write(' GRADIENT %s   COST %s SURCHARGE %s SURCHARGE %s SEGMENT LENGTH   %s \n' % (0.00000, 0.00000, 0.00000, 0.00000, seg_len))

      #Nodes are listed explicitly:
      out.write('  FROM  %s %s\n' % (seg.fromNode.pos.x/100.0, seg.fromNode.pos.y/100.0))
      out.write('  TO    %s %s\n' % (seg.toNode.pos.x/100.0, seg.toNode.pos.y/100.0))

  #Done
  return max_id


def __write_vissim_connectors(out :file, rn :simmob.RoadNetwork, start_lc_id :int):
  out.write('-- Connectors: --\n')
  out.write('-----------------\n\n')

  #VISSIM connectors are from/to Lanes based on their "number" (not ID), starting from 1 (not 0).
  globalId = helper.IdGenerator(start_lc_id) #LaneConnectors have no IDs in Sim Mobility.
  for lnk in rn.links.values():
    for seg in lnk.segments:
      for lc in seg.lane_connectors.values():
        #TODO: For now, we can *only* connect "1" to "1", "2" to "2", etc. See if we can fix this later.
        if lc.fromLane.laneNumber == lc.toLane.laneNumber:
          out.write('CONNECTOR %s NAME "%s" LABEL  %s %s\n' % (globalId.next(), "", 0.00, 0.00))

          #We estimate the stop-line distance as 10m before the segment's end (if there's enough length).
          stopLineDist = (geo.helper.dist(seg.fromNode, seg.toNode)/100)
          if stopLineDist > 10.0:
            stopLineDist -= 10.0
          out.write('  FROM LINK %s LANES %s AT %s\n' % (lc.fromSegment.segId, lc.fromLane.laneNumber, stopLineDist))
          
          #Next is "over"  --I'm not really sure what this is.
          #'  OVER 81.31166 -20.52048 0.00000  OVER 81.352 -20.550 0.000  OVER 81.540 -36.642 0.000  OVER 81.512 -36.683 0.000  \n'

          #Next estimate the stop(start)-line distance for the "to" segment:
          stopLineDist = (geo.helper.dist(lc.toSegment.fromNode, lc.toSegment.toNode)/100)
          if stopLineDist > 10.0:
            stopLineDist = 10.0
          out.write('  TO LINK %s LANES %s AT %s  BEHAVIORTYPE %s  DISPLAYTYPE %s  ALL\n' % (lc.toSegment.segId, lc.toLane.laneNumber, stopLineDist, 1, 1))

          #Not sure what these mean.
          out.write('  DX_EMERG_STOP %s DX_LANE_CHANGE %s\n' % (5.000, 200.000))

          #These are simpler, but we don't use them at the moment.
          out.write('  GRADIENT %s  COST %s  SURCHARGE %s  SURCHARGE %s\n' % (0.00000, 0.00000, 0.00000, 0.00000))
          out.write('  SEGMENT LENGTH %s ANIMATION \n' % 10.000)

