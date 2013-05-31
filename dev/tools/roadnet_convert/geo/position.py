#Contains an (x,y) point, usually in projected coords.
class Point:
  def __init__(self, x:float, y:float):
    self.x = x
    self.y = y

  def __repr__(self):
    return "Point(%f,%f)" % (self.x, self.y)

  def __str__(self):
    return "(%f,%f)" % (self.x, self.y)


#Contains a (lat/lng) location, usually as +/- rather than E/W
class Location:
  def __init__(self, lat:float, lng:float):
    self.lat = lat
    self.lng = lng

  def __repr__(self):
    return "Location(%f,%f)" % (self.lat, self.lng)

  def __str__(self):
    return "(%f,%f)" % (self.lat, self.lng)


