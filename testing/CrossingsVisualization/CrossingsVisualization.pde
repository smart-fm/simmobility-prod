/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

PFont f;
PFont f2;

//Some drawing constants
static final int BUFFER = 95;
static final int NODE_SIZE = 10;

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
  
  Hashtable<Integer, ArrayList<Crossing>> crossings = new Hashtable<Integer, ArrayList<Crossing>>();
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
  String laneType;

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
  c.xPos = scalePointForDisplay(c.xPos, xBounds);
  c.yPos = scalePointForDisplay(c.yPos, yBounds);
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
  double[] xBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE, width};
  double[] yBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE, height};
  
  //Load sample input
  try {
    readNodes("nodes.txt", xBounds, yBounds);
    readSections("sections.txt");
    readCrossings("crossings.txt", xBounds, yBounds);
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
  
  //Scale all points
  for (Node n : nodes) {
    scaleNode(n, xBounds, yBounds);
  }
  for (Section s : sections) {
    for (int i : s.crossings.keySet()) {
      ArrayList<Crossing> crossings = s.crossings.get(i);
      for (Crossing c : crossings) {
        scaleCrossing(c, xBounds, yBounds);
      }
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
    int crsID=0;
    for (int keyID : s.crossings.keySet()) {
      ArrayList<Crossing> crs = s.crossings.get(keyID);
      stroke(crossingColors[crsID%crossingColors.length]);
      strokeWeight(1.0);
      for (Crossing cr : crs) {
        point((float)cr.getX(), (float)cr.getY());
      }
      crsID++;
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
      fill(0x33);
      text(s.name, (float)(s.from.xPos+(s.to.xPos-s.from.xPos)/2), (float)(s.from.yPos+(s.to.yPos-s.from.yPos)/2)); 
      alreadyDrawn.add(key);
    }
  }
  
}



void readNodes(String nodesFile, double[] xBounds, double[] yBounds) throws IOException { 
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



void readSections(String sectionsFile) throws IOException { 
  String lines[] = loadStrings(sectionsFile);
  sections.clear();
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (id, name, numLanes, speed, capactity, from, to, length)
    String[] items = nextLine.split("\t");
    if (items.length != 8) {
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
      s.to = getNode(Integer.parseInt(items[6]));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }

    sections.add(s);
  }
}


void readCrossings(String crossingsFile, double[] xBounds, double[] yBounds) throws IOException { 
  String lines[] = loadStrings(crossingsFile);
  nodes.clear();
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.startsWith("#") || nextLine.isEmpty()) {
      continue;
    }
    
    //Parse: (lane_id, lane_type, lane_type_desc, section, road_name, xpos, ypos)
    String[] items = nextLine.split("\t");
    if (items.length != 7) {
      throw new RuntimeException("Bad line in crossings file: " + nextLine);
    }
    
    //Create a Crossing, populate it.
    Crossing c = new Crossing();
    try {
      c.laneID = Integer.parseInt(items[0]);
      c.laneType = items[1].trim();
      if (!c.laneType.equals("J") && !c.laneType.equals("A4")) {
        throw new RuntimeException("Unknown crossing laneType: " + c.laneType);
      }
      c.xPos = Double.parseDouble(items[5]);
      c.yPos = Double.parseDouble(items[6]);
      
      c.section = getSection(Integer.parseInt(items[3]));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
    
    //Expand bounds as necessary
    checkBounds(xBounds, c.xPos);
    checkBounds(yBounds, c.yPos);
    
    //Add it
    if (!c.section.crossings.containsKey(c.laneID)) {
        c.section.crossings.put(c.laneID, new ArrayList<Crossing>());
    }
    c.section.crossings.get(c.laneID).add(c);
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


