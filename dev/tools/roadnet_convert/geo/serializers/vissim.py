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


def __write_vissim_links(out, rn :simmob.RoadNetwork):
  '''A VISSIM Link is more like a Sim Mobility Segment'''
  out.write('-- Links: --\n')
  out.write('------------\n\n')
  max_id = 0
  for ln in rn.links.values():
    for seg in ln.segments:
      #Basic stuff.
      out.write('LINK      %s NAME "%s" LABEL  %s %s\n' % (seg.segId, "", 0.0, 0.0)) 
      out.write('  BEHAVIORTYPE     %s   DISPLAYTYPE     %s \n' % (1, 1))
      out.write('  LENGTH  %s LANES  %s ' % (geo.helper.dist(seg.fromNode, seg.toNode)/100.0, len(seg.lanes)))
      if len(seg.lanes)==0:
        raise Exception('Segment must have at least 1 lane.')
      max_id = max(max_id, int(seg.segId))

      #Lane widths
      lw = geo.helper.get_lane_widths(seg.lane_edges)
      if len(lw) != len(seg.lanes):
        raise Exception('Lane widths mismatches number of lanes.')
      out.write('LANE_WIDTH')
      for w in lw:
        out.write('  %s' % (w/100.0))

      #More simple properties.
      seg_len = 10.00 #No idea what this is.
      out.write(' GRADIENT %s   COST %s SURCHARGE %s SURCHARGE %s SEGMENT LENGTH   %s \n' % (0.00000, 0.00000, 0.00000, 0.00000, seg_len))

      #Nodes are listed explicitly:
      out.write('  FROM  %s %s\n' % (seg.fromNode.pos.x/100.0, seg.fromNode.pos.y/100.0))
      out.write('  TO    %s %s\n' % (seg.toNode.pos.x/100.0, seg.toNode.pos.y/100.0))

  #Done
  return max_id


#Useful; from: http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_substring#Python
def longest_common_substring(s1, s2):
  m = [[0] * (1 + len(s2)) for i in range(1 + len(s1))]
  longest, x_longest = 0, 0
  for x in range(1, 1 + len(s1)):
    for y in range(1, 1 + len(s2)):
      if s1[x - 1] == s2[y - 1]:
        m[x][y] = m[x - 1][y - 1] + 1
        if m[x][y] > longest:
          longest = m[x][y]
          x_longest = x
      else:
        m[x][y] = 0
  return s1[x_longest - longest: x_longest]


#Helper
class __SegConn:
  def __init__(self):
    self.fromLanes = {} #lane numbers represented in "from" connectors.
    self.toLanes = {}   #lane numbers represented in "to" connectors.

  #Add lane information to this segment connector.
  def add_conn(self, laneFromNum, laneToNum):
    self.fromLanes[laneFromNum] = True
    self.toLanes[laneToNum] = True

  #Retrieve the longest consecutive range of shared lanes. (e.g., [1,2,3]).
  def get_lane_range(self):
    #Works on any type of array, magically:
    return longest_common_substring(sorted(self.fromLanes), sorted(self.toLanes))



def __write_vissim_connectors(out, rn :simmob.RoadNetwork, start_lc_id :int):
  out.write('\n\n-- Connectors: --\n')
  out.write('-----------------\n\n')

  #VISSIM connectors are from/to Lanes based on their "number" (not ID), starting from 1 (not 0).
  globalId = geo.helper.IdGenerator(start_lc_id) #LaneConnectors have no IDs in Sim Mobility.
  for lnk in rn.links.values():
    for seg in lnk.segments:
      #Connectors in VISSIM are from segment to segment, and (seem) to line up lanes evenly (e.g., lane 1,2,3 => lane 1,2,3). 
      #TODO: For now, this simplification works for our Sim Mobility network. 
      seg_connect = {} #{toSegment,__SegConn}
      for lc in seg.lane_connectors.values():
        if not lc.toSegment in seg_connect:
          seg_connect[lc.toSegment] = __SegConn()
        seg_connect[lc.toSegment].add_conn(lc.fromLane.laneNumber, lc.toLane.laneNumber)

      #Now add a VISSIM connector for each Segment connector.
      for toSeg,sc in seg_connect:
        #Get a range of arrays (e.g., [1,2,3]). Might be empty.
        lanes = sc.get_lane_range()
        if len(lanes)==0:
          continue

        #We also need to estimate the "OVER" quadrant of interior points. This is massively obnoxious, because I don't really 
        #  understand the criteria for it. For now, we just follow some heuristics:
        #  1A) The first point is located about X meters back, where X is 2 times the connector's stop line.
        #   B) The first point is located Y meters laterally, where Y is the middle of the segment minus the width of each lane which is subtracted.
        #  2)  The second point is at the same (X,Y), but 0.05m further along the segment (going forwards).
        #  3)  The third point is located using the same logic as point 1, but with the "to" segment and the start line of the connector.
        #  4)  The fourth point is located similarly to point 2 (0.05 meters further along).
        #TODO:
        over_pts = []

        #Everything else is relatively easy to figure out.
        out.write('CONNECTOR %s NAME "%s" LABEL  %s %s\n' % (globalId.next(), "", 0.00, 0.00))

        #We estimate the stop-line distance as 10m before the segment's end (if there's enough length).
        stopLineDist = (geo.helper.dist(seg.fromNode, seg.toNode)/100)
        if stopLineDist > 10.0:
          stopLineDist -= 10.0
        out.write('  FROM LINK %s LANES %s AT %s\n' % (seg.segId, ' '.join(lanes), stopLineDist))
          
        #TODO:
        if len(over)>0:
          for pt in over_pts:
            out.write('  OVER %s %s %s' % (pt.x, pt.y, 0.00000))
          out.write('  \n')

        #Next estimate the stop(start)-line distance for the "to" segment:
        stopLineDist = (geo.helper.dist(lc.toSegment.fromNode, lc.toSegment.toNode)/100)
        if stopLineDist > 10.0:
          stopLineDist = 10.0
        out.write('  TO LINK %s LANES %s AT %s  BEHAVIORTYPE %s  DISPLAYTYPE %s  ALL\n' % (toSeg.segId, ' '.join(lanes), stopLineDist, 1, 1))

        #Not sure what these mean.
        out.write('  DX_EMERG_STOP %s DX_LANE_CHANGE %s\n' % (5.000, 200.000))

        #These are simpler, but we don't use them at the moment.
        out.write('  GRADIENT %s  COST %s  SURCHARGE %s  SURCHARGE %s\n' % (0.00000, 0.00000, 0.00000, 0.00000))
        out.write('  SEGMENT LENGTH %s ANIMATION \n' % 10.000)

