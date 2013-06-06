from geo.formats import simmob
from geo.helper import ScaleHelper


def serialize(rn :simmob.RoadNetwork, outFilePath :str, outBounds :'[Location,Location] (min,max)'):
  '''Serialize a Road Network to Open Street Map's OSM format.
     Note that a generalized network may include data that never had an associated lat/lng
     bounding box. Thus, it is impossible to reverse the UTM transform, since we won't know 
     the zone. Therefore, output to OSM requires (for now) a "bounding box" describing 
     min/max lat/lng.
     TODO: There must be some way of getting around this... maybe an optional tag that 
           saves any known UTM zone? Otherwise, OSM->OSM won't perform correctly.
     TODO: Currently we linearly scale the points to the lat/lng bounding box. This is 
           inaccurate (we should reverse-transform via UTM), but the previous problem 
           is more indicative of the general issue than this.
  '''

  #The header starts out innocently enough...
  print("Saving file:", outFilePath)
  out = open(outFilePath, 'w')
  out.write('<?xml version="1.0" encoding="UTF-8"?>\n')
  out.write('<osm version="0.6" generator="convert.py" copyright="OpenStreetMap and contributors" attribution="http://www.openstreetmap.org/copyright" license="http://opendatacommons.org/licenses/odbl/1-0/">\n')

  #Now we need to know the network bounds as well as a reverse transformation.
  helper = ScaleHelper(outBounds)

  #Figure out the bounds
  for n in rn.nodes.values():
    helper.add_point(n.pos)
  for lk in rn.links.values():
    for seg in lk.segments:
      for l in seg.lane_edges:
        for pos in l.points:
          helper.add_point(pos)

  #Now we can just use the helper as-is
  f.write('<bounds minlat="%f" minlon="%f" maxlat="%f" maxlon="%f"/>' % (latLngBounds[0].lat, latLngBounds[0].lng, latLngBounds[1].lat, latLngBounds[1].lng))

  #Nodes
  for n in rn.nodes.values():
    (lat,lng) = helper.convert(n.pos)
    f.write(' <node id="%s" lat="%s" lon="%s" visible="true"/>\n' % (n.nodeId, lat, lng))

  #Ways are tied to segments
  for lk in rn.links.values():
    for seg in lk.segments:
      f.write(' <way id="%s" visible="true">\n' % seg.segId)
    
      #We need to write the Nodes of this Way in order.
      #For now, SUMO links only have 2 nodes and 1 segment each.
      #TODO: Deal with this better, esp. with regards to UniNodes.
      f.write('  <nd ref="%s"/>\n' % e.fromNode.nodeId)
      f.write('  <nd ref="%s"/>\n' % e.toNode.nodeId)

      #Now write the Way's tags
      #TODO: Better customization of output tags.
      f.write('  <tag k="highway" v="primary"/>\n')
      f.write('  <tag k="oneway" v="yes"/>\n')

      f.write(' </way>\n')

  #Done
  f.write('</osm>\n')
  out.close()




