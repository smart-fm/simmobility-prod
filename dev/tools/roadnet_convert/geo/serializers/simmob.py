from geo.formats import simmob


def serialize(outFilePath :str, rn :simmob.RoadNetwork):
  '''Serialize a Road Network to Sim Mobility's XML format.'''
  out = open(outFilePath, 'w')

  #We'll need an index (naturally)
  rnIndex = simmob.RNIndex(rn)

  #There's a nested dispatch for most of this (just how XML tends to work).
  f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
  f.write('<geo:SimMobility\n')
  f.write('    xmlns:geo="http://www.smart.mit.edu/geo"\n')
  f.write('    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \n')
  f.write('    xsi:schemaLocation="http://www.smart.mit.edu/geo   ../../Basic/shared/geospatial/xmlLoader/geo10.xsd">\n\n')

  f.write('    <GeoSpatial>\n')
  f.write('    <RoadNetwork>\n')
  __write_xml_nodes(f, rn, rnIndex)
  __write_xml_links(f, rn, rnIndex)
  f.write('    </RoadNetwork>\n')
  f.write('    </GeoSpatial>\n')
  f.write('</geo:SimMobility>\n')

  #Done
  out.close()


def __write_xml_nodes(f, rn):
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
    f.write('            <nodeID>%d</nodeID>\n' % n.nodeId)
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    #Instaed of "segments at", we have "firstPair", which is just the two segments at that UniNode.
    if not (n.nodeId in rnIndex.segsAtNodes):
      raise Exception("UniNode without Road segments")
    if len(rnIndex.segsAtNodes[n.nodeId]) != 2:
      raise Exception("UniNode requires exactly 2 Road segments")

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
    f.write('            <nodeID>%d</nodeID>\n' % n.nodeId)
    f.write('            <location>\n')
    f.write('              <xPos>%d</xPos>\n' % n.pos.x)
    f.write('              <yPos>%d</yPos>\n' % n.pos.y)
    f.write('            </location>\n')

    #Use our "segments at Node" index.
    f.write('            <roadSegmentsAt>\n')
    if (n.nodeId in rnIndex.segsAtNodes):
      for sg in rnIndex.segsAtNodes[n.nodeId]:
        f.write('              <segmentID>%d</segmentID>\n' % sg.segId)
    f.write('            </roadSegmentsAt>\n')

    #Write connectors
    f.write('            <Connectors>\n')
    for sg in rnIndex.segsAtNodes[n.nodeId]:
      f.write('              <MultiConnectors>\n')
      f.write('                <RoadSegment>%d</RoadSegment>\n' % rs.guid)
      f.write('                <Connectors>\n')
      for lc in sg.lane_connectors.values():
        f.write('                  <Connector>\n')
        f.write('                    <laneFrom>%d</laneFrom>\n' % lc.laneFrom.laneId)
        f.write('                    <laneTo>%d</laneTo>\n' % lc.laneTo.laneId)
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
    f.write('          <linkID>%d</linkID>\n' % lk.guid)
    f.write('          <roadName/>\n')
    f.write('          <StartingNode>%d</StartingNode>\n' % rn.nodes[lk.fromNode].guid)
    f.write('          <EndingNode>%d</EndingNode>\n' % rn.nodes[lk.toNode].guid)
    f.write('          <Segments>\n')
    for seg in lk.segments:
      __write_xml_segment(f, seg, rn, rnIndex)
    f.write('          </Segments>\n')
    f.write('        </Link>\n')
  f.write('      </Links>\n')


def __write_xml_segment(f, seg, rn, rnIndex):
  #Each Link has only 1 segment
  seg_width = dist(seg.lane_edges[0].points[0],seg.lane_edges[-1].points[0])
  f.write('            <Segment>\n')
  f.write('              <segmentID>%d</segmentID>\n' % seg.segId)
  f.write('              <startingNode>%d</startingNode>\n' % seg.fromNode.nodeId)
  f.write('              <endingNode>%d</endingNode>\n' % seg.toNode.nodeId)
  f.write('              <maxSpeed>60</maxSpeed>\n')
  f.write('              <Length>%d</Length>\n' % dist(seg.fromNode,seg.toNode))
  f.write('              <Width>%d</Width>\n' % seg_width)
  f.write('              <polyline>\n')
  f.write('                <PolyPoint>\n')
  f.write('                  <pointID>0</pointID>\n')
  f.write('                  <location>\n')
  f.write('                    <xPos>%d</xPos>\n' % se.fromNode.pos.x)
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
    for p in le.points:
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
    f.write('                  <laneID>%d</laneID>\n' % l.guid)
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
    for p in l.shape.points:
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







