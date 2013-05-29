from geo.LatLongUTMconversion import LLtoUTM
from geo.LatLongUTMconversion import UTMtoLL
from geo.point import Point

#Project coordinates (TODO: currently very rigid)
def project_coords(wgsRev, utmZone, lat, lon):
  #Make sure we have both params, convert to float.
  if not (lat and lon):
    raise Exception('lat/lon required in project_coords')
  lat = float(lat)
  lon = float(lon)

  #Make sure they are using the latest standard
  if (wgsRev.replace(' ', '') != 'WGS84'):
    raise Exception('Deprecated WGS specification (only WGS 84 supported)')

  #Now, perform the projection. Make sure our result matches our expectations.
  (resZone, x, y) = LLtoUTM(23, lat, lon)
  if (utmZone.replace(' ', '') != "UTM"+resZone.replace(' ', '')):
    raise Exception('Resultant UTM zone (%s) does not match expected zone (%s).' % (resZone,utmZone))

  #All is good; return a Point
  return Point(x,y)



#Helper class for UTM scaling
class ScaleHelper:
  def __init__(self):
    self.bounds = [None, None, None, None] #minX,minY,maxX,maxY
    self.center = project_coords('WGS 84', 'UTM 48N', 1.305, 103.851)  #Singapore, roughly

  #Add a point to the bounds
  def add_point(self, pt):
    self.bounds[0] = min(x for x in [pt.x, self.bounds[0]] if x is not None)
    self.bounds[1] = min(y for y in [pt.y, self.bounds[1]] if y is not None)
    self.bounds[2] = max(x for x in [pt.x, self.bounds[2]] if x is not None)
    self.bounds[3] = max(y for y in [pt.y, self.bounds[3]] if y is not None)

  #Convert a point to lat/long
  def convert(self, pt):
    #Convert this point to an offset "relative" to the center of the bounds, then add this to our UTM center.
    offset = Point(self.bounds[0]+(self.bounds[2]-self.bounds[0])/2,self.bounds[1]+(self.bounds[3]-self.bounds[1])/2)
    newPt = Point(self.center.x+(pt.x-offset.x), self.center.y+(pt.y-offset.y))

    #Finally, convert back to lat/lng
    return UTMtoLL(23, newPt.y, newPt.x, '48N')

  #Get the minimum/maximum points
  def min_pt(self):
    return Point(self.bounds[0], self.bounds[1])
  def max_pt(self):
    return Point(self.bounds[2], self.bounds[3])

