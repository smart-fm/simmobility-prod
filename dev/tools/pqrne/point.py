#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import math

def is_close(x1, x2):
    return abs(x1 - x2) < 0.001

class Point:
    def __init__(self, x, y, z=0.0):
        self.x = x
        self.y = y
        self.z = z

    def __repr__(self):
        return "point(%f, %f, %f)" % (self.x, self.y, self.z)

    def is_equal(self, point):
        return self.x == point.x and self.y == point.y and self.z == point.z

    def is_almost_equal(self, point):
        return is_close(self.x, point.x) and is_close(self.y, point.y) and is_close(self.z, point.z)

    def distance(self, point):
        x = self.x - point.x
        y = self.y - point.y
        z = self.z - point.z
        return math.sqrt(x*x + y*y + z*z)

    def manhattan_distance(self, point):
        return abs(self.x - point.x) + abs(self.y - point.y) + abs(self.z - point.z)

    # Although Point models 3D points, it can also model 2D vectors.

    def __sub__(self, point):
        """Given 2 points p and q, the expression "p - q" creates a Point, to be viewed as a
        2D vector."""
        return Point(self.x - point.x, self.y - point.y, 0)

    def __add__(self, vector):
        """Given a point p and a 2D vector v, the expression "p + v" creates a new Point."""
        return Point(self.x + vector.x, self.y + vector.y, self.z)

    def __neg__(self):
        """Given a point (or vector) p, the expression "-p" creates a new Point (or vector)."""
        return Point(-self.x, -self.y, -self.z)

    def __mul__(self, rhs):
        """Given two 2D vectors v and w, the expression "v * w" returns the dot-product of
        u and v."""
        return (self.x * rhs.x) + (self.y * rhs.y)

    def __rmul__(self, scalar):
        """Given a 2D vector v and a number, the expression "s * v" returns a new Vector scaled
        by the number."""
        return Point(scalar * self.x, scalar * self.y, 0)

    def __abs__(self):
        """Given a vector v, the expression "abs(v)" returns the vector's length."""
        return math.hypot(self.x, self.y)

def cross_product(v, w):
    """Given 2 vectors v and w, the expression "cross_product(v, w)" returns the cross-product."""
    # The cross-product of v(x, y, 0) and w(x, y, 0) is Point(0, 0, (v.x * w.y) - (v.y * w.x))
    # Since the
    return (v.x * w.y) - (v.y * w.x)

def intersection_point(p1, p2, p3, p4):
    """Return the intersection point between line L1 from p1 to p2 and line L2 from p3 to p4."""
    # The intersection point p is given by p1 + t * (p2 - p1) where
    # t = ((y4-y3)(x3-x1) - (x4-x3)(y3-y1)) / ((y4-y3)(x2-x1)-(x4-x3)(y2-y1))
    v1 = p2 - p1    # p2.__sub__(p1)
    v2 = p4 - p3    # p4.__sub__(p3)
    v = p3 - p1     # p3.__sub__(p1)
    num = cross_product(v, v2)
    den = cross_product(v1, v2)
    if abs(den) < 0.00001:
        if den > 0:
            den = 0.00001
        else:
            den = -0.00001
    t = num / den
    return p1 + t * v1  # p1.__add__(v1.__rmul__(t))

def nearest_point(p1, p, p2):
    """Return the nearest point (and its distance) on the line from p1 to p2 from point p."""
    # The formula for the equation of the line is described in Section 13.7 "General form of the
    # line equation from two points" of "Mathematics for Computer Graphics, 3rd ed" by John Vince.
    # The procedure for calculating the nearest point to a line from a point is shown in Section
    # 13.10 "Position and distance of the nearest point on a line to a point" of the same book.
    # The procedure does not determine whether the nearest point falls between the 2 end points
    # of the line or outside the line segment.
    if p1.is_almost_equal(p2):
        return p1, abs(p - p1)
    n = Point(p2.y - p1.y, p1.x - p2.x)
    c = p2.x*p1.y - p2.y*p1.x
    s = -(n * p + c) / (n * n)  # -(n.__mul__(p2) + c) / n.__mul__(n)
    r = s * n   # n.__rmul__(s)
    nearest_pt = p + r   # p.__add__(r)
    distance =  abs(r)   # r.abs()
    return nearest_pt, distance

def angle_between_2_lines(p1, p, p2):
    """Return the angle between the line from p to p1 and the line from p to p2."""
    if p.is_almost_equal(p1) or p.is_almost_equal(p2):
        return 0.0
    n = Point(p1.y - p.y, p.x - p1.x)
    m = Point(p2.y - p.y, p.x - p2.x)
    return math.degrees(math.acos((n * m) / (abs(n) * abs(m))))

def is_between(p, p1, p2):
    vec2 = p2 - p1
    vec = p - p1
    if abs(vec) > abs(vec2):
        return False
    if abs(p2 - p) > abs(vec2):
        return False
    return True

def re_arrange_co_linear_points(points):
    points.sort(key = lambda point : point.x)
    return points

def simplify_polyline(polyline):
    """Return a copy of polyline with co-linear points removed."""
    result = list()
    if len(polyline) < 3:
        for point in polyline:
            result.append(point)
        return result

    result.append(polyline[0])
    i = 1
    num_points = len(polyline)
    while i < num_points - 1:
        if result[-1].z != polyline[i].z or polyline[i].z != polyline[i+1].z:
            result.append(polyline[i])
        else:
            discard, distance = nearest_point(result[-1], polyline[i], polyline[i+1])
            if abs(distance) > 0.001:
                result.append(polyline[i])
        i = i + 1
    result.append(polyline[-1])
    return result

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
