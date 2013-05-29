#Contains an (x,y) point, usually in projected coords.
class Point:
  def __init__(self, x, y):
    self.x = x
    self.y = y

  def __repr__(self):
    return "Point(%f,%f)" % (self.x, self.y)

  def __str__(self):
    return "(%f,%f)" % (self.x, self.y)

