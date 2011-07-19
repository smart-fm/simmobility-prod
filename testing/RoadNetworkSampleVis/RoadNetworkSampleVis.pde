PFont f;
PFont f2;

//Constants
int NODE_SIZE = 16;

//Globally managed max/min on all x/y positions (for scaling)
static double[] xBounds = null;
static double[] yBounds = null;


int scaleX(double orig) {
    double percent = (orig - xBounds[0]) / (xBounds[1] - xBounds[0]);
    int scaledWidth = (width * 90) / 100;
    int newX = (int)(percent * scaledWidth) + (width-scaledWidth)/2; //Slightly easier to view.
    return newX;
}

int scaleY(double orig) {
    double percent = (orig - yBounds[0]) / (yBounds[1] - yBounds[0]);
    int scaledHeight = (height * 90) / 100;
    int newY = (int)(percent * scaledHeight) + (height-scaledHeight)/2; //Slightly easier to view.
    return newY;
}


ArrayList<Node> nodes = new ArrayList<Node>();
class Node {
  int id;
  double xPos;
  double yPos;
  boolean isIntersection;
  
  //Scale
  int getX() {
    return scaleX(this.xPos);
  }
  int getY() {
    return scaleY(this.yPos);
  }
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
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
}


void draw()
{
  smooth();
  background(255);
  
  //Draw all nodes
  textFont(f);
  textAlign(CENTER);
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    int x = n.getX();
    int y = n.getY();
    
    //Draw the circle
    stroke(0x99);
    fill(0x00, 0xCC, 0xCC);
    ellipse(x, y, NODE_SIZE, NODE_SIZE);
    
    //Write its ID
    fill(0x33);
    text(""+(n.id), x, y); 
    
    //Intersection?
    if (n.isIntersection) {
      fill(0x99, 0x00, 0x00);
      text("*I*", x, y+NODE_SIZE);
    }
  }
  
  //Draw all sections
  textFont(f2);
  textAlign(CENTER);
  Set<String> alreadyDrawn = new HashSet<String>();
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);
    
    //Draw the Euclidean line representing this section.
    stroke(0x00, 0x99, 0x00);
    strokeWeight(2.0);
    fill(0x00, 0xCC, 0x00);
    int fromX = s.from.getX();
    int fromY = s.from.getY();
    int toX = s.to.getX();
    int toY = s.to.getY();
    line(fromX, fromY, toX, toY);
    
    //Polyline
    stroke(0x00, 0x00, 0x99);
    strokeWeight(0.5);
    if (s.polyline.size()>2) {
      for (int i=1; i<s.polyline.size(); i++) {
        //Draw from X to Xprev
        DPoint from = s.polyline.get(i-1);
        DPoint to = s.polyline.get(i);
        line(scaleX(from.x), scaleY(from.y), scaleX(to.x), scaleY(to.y)); 
      }
    }
    
    //Write the road name. Only do this once for each section, unless there's a conflict.
    String key = Math.min(s.from.id,s.to.id) + ":" + Math.max(s.from.id,s.to.id) + ":" + s.name; 
    strokeWeight(1);
    if (!alreadyDrawn.contains(key)) {
      fill(0x33);
      text(s.name, fromX+(toX-fromX)/2, fromY+(toY-fromY)/2); 
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
      xBounds = new double[] {n.xPos, n.xPos};
    } else {
      xBounds[0] = Math.min(xBounds[0], n.xPos);
      xBounds[1] = Math.max(xBounds[1], n.xPos);
    }
    if (yBounds == null) {
      yBounds = new double[] {n.yPos, n.yPos};
    } else {
      yBounds[0] = Math.min(yBounds[0], n.yPos);
      yBounds[1] = Math.max(yBounds[1], n.yPos);
    }
    
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
      s.to = getNode(Integer.parseInt(items[6]));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }

    sections.add(s);
  }
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
      double x = Double.parseDouble(items[1]);
      double y = Double.parseDouble(items[2]);
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







