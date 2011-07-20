import java.awt.geom.*;

PFont f;
PFont f2;

//Constants
int BUFFER = 95;
int NODE_SIZE = 16;

//Flag; make into a checkbox later
boolean drawDetailedIntersections = true;

//Node zoom level; make into a set of sliders later.
int NODE_ZOOM = 90;
int NODE_ZOOM_INNER = 50;

//Derived
int NODE_LANE_WIDTH = NODE_ZOOM / 8; //Max: 4 lanes per side
int ARROW_CURVE = (NODE_LANE_WIDTH*3)/4;

//Globally managed max/min on all x/y positions (for scaling)
static double[] xBounds = null;
static double[] yBounds = null;


int scalePointForDisplay(double orig, double[] bounds) {
    double percent = (orig - bounds[0]) / (bounds[1] - bounds[0]);
    int scaledMagnitude = ((int)bounds[2] * BUFFER) / 100;
    int newVal = (int)(percent * scaledMagnitude) + ((int)bounds[2]-scaledMagnitude)/2; //Slightly easier to view.
    return newVal;
}

/*int scaleY(double orig) {
    double percent = (orig - yBounds[0]) / (yBounds[1] - yBounds[0]);
    int scaledHeight = (height * BUFFER) / 100;
    int newY = (int)(percent * scaledHeight) + (height-scaledHeight)/2; //Slightly easier to view.
    return newY;
}*/


ArrayList<Node> nodes = new ArrayList<Node>();
class Node {
  int id;
  double xPos;
  double yPos;
  boolean isIntersection;
  
  //For easy lookup
  Set<Section> segmentsFrom = new HashSet<Section>();
  Set<Section> segmentsTo = new HashSet<Section>();
  
  //Individual connectors (for now)
  ArrayList<LaneConnector> connectors = new ArrayList<LaneConnector>();
  
  //For drawing
  Ellipse2D.Double bounds;
  Ellipse2D.Double inner;
  Hashtable<Node, LaneInfo> edgePoints = new Hashtable<Node, LaneInfo>(); 
};
Node getNode(int id) {
  for (int i=0; i<nodes.size(); i++) {
    if (nodes.get(i).id == id) {
      return nodes.get(i);
    }
  }
  throw new RuntimeException("No node with id: " + id);
}

class DPoint {
  double x;
  double y;
  DPoint(double x, double y) {
    this.x = x;
    this.y = y;
  }
}

class LaneArrow {
  DPoint arrowStart;
  DPoint arrowEnd;
  DPoint arrowStartAnchor;
  DPoint arrowEndAnchor;
  DPoint[] arrowWings = new DPoint[2];
}


DPoint getLanesCenter(Line2D[] arr) {
  //Proper naming
  double x1 = arr[0].getX1();
  double y1 = arr[0].getY1();
  double x2 = arr[1].getX2();
  double y2 = arr[1].getY2();
  double x3 = arr[1].getX1();
  double y3 = arr[1].getY1();
  double x4 = arr[0].getX2();
  double y4 = arr[0].getY2();
    
  //Result, segmented
  double d1 = (x1*y2 - y1*x2);
  double d2 = (x3*y4 - y3*x4);
  double den = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
  double xResNum = d1*(x3-x4) - (x1-x2)*d2;
  double yResNum = d1*(y3-y4) - (y1-y2)*d2;


  //Temp
/*  println("Get center:");
  println("    L1: " + arr[0].getX1() + "," + arr[0].getY1() + "," + arr[0].getX2() + "," + arr[0].getY2());
  println("    L2: " + arr[1].getX1() + "," + arr[1].getY1() + "," + arr[1].getX2() + "," + arr[1].getY2());
  println("    Res: " + (xResNum/den) + "," + (yResNum/den));*/


  return new DPoint(xResNum/den, yResNum/den);
}


DPoint getAndScaleNormalVector(DPoint pCenter, DPoint p2, boolean flip, int scaleBy) {
  //Create a unit vector
  DPoint unitVect = new DPoint(p2.x-pCenter.x, p2.y-pCenter.y);
  makeUnit(unitVect);
  
  //Now, Flip the unit vector
  int sign = flip ? 1 : -1;
  DPoint normVect = new DPoint(-unitVect.y*sign, unitVect.x*sign);
  
  //Now scale the normal vector, and apply it to the midpoint.
  scaleVect(normVect, scaleBy);
  return new DPoint(pCenter.x+normVect.x, pCenter.y+normVect.y);
}


LaneArrow makeArrow(LaneConnector lc) {
  LaneArrow res = new LaneArrow();
  
  //First, get the pair of line segments for each lane.
  Line2D[] start = new Line2D[2];
  Line2D[] end = new Line2D[2];
  
  start[0] = getLaneSegmentLine(lc.fromSec.from, lc.fromSec.to, lc.fromLane, false);
  start[1] = getLaneSegmentLine(lc.fromSec.from, lc.fromSec.to, lc.fromLane+1, false);
  
  end[0] = getLaneSegmentLine(lc.toSec.from, lc.toSec.to, lc.toLane, true);
  end[1] = getLaneSegmentLine(lc.toSec.from, lc.toSec.to, lc.toLane+1, true);
  
  //Get the intersection between the lines formed by each segment's endponits, swapped.
  res.arrowStart = getLanesCenter(start);
  res.arrowEnd = getLanesCenter(end);
  
  //Now, get the two points normal to this line at a certain distance.
  DPoint mid = new DPoint((res.arrowStart.x+res.arrowEnd.x)/2, (res.arrowStart.y+res.arrowEnd.y)/2);
  DPoint p1 = getAndScaleNormalVector(mid, res.arrowEnd, true, ARROW_CURVE);
  DPoint p2 = getAndScaleNormalVector(mid, res.arrowEnd, false, ARROW_CURVE);
  
  //Find out which direction the normal vector should face:
  boolean flipParam = (dist((float)p1.x, (float)p1.y, (float)lc.fromSec.to.xPos, (float)lc.fromSec.to.yPos) < dist((float)p2.x, (float)p2.y, (float)lc.fromSec.to.xPos, (float)lc.fromSec.to.yPos));

  //Get the 1/4 and 3/4 points
  DPoint midLHS = new DPoint((res.arrowStart.x+mid.x)/2, (res.arrowStart.y+mid.y)/2);
  DPoint midRHS = new DPoint((res.arrowEnd.x+mid.x)/2, (res.arrowEnd.y+mid.y)/2);

  //Now create and scale the anchor points
  res.arrowStartAnchor = getAndScaleNormalVector(midLHS, res.arrowStart, !flipParam, ARROW_CURVE);
  res.arrowEndAnchor = getAndScaleNormalVector(midRHS, res.arrowEnd, flipParam, ARROW_CURVE);
  
  return res;
}


class LaneInfo {
  Line2D medianLine;   //Line in between left and right lanes.
  ArrayList<Line2D> lanesLeft = new ArrayList<Line2D>();  //Left-most line of each lane.
  ArrayList<Line2D> lanesRight = new ArrayList<Line2D>(); //Right-most line of each lane.
}


class SectionPolylineComparator implements Comparator<DPoint> {
  Node source;
  
  SectionPolylineComparator(Node source) {
    this.source = source;
  }

  int compare(DPoint o1, DPoint o2) {
    //Equal is easy.
    if (o1.x==o2.x && o1.y==o2.y) {
      return 0;
    }
    
    //Less then: for now, just a simple check of distance-from-the-source.
    // To be more accurate, we would first find the point on the line (from, to)
    // that this point intersects at and compare those. 
    double d1 = dist((float)source.xPos, (float)source.yPos, (float)o1.x, (float)o1.y);
    double d2 = dist((float)source.xPos, (float)source.yPos, (float)o2.x, (float)o2.y);
    if (d1 < d2) {
      return -1;
    }
    return 1;
  }
}



ArrayList<Section> sections = new ArrayList<Section>();
class Section {
  int id;
  String name;
  int numLanes;
  int speed;
  int capacity;

  Node from;
  Node to;
  ArrayList<DPoint> polyline = new ArrayList<DPoint>();
};
Section getSection(int id) {
  for (int i=0; i<sections.size(); i++) {
    if (sections.get(i).id == id) {
      return sections.get(i);
    }
  }
  throw new RuntimeException("No section with id: " + id);
}
Section getSection(Node from, Node to) {
  for (Section s : to.segmentsTo) {
    if (s.from.equals(from) && s.to.equals(to)) {
      return s;
    }
  }
  return null; //It might not exist...
}


class LaneConnector {
  //Logical data
  Section fromSec;
  int fromLane;
  Section toSec;
  int toLane;
  
  //Data for drawing this connector.
  LaneArrow arrowMarking;
};


//Get segments of lines
Line2D getLaneSegmentLine(Node from, Node to, int laneID, boolean atFrom) 
{
  if (atFrom) {  
    //Retrive the segment in reverse(laneID) at node "from" on the RHS
    LaneInfo li = from.edgePoints.get(to);
    if (li==null) {
      return new Line2D.Double(0, 0, 0, 0);
    }
    if (laneID==li.lanesRight.size()) { //Special case: center line
      return li.medianLine;
    }
    return li.lanesRight.get(li.lanesRight.size()-1 - laneID);
  } else {
    //Retrive the segment in reverse(laneID) at node "to" on the LHS
    LaneInfo li = to.edgePoints.get(from);
    if (li==null) {
      return new Line2D.Double(0, 0, 0, 0);
    }
    if (laneID==li.lanesLeft.size()) { //Special case: center line
      return li.medianLine;
    }
    return li.lanesLeft.get(li.lanesLeft.size()-1 - laneID);
  }
}


Line2D getLaneLine(Node from, Node to, int laneID) 
{
  //First, retrieve the line in reverse(laneID) at node "from" on the RHS
  LaneInfo li = from.edgePoints.get(to);
  if (li==null) {
    return new Line2D.Double(0, 0, 0, 0);
  }
  Line2D start = li.lanesRight.get(li.lanesRight.size()-1 - laneID);
  
  //Next, retrieve the line in reverse(laneID) at node "to" on the LHS
  li = to.edgePoints.get(from);
  if (li==null) {
    return new Line2D.Double(0, 0, 0, 0);
  }
  Line2D end = li.lanesLeft.get(li.lanesLeft.size()-1 - laneID);
  
  //Now, just make a line from Point2 to Point2 of each Line (since lines always come FROM the center TO the outer edge.)
  return new Line2D.Double(start.getX2(), start.getY2(), end.getX2(), end.getY2());
}



//Our button
Rectangle2D button = null;


void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  frameRate(4);
  
  DPoint btnSize = new DPoint(100, 30);
  button = new Rectangle2D.Double(width-btnSize.x - 10, 10, btnSize.x, btnSize.y);
  
  //Font
  f = createFont("Arial",16,true); 
  f2 = createFont("Arial",12,true); 

  //Load sample input
  try {
    readNodes("nodes.txt");
    readSections("sections.txt");
    readPolylines("polylines.txt");
    readConnectors("connectors.txt");
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
}


void draw()
{
  smooth();
  if (drawDetailedIntersections) {
    background(0x00);
  } else {
    background(0xFF);
  }
  strokeWeight(1.0);
  
  //Draw all nodes
  if (!drawDetailedIntersections) {
    for (int i=0; i<nodes.size(); i++) {
      Node n = nodes.get(i);
    
      //Draw the circle
      stroke(0x99);
      if (n.isIntersection) {
        fill(0x99, 0x00, 0x00);
      } else {
        fill(0x00, 0xCC, 0xCC);
      }
      ellipse((float)n.xPos, (float)n.yPos, NODE_SIZE, NODE_SIZE);
    }
  }
  
  //Draw all sections
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);
    
    //Draw the Euclidean line representing this section.
    //if (!drawDetailedIntersections) {
      if (drawDetailedIntersections) {
        stroke(0xFF, 0xFF, 0x00);
        strokeWeight(1.0);
      } else {
        stroke(0x00, 0x99, 0x00);
        strokeWeight(2.0);
      }
      fill(0x00, 0xCC, 0x00);
      line((float)s.from.xPos, (float)s.from.yPos, (float)s.to.xPos, (float)s.to.yPos);
    //}
    
    //Polyline
    if (!drawDetailedIntersections) {
      stroke(0x00, 0x00, 0x99);
      strokeWeight(0.5);
      if (s.polyline.size()>2) {
        for (int i=1; i<s.polyline.size(); i++) {
          //Draw from X to Xprev
          DPoint from = s.polyline.get(i-1);
          DPoint to = s.polyline.get(i);
          line((float)from.x, (float)from.y, (float)to.x, (float)to.y); 
        }
      }
    }
    
    //Not drawing speed limit or capacity for now...
  }
  
  
  //Draw "detailed" nodes on top of road lines
  Set<String> hasDrawnEdge = new HashSet<String>();
  if (drawDetailedIntersections) {
    for (int i=0; i<nodes.size(); i++) {
      Node n = nodes.get(i);
    
      //Draw the circle
      strokeWeight(1.5);
      fill(0x00);
      if (n.isIntersection) {
        stroke(0xFF, 0x00, 0x00);
      } else {
        stroke(0x99, 0x99, 0x99);
      }
      ellipse((float)n.bounds.getCenterX(), (float)n.bounds.getCenterY(), (int)n.bounds.getWidth(), (int)n.bounds.getHeight());
      fill(0x33);
      stroke(0x33);
      ellipse((float)n.inner.getCenterX(), (float)n.inner.getCenterY(), (int)n.inner.getWidth(), (int)n.inner.getHeight());
      
      //Draw lines to all "edge" points.
      strokeWeight(1.0);
      for (LaneInfo edge : n.edgePoints.values()) {
        //Draw the median.
        Line2D med = edge.medianLine;
        stroke(0xFF, 0x00, 0xFF);
        //stroke(0xFF, 0xFF, 0x00);
        line((float)med.getX1(), (float)med.getY1(), (float)med.getX2(), (float)med.getY2());
        
        //Draw all lanes to the left
        stroke(0xFF, 0x00, 0xFF);
        //stroke(0xFF);
        for (Line2D laneEdge : edge.lanesLeft) {
          line((float)laneEdge.getX1(), (float)laneEdge.getY1(), (float)laneEdge.getX2(), (float)laneEdge.getY2());
        }
        
        //Draw all lanes to the right
        stroke(0xFF, 0x00, 0xFF);
        //stroke(0xFF);
        for (Line2D laneEdge : edge.lanesRight) {
          line((float)laneEdge.getX1(), (float)laneEdge.getY1(), (float)laneEdge.getX2(), (float)laneEdge.getY2());
        }
      }
      
      //Draw lines from this node to another, but only once.
      for (Section s : n.segmentsFrom) {
        String keyVal = s.from.id + ":" + s.to.id;
        if (!hasDrawnEdge.contains(keyVal)) {
          //First, the center line  //No need; it's done already
          //stroke(0xFF, 0xFF, 0x00);
          //Line2D cent = getCenterLine(s.from, s.to);
          //line((float)cent.getX1(), (float)cent.getY1(), (float)cent.getX2(), (float)cent.getY2());
          
          //Now, draw all lanes (these will be "left" on one node and "right" on the other)
          stroke(0xFF);
          for (int laneID=0; laneID<s.numLanes; laneID++) {
            Line2D cent = getLaneLine(s.from, s.to, laneID);
            line((float)cent.getX1(), (float)cent.getY1(), (float)cent.getX2(), (float)cent.getY2());
          }
          
          
          hasDrawnEdge.add(keyVal);
        }
      }
    }
  }
  
  
  
  //Draw lane connectors
  if (drawDetailedIntersections) {
    strokeWeight(1.0);
    stroke(0x00, 0xFF, 0x00);
    noFill();
    for (Node n : nodes) {
      for (LaneConnector lc : n.connectors) {
        LaneArrow la = lc.arrowMarking;        
        bezier(
          (float)la.arrowStart.x, (float)la.arrowStart.y, 
          (float)la.arrowStartAnchor.x, (float)la.arrowStartAnchor.y, 
          (float)la.arrowEndAnchor.x, (float)la.arrowEndAnchor.y,
          (float)la.arrowEnd.x, (float)la.arrowEnd.y
        );
      }
    }
  }
  
  
  //Label nodes
  if (!drawDetailedIntersections) {
    textFont(f);
    textAlign(CENTER);
    for (int i=0; i<nodes.size(); i++) {
      Node n = nodes.get(i);
      fill(0x33);
      text(""+(n.id), (float)n.xPos, (float)n.yPos); 
    }
  }
  
  //Label sections
  //Draw all sections
  textFont(f2);
  textAlign(CENTER);
  Set<String> alreadyDrawn = new HashSet<String>();
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);
    
    //Write the road name. Only do this once for each section, unless there's a conflict.
    String key = Math.min(s.from.id,s.to.id) + ":" + Math.max(s.from.id,s.to.id) + ":" + s.name; 
    strokeWeight(1);
    if (!alreadyDrawn.contains(key)) {
      if (drawDetailedIntersections) {
        fill(0x00, 0x99, 0x99);
      } else {
        fill(0x33);
      }
      text(s.name, (float)(s.from.xPos+(s.to.xPos-s.from.xPos)/2), (float)(s.from.yPos+(s.to.yPos-s.from.yPos)/2)); 
      alreadyDrawn.add(key);
    }
    
    //Not drawing speed limit or capacity for now...
  }
  
  
  //Draw the button
  strokeWeight(2.0);
  textFont(f);
  textAlign(CENTER);  
  if (drawDetailedIntersections) {
    stroke(0xFF);
    fill(0x33, 0x00, 0x00);
  } else {
    stroke(0x00);
    fill(0x99, 0x66, 0x66);
  }
  rect((float)button.getX(), (float)button.getY(), (float)button.getWidth(), (float)button.getHeight());
  String msg = "";
  if (drawDetailedIntersections) {
    msg = "Lane Mode";
    fill(0xFF);
  } else {
    msg = "Shape Mode";
    fill(0x00);
  }  
  text(msg, (float)button.getCenterX(), (float)(button.getCenterY() + 6));
}



void mousePressed() {
  if (button.contains(mouseX, mouseY)) {
    drawDetailedIntersections = !drawDetailedIntersections;
  }
}


void readNodes(String nodesFile) throws IOException
{ 
  String lines[] = loadStrings(nodesFile);
  nodes.clear();
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (agentID, tickID, xPos, yPos, phase)
    String[] items = nextLine.split("\t");
    if (items.length != 4) {
      throw new RuntimeException("Bad line in nodes file: " + nextLine);
    }
    
    //Create a Node, populate it.
    Node n = new Node();
    try {
      n.id = Integer.parseInt(items[0]);
      n.xPos = Double.parseDouble(items[1]);
      n.yPos = Double.parseDouble(items[2]);
      n.isIntersection = myParseBool(items[3]);
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
    
    //Check
    if (xBounds == null) {
      xBounds = new double[] {n.xPos, n.xPos, width};
    } else {
      xBounds[0] = Math.min(xBounds[0], n.xPos);
      xBounds[1] = Math.max(xBounds[1], n.xPos);
    }
    if (yBounds == null) {
      yBounds = new double[] {n.yPos, n.yPos, height};
    } else {
      yBounds[0] = Math.min(yBounds[0], n.yPos);
      yBounds[1] = Math.max(yBounds[1], n.yPos);
    }
    
    nodes.add(n);
  }
  
  //The nodes define the canvas, so scale them all now!
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    n.xPos = scalePointForDisplay(n.xPos, xBounds);
    n.yPos = scalePointForDisplay(n.yPos, yBounds);
    
    //While we're at it....
    n.bounds = new Ellipse2D.Double(n.xPos-NODE_ZOOM/2, n.yPos-NODE_ZOOM/2, NODE_ZOOM, NODE_ZOOM);
    n.inner = new Ellipse2D.Double(n.xPos-NODE_ZOOM_INNER/2, n.yPos-NODE_ZOOM_INNER/2, NODE_ZOOM_INNER, NODE_ZOOM_INNER);
  }
}



void readSections(String sectionsFile) throws IOException
{ 
  String lines[] = loadStrings(sectionsFile);
  sections.clear();
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (agentID, tickID, xPos, yPos, phase)
    String[] items = nextLine.split("\t");
    if (items.length != 7) {
      throw new RuntimeException("Bad line in sections file: " + nextLine);
    }
    
    //Create a Section, populate it.
    Section s = new Section();
    try {
      s.id = Integer.parseInt(items[0]);
      s.name = items[1].trim();
      s.numLanes = Integer.parseInt(items[2]);
      s.speed = Integer.parseInt(items[3]);
      s.capacity = Integer.parseInt(items[4]);
      s.from = getNode(Integer.parseInt(items[5]));
      s.from.segmentsFrom.add(s);
      s.to = getNode(Integer.parseInt(items[6]));
      s.to.segmentsTo.add(s);
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }

    sections.add(s);
  }
}


void makeUnit(DPoint vect) 
{
  double magnitude = Math.sqrt(vect.x*vect.x + vect.y*vect.y);
  scaleVect(vect, 1/magnitude);
}

void scaleVect(DPoint vect, double value) 
{
  vect.x *= value;
  vect.y *= value;
}


LaneInfo getEdgePoint(Node cent, Node outer) 
{  
  LaneInfo res = new LaneInfo();
  
  //Make a vector from the center to the "outer" node, scale it down to the unit vector, then 
  //   scale it back up to the radius of the center.
  DPoint unitVect = new DPoint(outer.xPos-cent.xPos, outer.yPos-cent.yPos);
  makeUnit(unitVect);
  DPoint vect = new DPoint(unitVect.x, unitVect.y);
  scaleVect(vect, cent.bounds.getWidth()/2);
  DPoint edge = new DPoint(cent.xPos+vect.x, cent.yPos+vect.y);
  
  //Compute the inner edge too; save that as the median line.
  vect = new DPoint(unitVect.x, unitVect.y);
  scaleVect(vect, cent.inner.getWidth()/2);
  DPoint inner = new DPoint(cent.xPos+vect.x, cent.yPos+vect.y);
  res.medianLine = new Line2D.Double(inner.x, inner.y, edge.x, edge.y);
  
    
  //Find the segment that goes "to" the center node. That determines all "left" lanes.
  Section s = getSection(outer, cent);
  if (s!=null) {
    //Rotate the unit vector to the left. (I love unit vectors)
    vect = new DPoint(-unitVect.y, unitVect.x);
              
    //Add all lanes left of the median. (We're numbering them backwards, but it won't matter.)
    for (int i=0; i<s.numLanes; i++) {
      DPoint vect2 = new DPoint(vect.x, vect.y);
      scaleVect(vect2, NODE_LANE_WIDTH*(i+1));
      res.lanesLeft.add(
        new Line2D.Double(inner.x+vect2.x, inner.y+vect2.y, edge.x+vect2.x, edge.y+vect2.y)
      );
    }
  }  
  
  //Find the segment that goes "from" the center node. That determines all "right" lanes.
  s = getSection(cent, outer);
  if (s!=null) {
    //Rotate the unit vector to the right. (I stilllove unit vectors)
    vect = new DPoint(unitVect.y, -unitVect.x);
              
    //Add all lanes left of the median. (We're numbering them backwards, but it won't matter.)
    for (int i=0; i<s.numLanes; i++) {
      DPoint vect2 = new DPoint(vect.x, vect.y);
      scaleVect(vect2, NODE_LANE_WIDTH*(i+1));
      res.lanesRight.add(
        new Line2D.Double(inner.x+vect2.x, inner.y+vect2.y, edge.x+vect2.x, edge.y+vect2.y)
      );
    }
  }  
  
  return res;
}


void readPolylines(String polylinesFile) throws IOException
{ 
  String lines[] = loadStrings(polylinesFile);
  
  //Initialize polylines
  for (int i=0; i<sections.size(); i++) {
    Section s = sections.get(i);
    s.polyline.add(new DPoint(s.from.xPos, s.from.yPos));
  }
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (agentID, tickID, xPos, yPos, phase)
    String[] items = nextLine.split("\t");
    if (items.length != 3) {
      throw new RuntimeException("Bad line in polylines file: " + nextLine);
    }
    
    //Tie this polyline to a segment.
    try {
      Section s = getSection(Integer.parseInt(items[0]));
      double x = scalePointForDisplay(Double.parseDouble(items[1]), xBounds);
      double y = scalePointForDisplay(Double.parseDouble(items[2]), yBounds);
      s.polyline.add(new DPoint(x, y));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
  }
  
  //Finalize polylines
  for (int i=0; i<sections.size(); i++) {
    Section s = sections.get(i);
    s.polyline.add(new DPoint(s.to.xPos, s.to.yPos));
    
    //We need to sort it.
    Collections.sort(s.polyline, new SectionPolylineComparator(s.from));
  }
  
  //Finalize all edge points
  for (int i=0; i<sections.size(); i++) {
    Section s = sections.get(i);
    if (!s.from.edgePoints.containsKey(s.to)) {
      s.from.edgePoints.put(s.to, getEdgePoint(s.from, s.to));
    }
    if (!s.to.edgePoints.containsKey(s.from)) {
      s.to.edgePoints.put(s.from, getEdgePoint(s.to, s.from));
    }
  }
}



void readConnectors(String connectorsFile) throws IOException
{ 
  String lines[] = loadStrings(connectorsFile);
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (agentID, tickID, xPos, yPos, phase)
    String[] items = nextLine.split("\t");
    if (items.length != 7) {
      throw new RuntimeException("Bad line in connectors file: " + nextLine);
    }
    
    //We'll need a double for-loop to generate these
    try {
      //Double-check the nodes.
      Section fromSect = getSection(Integer.parseInt(items[1]));
      Section toSect = getSection(Integer.parseInt(items[2]));
      Node atNode = fromSect.to;
      if (fromSect.to != toSect.from) {
        throw new RuntimeException("Bad lane connector: " + nextLine);
      }
      
      //Get lane generators.
      int fromLaneStart = Integer.parseInt(items[3]);
      int fromLaneEnd = Integer.parseInt(items[4]);
      int toLaneStart = Integer.parseInt(items[5]);
      int toLaneEnd = Integer.parseInt(items[6]);
      
      //Generate all connectors.
      for (int fromLane=fromLaneStart; fromLane<=fromLaneEnd; fromLane++) {
        for (int toLane=toLaneStart; toLane<=toLaneEnd; toLane++) {
          //Create a Connector, populate it, add it to the right section.
          LaneConnector lc = new LaneConnector();
          //lc.id = Integer.parseInt(items[0]); //id isn't used.
          lc.fromSec = fromSect;
          lc.fromLane = fromLane;
          lc.toSec = toSect;
          lc.toLane = toLane;
          lc.arrowMarking = makeArrow(lc);
          
          //Assign it
          atNode.connectors.add(lc);
        }
      }
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
  }
}



boolean myParseBool(String input) {
    if (input.length()!=1) {
      throw new RuntimeException("Bad boolean input: " + input);
    }
    if (input.charAt(0) == 't') {
      return true;
    } else if (input.charAt(0) == 'f') {
      return false;
    }
  throw new RuntimeException("Bad boolean input: " + input);
}







