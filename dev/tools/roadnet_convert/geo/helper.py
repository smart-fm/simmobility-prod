from geo.LatLongUTMconversion import LLtoUTM
from geo.LatLongUTMconversion import UTMtoLL
from geo.point import Point
import math

#Assert that all values are non-null
def assert_non_null(msg, *args):
  for arg in args:
    if not arg:
      raise Exception(msg)

#Distance between 2 nodes/points
def dist(m, n):
  #Convert the various 'Node' types to Point
  if hasattr(m, 'pos'):
    m = m.pos
  if hasattr(n, 'pos'):
    n = n.pos

  #Calc distance
  dx = n.x - m.x;
  dy = n.y - m.y;
  return math.sqrt(dx**2 + dy**2)


#Assumes first/second are parallel lines
def get_line_dist(first, second):
  #Slope is vertical?
  dx= first[-1].x-first[0].x
  if (dx==0):
    return abs(first[0].y-second[0].y)

  #Otherwise, get y-intercept for each line.
  m1 = (first[-1].y-first[0].y) / float(dx)
  m2 = (second[-1].y-second[0].y) / float(dx)
  m = (m1+m2)/2.0  #We use the average slope just in case the lines aren't *quite* parallel
  fB = first[0].y - m * first[0].x
  sB = second[0].y - m * second[0].x

  #And then we have a magic formula:
  return abs(sB-fB) / math.sqrt(m**2 + 1)


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


#A basic vector (in the geometrical sense)
class DynVect:
  def __init__(self, start, end):
    #Ensure that we copy.
    self.pos = Point(float(start.x), float(start.y))
    self.mag = Point(float(end.x)-start.x, float(end.y)-start.y)

  def getPos(self):
    return self.pos

  def translate(self):
    self.pos.x += self.mag.x
    self.pos.y += self.mag.y
    return self

  def scaleVectTo(self, amount):
    #Factoring in the unit vector by dividing early is more accurate.
    factor = amount/self.getMagnitude()
    self.mag.x = factor * self.mag.x
    self.mag.y = factor * self.mag.y
    return self

  def getMagnitude(self):
    return math.sqrt(self.mag.x**2.0 + self.mag.y**2.0)

  def flipNormal(self, clockwise):
    sign =  1.0 if clockwise else -1.0
    newX = self.mag.y*sign
    newY = -self.mag.x*sign
    self.mag.x = newX
    self.mag.y = newY
    return self

  def rotateRight(self): #Flip this vector 90 degrees clockwise around the origin.
    return self.flipNormal(True)

  def rotateLeft(self):  #Flip this vector 90 degrees counter-clockwise around the origin.
    return self.flipNormal(False)
