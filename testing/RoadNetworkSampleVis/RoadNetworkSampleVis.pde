import java.awt.geom.*;

PFont f;
PFont f2;

//Constants
int BUFFER = 95;
int NODE_SIZE = 16;

//Flag; make into a checkbox later
boolean drawDetailedIntersections = true;

//Node zoom level; make into a set of sliders later.
int NODE_ZOOM = 64;
int NODE_ZOOM_INNER = 24;

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
  Hashtable<Section, LaneInfo> edgePoints = new Hashtable<Section, LaneInfo>(); 
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


class LaneConnector {
  //Logical data
  Section fromSec;
  int fromLane;
  Section toSec;
  int toLane;
  
  //Data for drawing this connector.
  
};



void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  frameRate(1);
  
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
  background(255);
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
    stroke(0x00, 0x99, 0x00);
    strokeWeight(2.0);
    fill(0x00, 0xCC, 0x00);
    line((float)s.from.xPos, (float)s.from.yPos, (float)s.to.xPos, (float)s.to.yPos);
    
    //Polyline
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
    
    //Not drawing speed limit or capacity for now...
  }
  
  
  //Draw "detailed" nodes on top of road lines
  if (drawDetailedIntersections) {
    for (int i=0; i<nodes.size(); i++) {
      Node n = nodes.get(i);
    
      //Draw the circle
      strokeWeight(1.5);
      fill(0x00);
      if (n.isIntersection) {
        stroke(0xFF, 0x00, 0x00);
      } else {
        stroke(0x00);
      }
      ellipse((float)n.bounds.getCenterX(), (float)n.bounds.getCenterY(), (int)n.bounds.getWidth(), (int)n.bounds.getHeight());
      fill(0x33);
      stroke(0x33);
      ellipse((float)n.inner.getCenterX(), (float)n.inner.getCenterY(), (int)n.inner.getWidth(), (int)n.inner.getHeight());
      
      //Draw lines to all "edge" points.
      strokeWeight(1.0);
      stroke(0xFF, 0xFF, 0x00);
      for (LaneInfo edge : n.edgePoints.values()) {
        Line2D med = edge.medianLine;
        line((float)med.getX1(), (float)med.getY1(), (float)med.getX2(), (float)med.getY2());
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
      fill(0x33);
      text(s.name, (float)(s.from.xPos+(s.to.xPos-s.from.xPos)/2), (float)(s.from.yPos+(s.to.yPos-s.from.yPos)/2)); 
      alreadyDrawn.add(key);
    }
    
    //Not drawing speed limit or capacity for now...
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
  DPoint vect = new DPoint(outer.xPos-cent.xPos, outer.yPos-cent.yPos);
  makeUnit(vect);
  scaleVect(vect, cent.bounds.getWidth()/2);
  res.medianLine = new Line2D.Double(cent.xPos, cent.yPos, cent.xPos+vect.x, cent.yPos+vect.y);
  
  //Now figure out the rest of its lane lines.
  
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
    if (!s.from.edgePoints.containsKey(s)) {
      s.from.edgePoints.put(s, getEdgePoint(s.from, s.to));
    }
    if (!s.to.edgePoints.containsKey(s)) {
      s.to.edgePoints.put(s, getEdgePoint(s.to, s.from));
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
      for (int fromLane=fromLaneStart; fromLane!=fromLaneEnd; fromLane++) {
        for (int toLane=toLaneStart; toLane!=toLaneEnd; toLane++) {
          //Create a Connector, populate it, add it to the right section.
          LaneConnector lc = new LaneConnector();
          //lc.id = Integer.parseInt(items[0]); //id isn't used.
          lc.fromSec = fromSect;
          lc.fromLane = fromLane;
          lc.toSec = toSect;
          lc.toLane = toLane;
          
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







