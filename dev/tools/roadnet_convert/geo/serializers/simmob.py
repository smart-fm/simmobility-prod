from geo.formats import simmob
from geo.position import Point
import geo.helper
import random


def serialize(rn :simmob.RoadNetwork, outFilePath :str):
  '''Serialize a Road Network to Sim Mobility's XML format.'''
  print("Saving file:", outFilePath)
  out = open(outFilePath, 'w')

  #We'll need an index (naturally)
  rnIndex = simmob.RNIndex(rn)

  #There's a nested dispatch for most of this (just how XML tends to work).
  out.write('<?xml version="1.0" encoding="utf-8" ?>\n')
  out.write('<geo:SimMobility\n')
  out.write('    xmlns:geo="http://www.smart.mit.edu/geo"\n')
  out.write('    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \n')
  out.write('    xsi:schemaLocation="http://www.smart.mit.edu/geo   ../../Basic/shared/geospatial/xmlLoader/geo10.xsd">\n\n')

  out.write('    <GeoSpatial>\n')
  out.write('    <RoadNetwork>\n')
  __write_xml_coordmap(outFilePath, out, rn, rnIndex)
  __write_xml_nodes(out, rn, rnIndex)
  __write_xml_links(out, rn, rnIndex)
  out.write('    </RoadNetwork>\n')
  out.write('    </GeoSpatial>\n')
  out.write('</geo:SimMobility>\n')

  #Done
  out.close()


def __write_xml_random_regions(out, latlng_vals, numGrids, numRegions): 
  #Little easier.
  minLat, minLng, maxLat, maxLng = latlng_vals
  latRng = maxLat - minLat
  lngRng = maxLng - minLng

  #Each "block" of latitude/longitude
  latCmp = latRng / numGrids
  lngCmp = lngRng / numGrids

  #Split the ranges into "numGrids" on each side (e.g., 10x10 if numGrids is 10).
  #Then, pick "numRegions" random regions to fill in.
  regions = {} #(lat,lng block)
  while len(regions) < numRegions:
    nextLat = random.randint(0,numGrids-1)
    nextLng = random.randint(0,numGrids-1)
    regions[(nextLat,nextLng)] = True

  for region in regions:
    #Calculate the region bounds.
    latStart = latCmp*region[0] 
    latEnd = latCmp*(region[0]+1)
    lngStart = lngCmp*region[1] 
    lngEnd = lngCmp*(region[1]+1)

    #Now mutate each region slightly, so that we don't have uniform regions.
    points = __mutate_region(latStart, latEnd, lngStart, lngEnd)

    #Now print it.
    out.write('Region:\n')
    for pt in points:
      out.write('  (%f,%f)\n' % (pt.x, pt.y))


def __mutate_region(latStart, latEnd, lngStart, lngEnd): #Returns 4 Points defining the region.
  #We basically generate one point per quadrant. However, I don't want to finagle what constitutes "up" or "+" for 
  #  latitude/longitude (since I always mess it up), so our algorithm works fine reflected.
  latRng = latEnd - latStart
  lngRng = lngEnd - lngStart

  #First, shrink by removing the outer 12.5%.
  latEnd -= latRng / 8
  latStart += latRng / 8
  lngEnd -= lngRng / 8
  lngStart += lngRng / 8

  #Recalc
  latRng = latEnd - latStart
  lngRng = lngEnd - lngStart

  #TODO: Need to randomize. For now, we just take the 4 corners.
  #TODO: Clockwise?
  return (Point(latStart,lngEnd), Point(latEnd,lngEnd), Point(latEnd,lngStart), Point(latStart,lngStart))


def __write_xml_coordmap(outFilePath, out, rn, rnIndex):
  source_vals = calc_point_bounds(rn, rnIndex) # [minX, minY, maxX, maxY]
  dest_vals = calc_latlng_bounds(source_vals) # [minLat, minLng, maxLat, maxLng]

  #Recursive file opening! Generate and print a random map of regions
  out2 = open(outFilePath+"-RRregions.txt", 'w')
  __write_xml_random_regions(out2, dest_vals, 10, 20)
  out2.close()

  #Each converted file is written with a LinearScale coordmap entry that allows it to be converted to roughly believable lat/lng.
  #TODO: We can actually save the UTM-projection zone here, but Sim Mobility can't do much with that at the moment.
  out.write('      <coordinate_map>\n')
  out.write('        <!-- This allows for a rough conversion to lat/lng. It is not that accurate, but should work for random networks. -->\n')
  out.write('        <linear_scale>\n')
  out.write('          <source>\n')
  out.write('            <x_range>%d : %d</x_range>\n' % (source_vals[0], source_vals[2]))
  out.write('            <y_range>%d : %d</y_range>\n' % (source_vals[1], source_vals[3]))
  out.write('          </source>\n')
  out.write('          <destination>\n')
  out.write('            <longitude_range>%f : %f</longitude_range>\n' % (dest_vals[1], dest_vals[3]))
  out.write('            <latitude_range>%f : %f</latitude_range>\n' % (dest_vals[0], dest_vals[2]))
  out.write('          </destination>\n')
  out.write('        </linear_scale>\n')
  out.write('      </coordinate_map>\n')


#Helper for calc_point_bounds
def update_xy_range(rng, currval):
    rng[0] = min(rng[0], currval)
    rng[1] = max(rng[1], currval)

#TODO: Put this in RN_Index?
def calc_point_bounds(rn, rnIndex):
  #This is a hassle otherwise
  if len(rn.nodes.values())==0:
    raise "Need at least one Node."

  #Prepare our output values
  n = next(iter(rn.nodes.values())) #"First" arbitrary element.
  x_rng = [n.pos.x, n.pos.x]
  y_rng = [n.pos.y, n.pos.y]

  #Iterate through all nodes
  for n in rn.nodes.values():
    update_xy_range(x_rng, n.pos.x)
    update_xy_range(y_rng, n.pos.y)
 
  #We also need all Lane and Lane Edge points
  for lk in rn.links.values():
    for seg in lk.segments:
      for l in seg.lanes:
        for p in l.polyline:
          update_xy_range(x_rng, p.x)
          update_xy_range(y_rng, p.y)
      for le in seg.lane_edges:
        for p in le.polyline:
          update_xy_range(x_rng, p.x)
          update_xy_range(y_rng, p.y)

  #Stretch 1m in each direction, just in case.
  x_rng[0] -= 100
  y_rng[0] -= 100
  x_rng[1] += 100
  y_rng[1] += 100

  #Done
  return [x_rng[0], y_rng[0], x_rng[1], y_rng[1]]


def calc_latlng_bounds(source_vals):
  #First, get the distance for each component.
  x_dist = float(source_vals[2] - source_vals[0])
  y_dist = float(source_vals[3] - source_vals[1])

  #At the equator, 1 degree of latitude is roughly 110.567 km. 1 degree of longitude is roughly 111.321 km at the equator.
  #We are centering our destination bounds on (0lat,0lng) to minimize longitudinal stretching.
  lng_dist = x_dist / 11132100
  lat_dist = y_dist / 11056700

  return [-lat_dist/2, -lng_dist/2, lat_dist/2, lng_dist/2]


def __write_xml_nodes(f, rn, rnIndex):
  #Write Nodes
  f.write('      <Nodes>\n')
  f.write('        <UniNodes>\n')
  __write_xml_uninodes(f, rn, rnIndex)
  f.write('        </UniNodes>\n')
  f.write('        <Intersections>\n')
  __write_xml_multinodes(f, rn, rnIndex)
  f.write('        </Intersections>\n')
  f.write('      </Nodes>\n')


def __write_xml_uninodes(f, rn, rnIndex):
  #Iterate through each node, skipping MultiNodes
  for n in rn.nodes.values():
    if isinstance(n, simmob.Intersection):
      continue

    #The header is the same as for Multi-Nodes
    f.write('          <UniNode>\n')
    f.write('            <nodeID>%s</nodeID>\n' % n.nodeId)
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    #Instaed of "segments at", we have "firstPair", which is just the two segments at that UniNode.
    if not (n.nodeId in rnIndex.segsAtNodes):
      raise Exception("UniNode without Road segments")
    if len(rnIndex.segsAtNodes[n.nodeId]) != 2:
      raise Exception("UniNode requires exactly 2 Road segments; instead, has %d" % len(rnIndex.segsAtNodes[n.nodeId]))

    #The order of these *might* matter (it certainly does later for Connectors) so figure it out.
    firstSeg = rnIndex.segsAtNodes[n.nodeId][0]
    secondSeg = rnIndex.segsAtNodes[n.nodeId][-1]
    if firstSeg.toNode.nodeId == secondSeg.fromNode.nodeId:
      pass
    elif secondSeg.toNode.nodeId == firstSeg.fromNode.nodeId:
      tempSeg = firstSeg
      firstSeg = secondSeg
      secondSeg = tempSeg
    else:
      raise Exception("Segments at UniNode don't appear to match up.")

    #Double-check; all Lane Connectors at firstSeg should point from firstSeg to secondSeg.
    for lc in firstSeg.lane_connectors.values():
      if (lc.fromSegment.segId != firstSeg.segId) or (lc.toSegment.segId != secondSeg.segId):
        raise Exception("Lane Connectors don't match up on UniNode.")
      

    #Write the "firstPair" tag.
    f.write('            <firstPair>\n')
    f.write('              <first>%s</first>\n' % firstSeg.segId)
    f.write('              <second>%s</second>\n' % secondSeg.segId)
    f.write('            </firstPair>\n')

    #Our connectors structure is similar, but doesn't feature the RoadSegment ID (it's implicit).
    f.write('            <Connectors>\n')
    for lc in firstSeg.lane_connectors.values():
      f.write('              <laneFrom>%s</laneFrom>\n' % lc.laneFrom.laneId)
      f.write('              <laneTo>%s</laneTo>\n' % lc.laneTo.laneId)
    f.write('            </Connectors>\n')

    #Done
    f.write('          </UniNode>\n')


def __write_xml_multinodes(f, rn, rnIndex):
  #Iterate through each node, skipping UniNodes
  for n in rn.nodes.values():
    if not isinstance(n, simmob.Intersection):
      continue
    f.write('          <Intersection>\n')
    f.write('            <nodeID>%s</nodeID>\n' % n.nodeId)
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    #Use our "segments at Node" index.
    f.write('            <roadSegmentsAt>\n')
    if (n.nodeId in rnIndex.segsAtNodes):
      for sg in rnIndex.segsAtNodes[n.nodeId]:
        f.write('              <segmentID>%s</segmentID>\n' % sg.segId)
    f.write('            </roadSegmentsAt>\n')

    #Write connectors
    f.write('            <Connectors>\n')
    for sg in rnIndex.segsAtNodes[n.nodeId]:
      if sg.fromNode.nodeId==n.nodeId:  #Skip the reverse Segments at this Node.
        continue

      f.write('              <MultiConnectors>\n')
      f.write('                <RoadSegment>%s</RoadSegment>\n' % sg.segId)
      f.write('                <Connectors>\n')
      for lcGrp in sg.lane_connectors.values():
        for lc in lcGrp:
          #Some last-minute checking. Can remove later, once we stop getting errors.
          if lc.fromSegment.segId != sg.segId:
            raise Exception('Segment contains a LaneConnector which originates at another Segment.')
          if lc.fromSegment.segId != rnIndex.segAtLanes[lc.fromLane.laneId].segId:
            raise Exception('LaneConnector (%s) specifies a different Segment than is actually (%s) the Lane\'s parent.' % (lc.fromSegment.segId, rnIndex.segAtLanes[lc.fromLane.laneId].segId))
          if lc.fromSegment.toNode.nodeId != lc.toSegment.fromNode.nodeId:
            raise Exception('Segments don\'t meet at Node.')
          if lc.fromSegment.toNode.nodeId != n.nodeId:
            raise Exception('Warning: Segments meet at the wrong Node.')
          #End of temporary checks.

          f.write('                  <Connector>\n')
          f.write('                    <laneFrom>%s</laneFrom>\n' % lc.fromLane.laneId)
          f.write('                    <laneTo>%s</laneTo>\n' % lc.toLane.laneId)
          f.write('                  </Connector>\n')
      f.write('                </Connectors>\n')
      f.write('              </MultiConnectors>\n')
    f.write('            </Connectors>\n')

    #Done (with this intersection
    f.write('          </Intersection>\n')


def __write_xml_links(f, rn, rnIndex):
  #Write Links
  f.write('      <Links>\n')
  for lk in rn.links.values():
    f.write('        <Link>\n')
    f.write('          <linkID>%s</linkID>\n' % lk.linkId)
    f.write('          <roadName/>\n')
    f.write('          <StartingNode>%s</StartingNode>\n' % lk.fromNode.nodeId)
    f.write('          <EndingNode>%s</EndingNode>\n' % lk.toNode.nodeId)
    f.write('          <Segments>\n')
    for seg in lk.segments:
      __write_xml_segment(f, seg, rn, rnIndex)
    f.write('          </Segments>\n')
    f.write('        </Link>\n')
  f.write('      </Links>\n')


def __write_xml_segment(f, seg, rn, rnIndex):
  #Each Link has only 1 segment
  seg_width = geo.helper.dist(seg.lane_edges[0].polyline[0],seg.lane_edges[-1].polyline[0])
  f.write('            <Segment>\n')
  f.write('              <segmentID>%s</segmentID>\n' % seg.segId)
  f.write('              <startingNode>%s</startingNode>\n' % seg.fromNode.nodeId)
  f.write('              <endingNode>%s</endingNode>\n' % seg.toNode.nodeId)
  f.write('              <maxSpeed>60</maxSpeed>\n')
  f.write('              <Length>%d</Length>\n' % geo.helper.dist(seg.fromNode,seg.toNode))
  f.write('              <Width>%d</Width>\n' % seg_width)
  f.write('              <polyline>\n')
  f.write('                <PolyPoint>\n')
  f.write('                  <pointID>0</pointID>\n')
  f.write('                  <location>\n')
  f.write('                    <xPos>%d</xPos>\n' % seg.fromNode.pos.x)
  f.write('                    <yPos>%d</yPos>\n' % seg.fromNode.pos.y)
  f.write('                  </location>\n')
  f.write('                </PolyPoint>\n')
  f.write('                <PolyPoint>\n')
  f.write('                  <pointID>1</pointID>\n')
  f.write('                  <location>\n')
  f.write('                    <xPos>%d</xPos>\n' % seg.toNode.pos.x)
  f.write('                    <yPos>%d</yPos>\n' % seg.toNode.pos.y)
  f.write('                  </location>\n')
  f.write('                </PolyPoint>\n')
  f.write('              </polyline>\n')
  f.write('              <laneEdgePolylines_cached>\n')
  __write_xml_lane_edge_polylines(f, seg.lane_edges)
  f.write('              </laneEdgePolylines_cached>\n')
  f.write('              <Lanes>\n')
  __write_xml_lanes(f, seg.lanes, seg_width/float(len(seg.lanes)))
  f.write('              </Lanes>\n')
  f.write('              <Obstacles>\n')  #No obstacles for now, but we might generate pedestrian crossings later.
  f.write('              </Obstacles>\n')
  f.write('            </Segment>\n')


def __write_xml_lane_edge_polylines(f, lane_edges):
  #Relatively simple layout
  curr_id = 0
  for le in lane_edges:
    f.write('                <laneEdgePolyline_cached>\n')
    f.write('                  <laneNumber>%d</laneNumber>\n' % curr_id)
    curr_id += 1
    f.write('                  <polyline>\n')
    pt_id = 0
    for p in le.polyline:
      f.write('                    <PolyPoint>\n')
      f.write('                      <pointID>%d</pointID>\n' % pt_id)
      pt_id += 1
      f.write('                      <location>\n')
      f.write('                        <xPos>%d</xPos>\n' % p.x)
      f.write('                        <yPos>%d</yPos>\n' % p.y)
      f.write('                      </location>\n')
      f.write('                    </PolyPoint>\n')
    f.write('                  </polyline>\n')
    f.write('                </laneEdgePolyline_cached>\n')


def __write_xml_lanes(f, lanes, est_width):
  #Lanes just have a lot of properties; we fake nearly all of them.
  for l in lanes:
    f.write('                <Lane>\n')
    f.write('                  <laneID>%s</laneID>\n' % l.laneId)
    f.write('                  <width>%d</width>\n' % est_width)
    f.write('                  <can_go_straight>true</can_go_straight>\n')
    f.write('                  <can_turn_left>true</can_turn_left>\n')
    f.write('                  <can_turn_right>true</can_turn_right>\n')
    f.write('                  <can_turn_on_red_signal>false</can_turn_on_red_signal>\n')
    f.write('                  <can_change_lane_left>true</can_change_lane_left>\n')
    f.write('                  <can_change_lane_right>true</can_change_lane_right>\n')
    f.write('                  <is_road_shoulder>false</is_road_shoulder>\n')
    f.write('                  <is_bicycle_lane>false</is_bicycle_lane>\n')
    f.write('                  <is_pedestrian_lane>false</is_pedestrian_lane>\n')
    f.write('                  <is_vehicle_lane>true</is_vehicle_lane>\n')
    f.write('                  <is_standard_bus_lane>false</is_standard_bus_lane>\n')
    f.write('                  <is_whole_day_bus_lane>false</is_whole_day_bus_lane>\n')
    f.write('                  <is_high_occupancy_vehicle_lane>false</is_high_occupancy_vehicle_lane>\n')
    f.write('                  <can_freely_park_here>false</can_freely_park_here>\n')
    f.write('                  <can_stop_here>false</can_stop_here>\n')
    f.write('                  <is_u_turn_allowed>false</is_u_turn_allowed>\n')
    f.write('                  <PolyLine>\n')
    pt_id = 0
    for p in l.polyline:
      f.write('                    <PolyPoint>\n')
      f.write('                      <pointID>%d</pointID>\n' % pt_id)
      pt_id += 1
      f.write('                      <location>\n')
      f.write('                        <xPos>%d</xPos>\n' % p.x)
      f.write('                        <yPos>%d</yPos>\n' % p.y)
      f.write('                      </location>\n')
      f.write('                    </PolyPoint>\n')
    f.write('                  </PolyLine>\n')
    f.write('                </Lane>\n')







