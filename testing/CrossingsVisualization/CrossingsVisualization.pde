/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

PFont f;
PFont f2;

//Colors
color nodeStroke = color(0xFF, 0x88, 0x22);
color nodeFill = color(0xFF, 0xFF, 0xFF);
color[] crossingColors = new color[] {
  color(0xFF, 0x00, 0x00),
  color(0x00, 0xFF, 0x00),
  color(0x00, 0x00, 0xFF),
};

void checkBounds(double[] bounds, double newVal) {
  bounds[0] = Math.min(bounds[0], newVal);
  bounds[1] = Math.max(bounds[1], newVal);
}

int scalePointForDisplay(double orig, double[] bounds) {
    double percent = (orig - bounds[0]) / (bounds[1] - bounds[0]);
    int scaledMagnitude = ((int)bounds[2] * BUFFER) / 100;
    int newVal = (int)(percent * scaledMagnitude) + ((int)bounds[2]-scaledMagnitude)/2; //Slightly easier to view.
    return newVal;
}

ArrayList<Node> nodes = new ArrayList<Node>();
class Node {
  int id;
  
  double getX() {
    return xPos;
  }
  double getY() {
    return height - yPos;
  }
  
  double xPos;
  double yPos;
  
  boolean isIntersection;
};
Node getNode(int id) {
  for (int i=0; i<nodes.size(); i++) {
    if (nodes.get(i).id == id) {
      return nodes.get(i);
    }
  }
  throw new RuntimeException("No node with id: " + id);
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
  
  Hastable<Integer, ArrayList<Crossing>> crossings = new Hashtable<Integer, ArrayList<Crossing>>();
};
Section getSection(int id) {
  for (int i=0; i<sections.size(); i++) {
    if (sections.get(i).id == id) {
      return sections.get(i);
    }
  }
  throw new RuntimeException("No section with id: " + id);
}


class Crossing {
  int laneID;
  String type;

  double getX() {
    return xPos;
  }
  double getY() {
    return height - yPos;
  }

  double xPos;
  double yPos;

  Section section;
};



void scaleNode(Node n, double[] xBounds, double[] yBounds)  {
  n.xPos = scalePointForDisplay(n.xPos, xBounds);
  n.yPos = scalePointForDisplay(n.yPos, yBounds);
}

void scaleCrossing(Crossing c, double[] xBounds, double[] yBounds)  {
  c.xPos = scalePointForDisplay(n.xPos, xBounds);
  c.yPos = scalePointForDisplay(n.yPos, yBounds);
}



void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  frameRate(4);
    
  //Fonts
  f = createFont("Arial",16,true); 
  f2 = createFont("Arial",12,true); 
  
  //Bounds
  double[] xBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE};
  double[] yBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE};
  
  //Load sample input
  try {
    readNodes("nodes.txt", xBounds, yBounds;
    readSections("sections.txt");
    readCrossings("crossings.txt", xBounds, yBounds;
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
  
  //Scale all points
  for (Node n : nodes) {
    scaleNode(n, xBounds, yBounds);
  }
  for (Section s : sections) {
    for (Crossing c : s.crossings) {
      scaleCrossing(c, xBounds, yBounds);
    }
  }
}


void draw()
{
  smooth();
  background(0xFF);
  
  //Draw all nodes
  strokeWeight(3.0);
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    
    //Draw the node
    stroke(nodeStroke);
    fill(nodeFill);
    ellipse((float)n.getX(), (float)n.getY(), NODE_SIZE, NODE_SIZE);
  }
  
  //Draw all sections
  strokeWeight(2.0);
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);

    //Draw a simple line
    stroke(nodeStroke);
    strokeWeight(2.0);
    line((float)s.from.getX(), (float)s.from.getY(), (float)s.to.getX(), (float)s.to.getY());
    
    //Draw crossings
    for (int crsID=0; crsID<s.crossings.size(); crsID++) {
      ArrayList<Crossing> crs = s.crossings.get(crsID);
      stroke(crossingColors[crsID%crossingColors.length]);
      strokeWeight(1.0);
      for (Crossing cr : crs) {
        point(cr.getX(), cr.getY());
      }
    }
  }
  
  
  //Label nodes
  textFont(f);
  textAlign(CENTER);
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    fill(0x33);
    text(""+(n.id), (float)n.xPos, (float)n.yPos); 
  }
  
  //Label sections
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
  }
  
}



void readNodes(String nodesFile, double[] xBounds, double[] yBounds) throws IOException
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
    
    //Parse: (node_id, x_pos, y_pos)
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
    
    //Expand bounds as necessary
    checkBounds(xBounds, n.xPos);
    checkBounds(yBounds, n.yPos);
    
    nodes.add(n);
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







