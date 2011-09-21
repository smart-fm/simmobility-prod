/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

//Regexes
String RHS = "\\{([^}]*)\\}"; //NOTE: Contains a capture group
String sep = ", *";
String strn = "\"([^\"]+)\"";
String num = "([0-9]+)";
String numH = "((?:0x)?[0-9a-fA-F]+)";
Pattern logLHS = Pattern.compile("\\(" + strn + sep + num + sep + numH + sep  + RHS + "\\)");
Pattern logRHS = Pattern.compile(strn + ":" + strn + ",?");

//Fonts
PFont f;
PFont f2;

//Turn on/off crossings
boolean paintCrossings = false;

//Turn on/off lanes
boolean paintLanes = true;

//Bit of a painting hack
boolean doRepaint = true;  

//Bit of an action hack
boolean skipAction = false;

//Button background area
float[] btnBackgroundRect;

//Indicator
PImage zoomFitInd;

//Buttons
ToggleButton btnZoomIn;
ToggleButton btnZoomOut;
ToggleButton btnZoomFit;

//Some drawing constants
static final int BUFFER = 95;
static final int NODE_SIZE = 12;
static final int CROSS_POINT_SIZE = 4;

//Current scale matrix.
double[] scaleMatrix;    //CenterX, CenterY, WidthOfWindow_M, HeightOfWindow_M
double[] zoomAmounts;    //Width_M, Height_M to subtract/add on zoom in/out
double[] zoomFitULPoint; //For zoom fit. Upper-left point

//Defaults
static String restrictRoadName = null;
static double[] forceZoomX = null;
static double[] forceZoomY = null;
static int tagLaneID = 0;

//Set
static {
  //For more scaling
  //forceZoomX = new double[]{372455.0595, 372549.8827};
  //forceZoomY = new double[]{143518.2025, 143594.3131};
  //forceZoomX = new double[]{372369.5087, 372455.0595};
  //forceZoomY = new double[]{143594.3131, 143663.9532};
  //forceZoomX = new double[]{372121-150, 372121+150};
  //forceZoomY = new double[]{143107-150, 143107+150};
//  tagLaneID = 4550;
//  tagLaneID = 4215;
  tagLaneID = 839;

  //Restrict which road names have crossings drawn
  //restrictRoadName = "QUEEN STREET";
  //restrictRoadName = "MIDDLE ROAD";
}

//Colors
color taggedLaneColor = color(0xFF, 0x00, 0x00);
color nodeStroke = color(0xFF, 0x88, 0x22);
color nodeFill = color(0xFF, 0xFF, 0xFF);
color csStroke = color(0x00, 0x77, 0x00);
color csFill = color(0xAA, 0xFF, 0xAA);
color[] crossingColors = new color[] {
  color(0x00, 0x00, 0xFF),
  color(0x00, 0xFF, 0xFF),
  color(0x00, 0xFF, 0x00),
  color(0xFF, 0x00, 0xFF),
  color(0xFF, 0xFF, 0x00),
};
Hashtable<String, Integer> laneColors = new Hashtable<String, Integer>();
void populateLaneColorsTable() {
  //Yellow lines on the side of the road. 
  laneColors.put("I", color(0x66, 0x66, 0x00));
  
  //Various internal lane lines
  laneColors.put("F", color(0x33)); 
  laneColors.put("B", color(0x99));
  laneColors.put("C", color(0x99)); 
  laneColors.put("A", color(0x99)); 
  laneColors.put("A2", color(0x99)); 
  
  //Central line for a small side street.
  laneColors.put("E", color(0x33, 0x33, 0xFF)); 

  //Represents zig-zags; used on parts of Queen St. (but that's messed up to begin with).
  laneColors.put("P", color(0x66, 0x66, 0x00)); 
  laneColors.put("K", color(0x66, 0x66, 0x00)); 

  //Bus lines; will be needed later.
  laneColors.put("R", color(0x33, 0xAA, 0x33)); 

  //Stop line; not needed.
  laneColors.put("M", color(0xCC, 0x33, 0xCC)); 
  
  //Rare, seems to represent some kind of temporary side-lane, like a taxi lane. Not needed.
  laneColors.put("D", color(0xCC, 0x33, 0xCC)); 
  
  //Represents intersection box; not needed.
  laneColors.put("N", color(0x66, 0x66, 0x00));
  
  //Represents turning arrows/pockets in intersections; not needed.
  laneColors.put("Q", color(0xAA, 0xAA, 0xAA));
  laneColors.put("T", color(0xAA, 0xAA, 0xAA));
  
  //"Side" lines; seems to trace various miscellaneous points that are (usually) not needed.
  laneColors.put("G", color(0xAA, 0xAA, 0xAA));
  
  //Represents yellow zig-zag on side streets; not needed.
  laneColors.put("O", color(0x66, 0x66, 0x00)); 
  
  //Always seems to follow "I" lines; not needed.
  laneColors.put("A1", color(0x66, 0x66, 0x00));
  laneColors.put("A3", color(0x66, 0x66, 0x00));
  laneColors.put("S1", color(0x99, 0x33, 0x33)); //Represents full-day bus lane.
  laneColors.put("S", color(0x99, 0x33, 0x33)); //Represents bus lane.
  laneColors.put("L", color(0x99, 0x33, 0x33)); //Represents bus lane.
  laneColors.put("H", color(0x99, 0x33, 0x33));  //Only a few; nothing important
  laneColors.put("\\N", color(0x99, 0x33, 0x33)); //Strange circular area, already covered elsewhere.
 
  laneColors.put("Unknown", color(0x00));
}
int getLaneColor(String laneMarking) {
  if (laneColors.containsKey(laneMarking)) {
    return laneColors.get(laneMarking);
  }
  println("Unknown lane marking: " + laneMarking);
  return laneColors.get("Unknown");
}



void checkBounds(double[] bounds, double newVal) {
  bounds[0] = Math.min(bounds[0], newVal);
  bounds[1] = Math.max(bounds[1], newVal);
}

ArrayList<Node> nodes = new ArrayList<Node>();
Hashtable<Integer, ScaledPoint> decoratedNodes = new Hashtable<Integer, ScaledPoint>();
class Node {
  int id;
  
  ScaledPoint pos;
  
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
Hashtable<Integer, MySeg> decoratedSegments = new Hashtable<Integer, MySeg>();
class MySeg {
  int fromNodeID;
  int toNodeID;
  
  MySeg(int fromNode, int toNode) {
    fromNodeID = fromNode;
    toNodeID = toNode;
  }
};
class Section {
  int id;
  String name;
  int numLanes;
  int speed;
  int capacity;

  Node from;
  Node to;
  
  Hashtable<Integer, ArrayList<Crossing>> crossings = new Hashtable<Integer, ArrayList<Crossing>>();
  Hashtable<Integer, ArrayList<Lane>> lanes = new Hashtable<Integer, ArrayList<Lane>>();
};
Section getSection(int id) {
  for (int i=0; i<sections.size(); i++) {
    if (sections.get(i).id == id) {
      return sections.get(i);
    }
  }
  throw new RuntimeException("No section with id: " + id);
}


//Lanes are similar to crossings, but we store them in distinct classes for better
// type-checking. 
class Lane {
  int laneID;
  String laneType;
  String roadName;

  ScaledPoint pos;

  Section section;
};

class Crossing {
  int laneID;
  String laneType;
  String roadName;

  ScaledPoint pos;

  Section section;
};


ArrayList<Circular> circs = new ArrayList<Circular>();
class Circular {
  ScaledPoint pos;
  String label;
  
  Circular(double xOrig, double yOrig, String lbl) {
    pos = new ScaledPoint(xOrig, yOrig);
    label = lbl;
  }
};


ArrayList<CrossShape> crossshapes = new ArrayList<CrossShape>();
class CrossShape {
  ScaledPoint near1;
  ScaledPoint near2;
  ScaledPoint far1;
  ScaledPoint far2;
  
  CrossShape(ScaledPoint near1, ScaledPoint near2, ScaledPoint far1, ScaledPoint far2) {
    this.near1 = near1; 
    this.near2 = near2; 
    this.far1 = far1;
    this.far2 = far2;
  }
};



double[] convertDispToM(double x, double y) {
  double tlX = scaleMatrix[0] - scaleMatrix[2]/2;
  double tlY = scaleMatrix[1] - scaleMatrix[3]/2;
  double normX = x / width;
  double normY = (height-y) / height;
  double newX = normX*scaleMatrix[2] + tlX;
  double newY = normY*scaleMatrix[3] + tlY;
  return new double[]{newX, newY};
}


double[] getScaleRect(double x1, double y1, double x2, double y2) {
  double minX = Math.min(x1, x2);
  double maxX = Math.max(x1, x2);
  double minY = Math.min(y1, y2);
  double maxY = Math.max(y1, y2);
  
  double scaleWidth = maxX-minX;
  double scaleHeight = maxY-minY;
  double centerX = minX + scaleWidth/2;
  double centerY = minY + scaleHeight/2;
  return new double[]{centerX, centerY, scaleWidth, scaleHeight};
}


void scaleAndZoom(double centerX, double centerY, double widthInM, double heightInM) {
  //Save for later.
  scaleMatrix = new double[]{centerX, centerY, widthInM, heightInM};
  
  //Prepare a simpler min-max scaling matrix
  double[] xBounds = new double[]{centerX-widthInM/2, centerX+widthInM/2, width};
  double[] yBounds = new double[]{centerY-heightInM/2, centerY+heightInM/2, height};
  
  //Scale all nodes
  for (Node n : nodes) {
    n.pos.scaleTo(xBounds, yBounds);
  }
  
  //Scale all crossings/lanes (which are saved by section)
  for (Section s : sections) {
    //Crossings
    for (int i : s.crossings.keySet()) {
      ArrayList<Crossing> crossings = s.crossings.get(i);
      for (Crossing c : crossings) {
        c.pos.scaleTo(xBounds, yBounds);
      }
    }
    
    //Lanes
    for (int i : s.lanes.keySet()) {
      ArrayList<Lane> lanes = s.lanes.get(i);
      for (Lane l : lanes) {
        l.pos.scaleTo(xBounds, yBounds);
      }
    }
  }
  
  //Scale all CrossShapes
  for (CrossShape s : crossshapes) {
    s.near1.scaleTo(xBounds, yBounds);
    s.near2.scaleTo(xBounds, yBounds);
    s.far1.scaleTo(xBounds, yBounds);
    s.far2.scaleTo(xBounds, yBounds);
  }
  
  //Scale all circulars
  for (Circular c : circs) {
    c.pos.scaleTo(xBounds, yBounds);
  }

}



void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  frameRate(30);
  populateLaneColorsTable();
  
  //Indicator
  zoomFitInd = loadImage("zoom_indicator.png");
    
  //Fonts
  f = createFont("Arial",16,true); 
  f2 = createFont("Arial",12,true);
  
  //Handle button consistency
  ToggleAction keepButtonsConsistent = new ToggleAction() {
    public void doAction(ToggleButton src) {
      if (skipAction) {
        return;
      }
      skipAction = true;
      
      //Reset zoom fit icon
      zoomFitULPoint = null;
      
      //Two possibilities: 1) This button is now un-pressed; 2) This button is now pressed. Only the second can generate inconsistencies.
      if (src.getIsDown()) {
        //Essentially, just un-press the other buttons
        ToggleButton[] allButtons = new ToggleButton[]{btnZoomOut, btnZoomIn, btnZoomFit};
        for (ToggleButton btn : allButtons) {
          if (btn != src) {
            btn.setIsDown(false);
          }
        }
      }
      
      skipAction = false;
    }
  };
  
  //Buttons
  int btnX = 10;
  int btnY = 10;
  int btnSize = 40;
  int margin = 20;
  btnZoomOut = new ToggleButton(width-btnX-btnSize, btnY, btnSize, "zoom_out.png", "zoom_out_gray.png", keepButtonsConsistent);
  btnX += btnSize + margin;
  btnZoomIn = new ToggleButton(width-btnX-btnSize, btnY, btnSize, "zoom_in.png", "zoom_in_gray.png", keepButtonsConsistent);
  btnX += btnSize + margin;
  btnZoomFit = new ToggleButton(width-btnX-btnSize, btnY, btnSize, "zoom_fit.png", "zoom_fit_gray.png", keepButtonsConsistent);
  
  //Save the buttons' background rectangle.
  btnBackgroundRect = new float[]{width-btnX-btnSize, btnY, btnSize*3+margin*2, btnSize};
  margin = 5;
  btnBackgroundRect[0] -= margin;
  btnBackgroundRect[1] -= margin;
  btnBackgroundRect[2] += 2*margin;
  btnBackgroundRect[3] += 2*margin;
  
  //Bounds
  double[] xBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE, width};
  double[] yBounds = new double[] {Double.MAX_VALUE, Double.MIN_VALUE, height};
  
  //Load sample input
  try {
    readNodes("nodes.txt", xBounds, yBounds);
    readSections("sections.txt");
    readCrossings("crossings.txt", xBounds, yBounds);
    readLanes("lanes.txt", xBounds, yBounds);
    readDecoratedData("runtime_annotations.txt");
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
  
  //Save zoom amounts
  zoomAmounts = new double[]{(xBounds[1]-xBounds[0])/10, (yBounds[1]-yBounds[0])/10};
  
  //Force?
  if (forceZoomX!=null && forceZoomY!=null) {
    xBounds[0] = forceZoomX[0];
    xBounds[1] = forceZoomX[1];
    yBounds[0] = forceZoomY[0];
    yBounds[1] = forceZoomY[1];
  }
  
  //Scale all points
  double[] scaleRect = getScaleRect(xBounds[0], yBounds[0], xBounds[1], yBounds[1]);
  scaleAndZoom(scaleRect[0], scaleRect[1], scaleRect[2], scaleRect[3]);
}


void draw()
{
  //Bit of a hack; we only repaint when something's changed. Should get decent performance, but a double-buffered mapping surface would of course be better.
  if (!doRepaint) {
    return;
  }
  doRepaint = false;
  
  //Turn on smooth-scaling. Paint the background.
  smooth();
  background(0xFF);
  
  //Draw all nodes
  strokeWeight(3.0);
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    
    //Draw the node
    stroke(nodeStroke);
    fill(nodeFill);
    ellipse((float)n.pos.getX(), (float)n.pos.getY(), NODE_SIZE, NODE_SIZE);
  }
  
  //Draw all Cross Shapes
  if (paintCrossings) {
    strokeWeight(2.0);
    stroke(csStroke);
    fill(csFill);
    for (CrossShape cs : crossshapes) {
      beginShape();
      vertex((float)cs.near1.getX(), (float)cs.near1.getY());
      vertex((float)cs.near2.getX(), (float)cs.near2.getY());
      vertex((float)cs.far2.getX(), (float)cs.far2.getY());
      vertex((float)cs.far1.getX(), (float)cs.far1.getY());
      endShape(CLOSE);
    }
  }
  
  //Draw all sections
  strokeWeight(2.0);
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);

    //Draw a simple line
    stroke(nodeStroke);
    strokeWeight(2.0);
    line((float)s.from.pos.getX(), (float)s.from.pos.getY(), (float)s.to.pos.getX(), (float)s.to.pos.getY());
    
    //Draw lanes
    if (paintLanes) {
      for (int keyID : s.lanes.keySet()) {      
        ArrayList<Lane> lanes = s.lanes.get(keyID);
        if (!lanes.isEmpty()) {
          int clr = getLaneColor(lanes.get(0).laneType);
          stroke(clr);
          fill(clr);
          strokeWeight(1.0);
        }
        
        double[] lastPoint = null;
        for (Lane lane : lanes) {
          ellipse((float)lane.pos.getX(), (float)lane.pos.getY(), CROSS_POINT_SIZE, CROSS_POINT_SIZE);
          
          //Connect?
          if (lastPoint!=null) {
            line((float)lane.pos.getX(), (float)lane.pos.getY(), (float)lastPoint[0], (float)lastPoint[1]);
          }
          
          //Save
          lastPoint = new double[] {lane.pos.getX(), lane.pos.getY()};
        }
      }
    }
    
    //Draw crossings
    if (paintCrossings) {
      int crsID=0;
      for (int keyID : s.crossings.keySet()) {      
        ArrayList<Crossing> crs = s.crossings.get(keyID);
        stroke(crossingColors[crsID%crossingColors.length]);
        fill(crossingColors[crsID%crossingColors.length]);
        strokeWeight(1.0);
        
        //Over-rides for "tagged" lane IDs
        if (keyID==tagLaneID) {
          stroke(taggedLaneColor);
          strokeWeight(2.5);
        }
  
        double[] lastPoint = null;
        for (Crossing cr : crs) {
          //Skip?
          if (restrictRoadName!=null && !restrictRoadName.equals(cr.roadName)) {
            continue;
          }
          ellipse((float)cr.pos.getX(), (float)cr.pos.getY(), CROSS_POINT_SIZE, CROSS_POINT_SIZE);
          
          //Connect?
          if (lastPoint!=null) {
            line((float)cr.pos.getX(), (float)cr.pos.getY(), (float)lastPoint[0], (float)lastPoint[1]);
          }
          
          //Save
          lastPoint = new double[] {cr.pos.getX(), cr.pos.getY()};
        }
        crsID++;
      }
    }
  }
  
  
  //Label nodes
  textFont(f);
  textAlign(CENTER);
  for (int i=0; i<nodes.size(); i++) {
    Node n = nodes.get(i);
    fill(0x33);
    text(""+(n.id), (float)n.pos.getX(), (float)n.pos.getY()); 
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
      text(s.name, (float)(s.from.pos.getX()+(s.to.pos.getX()-s.from.pos.getX())/2), (float)(s.from.pos.getY()+(s.to.pos.getY()-s.from.pos.getY())/2)); 
      alreadyDrawn.add(key);
    }
  }
  
  //Label circulars
  if (paintCrossings) {
    for (Circular c : circs) {
      fill(0x33);
      text(c.label, (float)c.pos.getX(), (float)c.pos.getY()); 
    }
  }
  
  //Draw button background
  fill(0x99);
  stroke(0x33);
  strokeWeight(2.5);
  rect(btnBackgroundRect[0], btnBackgroundRect[1], btnBackgroundRect[2], btnBackgroundRect[3]);

  //Draw "zoom fit" indicator
  if (zoomFitULPoint!=null) {
    double screenPosX = zoomFitULPoint[2];
    double screenPosY = zoomFitULPoint[3];
    imageMode(CENTER);
    image(zoomFitInd, (float)screenPosX+12, (float)screenPosY-13); //TODO: Image offsets shouldn't be hard-coded....
  }  

  //Draw buttons
  btnZoomOut.display();
  btnZoomIn.display();
  btnZoomFit.display();  
}


void mousePressed() {
  //Update buttons
  // NOTE: Unless buttons overlap, the short-circuiting behavior doesn't matter.
  boolean btnPressed = btnZoomOut.update() || btnZoomIn.update()|| btnZoomFit.update();
  if (btnPressed) {
    doRepaint = true;
    return;
  }
  
  //Convert screen co-ordinates to meters
  double[] mousePos_M = convertDispToM(mouseX, mouseY);
  
  //If we're clicking on the client area, perform the relevant zoom action
  if (btnZoomIn.getIsDown()) {
    scaleAndZoom(mousePos_M[0], mousePos_M[1], scaleMatrix[2]-zoomAmounts[0], scaleMatrix[3]-zoomAmounts[1]);
    doRepaint = true;
  } else if (btnZoomOut.getIsDown()) {
    scaleAndZoom(scaleMatrix[0], scaleMatrix[1], scaleMatrix[2]+zoomAmounts[0], scaleMatrix[3]+zoomAmounts[1]);
    doRepaint = true;
  } else if (btnZoomFit.getIsDown()) {
    if (zoomFitULPoint==null) {
      zoomFitULPoint = new double[]{mousePos_M[0], mousePos_M[1], mouseX, mouseY};
    } else {
      double[] scaleRect = getScaleRect(zoomFitULPoint[0], zoomFitULPoint[1], mousePos_M[0], mousePos_M[1]);
      scaleAndZoom(scaleRect[0], scaleRect[1], scaleRect[2], scaleRect[3]);
      zoomFitULPoint = null;
      mousePos_M = null;
    }
    doRepaint = true;
  } else if (true) {    //Note: Any time when we don't want this behavior?
    scaleAndZoom(mousePos_M[0], mousePos_M[1], scaleMatrix[2], scaleMatrix[3]);
    doRepaint = true;
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
    double xPos, yPos;
    try {
      n.id = Integer.parseInt(items[0]);
      xPos = Double.parseDouble(items[1]);
      yPos = Double.parseDouble(items[2]);
      n.pos = new ScaledPoint(xPos, yPos);
      n.isIntersection = myParseBool(items[3]);
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
    
    //Expand bounds as necessary
    checkBounds(xBounds, xPos);
    checkBounds(yBounds, yPos);
    
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
    double xPos, yPos;
    try {
      c.laneID = Integer.parseInt(items[0]);
      c.laneType = items[1].trim();
      c.roadName = items[4].trim();
      if (!c.laneType.equals("J") && !c.laneType.equals("A4")) {
        throw new RuntimeException("Unknown crossing laneType: " + c.laneType);
      }
      xPos = Double.parseDouble(items[5]);
      yPos = Double.parseDouble(items[6]);
      c.pos = new ScaledPoint(xPos, yPos);
      
      c.section = getSection(Integer.parseInt(items[3]));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
    
    //Expand bounds as necessary
    checkBounds(xBounds, xPos);
    checkBounds(yBounds, yPos);
    
    //Add it
    if (!c.section.crossings.containsKey(c.laneID)) {
        c.section.crossings.put(c.laneID, new ArrayList<Crossing>());
    }
    c.section.crossings.get(c.laneID).add(c);
  }
}



void readLanes(String lanesFile, double[] xBounds, double[] yBounds) throws IOException { 
  String lines[] = loadStrings(lanesFile);
    
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
    
    //Skip things which were processed in the "crossings" section
    if (items[1].trim().equals("J") || items[1].trim().equals("A4")) {
      continue;
    }
    
    //Create a Lane, populate it.
    Lane ln = new Lane();
    double xPos, yPos;
    try {
      ln.laneID = Integer.parseInt(items[0]);
      ln.laneType = items[1].trim();
      ln.roadName = items[4].trim();
      xPos = Double.parseDouble(items[5]);
      yPos = Double.parseDouble(items[6]);
      ln.pos = new ScaledPoint(xPos, yPos);
      ln.section = getSection(Integer.parseInt(items[3]));
    } catch (Exception ex) {
      throw new RuntimeException(ex);
    }
    
    //Expand bounds as necessary
    checkBounds(xBounds, xPos);
    checkBounds(yBounds, yPos);
    
    //Add it
    if (!ln.section.lanes.containsKey(ln.laneID)) {
        ln.section.lanes.put(ln.laneID, new ArrayList<Lane>());
    }
    ln.section.lanes.get(ln.laneID).add(ln);
  }
}



void readDecoratedData(String path) {
  String lines[] = loadStrings(path);
    
  //Read line-by-line
  for (int lineID=0; lineID<lines.length; lineID++) {
    String nextLine = lines[lineID].trim();
    
    //Skip this line?
    if (nextLine.isEmpty() || !nextLine.startsWith("(") || !nextLine.endsWith(")")) {
      continue;
    }
    
    //"Type"
    Matcher m = logLHS.matcher(nextLine);
    if (!m.matches()) {
      throw new RuntimeException("Invalid line: " + nextLine);
    }
    if (m.groupCount()!=4) {
      throw new RuntimeException("Unexpected group count (" + m.groupCount() + ") for: " + nextLine);
    }
    String type = m.group(1);
    
    //No need to continue?
    if (!type.equals("multi-node") && !type.equals("uni-node") && !type.equals("tmp-circular") && !type.equals("road-segment") && !type.equals("crossing")) {
      continue;
    }
    
    //Known fields
    if (Integer.parseInt(m.group(2))!=0) {
      throw new RuntimeException("Unexpected frame ID, should be zero: " + nextLine);
    }
    int objID = myParseOptionalHex(m.group(3));
    String rhs = m.group(4);
    
    //Json-esque matching
    Hashtable<String, String> properties = new Hashtable<String, String>();
    m = logRHS.matcher(rhs);
    while (m.find()) {
      if (m.groupCount()!=2) {
        throw new RuntimeException("Unexpected group count (" + m.groupCount() + ") for: " + rhs);
      }
      
      String keyStr = m.group(1);
      String value = m.group(2);
      if (properties.containsKey(keyStr)) {
        throw new RuntimeException("Duplicate key: " + keyStr);
      }
      
      properties.put(keyStr, value);
    }
    
    
    //Now, deal with it:
    String[] nodeReqKeys = new String[]{"xPos", "yPos"};
    String[] circReqKeys = new String[]{"at-node", "at-segment", "fwd", "number"};
    String[] segReqKeys = new String[]{"from-node", "to-node"};
    String[] crossReqKeys = new String[]{"near-1", "near-2", "far-1", "far-2"};
    if (type.equals("multi-node") || type.equals("uni-node")) {
      //Check.
      for (String reqKey : nodeReqKeys) {
        if (!properties.containsKey(reqKey)) {
          throw new RuntimeException("Missing key: " + reqKey + " in: " + rhs);
        }
      }
      
      //Retrieve
      double x = Double.parseDouble(properties.get("xPos"));
      double y = Double.parseDouble(properties.get("yPos"));
      
      //Scale
      x /= 100;
      y /= 100;
      
      //Save
      decoratedNodes.put(objID, new ScaledPoint(x, y));
    } else if (type.equals("road-segment")) {
      //Check.
      for (String reqKey : segReqKeys) {
        if (!properties.containsKey(reqKey)) {
          throw new RuntimeException("Missing key: " + reqKey + " in: " + rhs);
        }
      }
      
      //Retrieve
      int fromNodeID = myParseOptionalHex(properties.get("from-node"));
      int toNodeID = myParseOptionalHex(properties.get("to-node"));
      
      //Check
      if (!decoratedNodes.containsKey(fromNodeID)) {
        throw new RuntimeException("Node doesn't exist: " + Integer.toHexString(fromNodeID));
      }
      if (!decoratedNodes.containsKey(toNodeID)) {
        throw new RuntimeException("Node doesn't exist: " + Integer.toHexString(toNodeID));
      }

      //Save      
      decoratedSegments.put(objID, new MySeg(fromNodeID, toNodeID));
    } else if (type.equals("tmp-circular")) {
      //Check.
      for (String reqKey : circReqKeys) {
        if (!properties.containsKey(reqKey)) {
          throw new RuntimeException("Missing key: " + reqKey + " in: " + rhs);
        }
      }
      
      //Retrieve
      int atNodeID = myParseOptionalHex(properties.get("at-node"));
      int atSegmentID = myParseOptionalHex(properties.get("at-segment"));
      boolean isFwd = properties.get("fwd").charAt(0)=='0'?false:true;
      int printNumber = Integer.parseInt(properties.get("number"));
      
      //Check
      if (!decoratedNodes.containsKey(atNodeID)) {
        throw new RuntimeException("Node doesn't exist: " + Integer.toHexString(atNodeID));
      }
      if (!decoratedSegments.containsKey(atSegmentID)) {
        throw new RuntimeException("Segment doesn't exist: " + Integer.toHexString(atSegmentID) );
      }

      //Save
      circs.add(scaleAndMakeCirc(decoratedNodes.get(atNodeID), decoratedSegments.get(atSegmentID), isFwd, printNumber));
    } else if (type.equals("crossing")) {
      //Check.
      for (String reqKey : crossReqKeys) {
        if (!properties.containsKey(reqKey)) {
          throw new RuntimeException("Missing key: " + reqKey + " in: " + rhs);
        }
      }
      
      //Retrieve
      ScaledPoint near1 = myParseScaled(properties.get("near-1"));
      ScaledPoint near2 = myParseScaled(properties.get("near-2"));
      ScaledPoint far1 = myParseScaled(properties.get("far-1"));
      ScaledPoint far2 = myParseScaled(properties.get("far-2"));
      
      //Scale
      near1.origX /= 100;  near1.origY /= 100;
      near2.origX /= 100;  near2.origY /= 100;
      far1.origX /= 100;  far1.origY /= 100;
      far2.origX /= 100;  far2.origY /= 100;

      //Save
      crossshapes.add(new CrossShape(near1, near2, far1, far2));
    }
    
  }
}


ScaledPoint myParseScaled(String combined)  {
  String[] xy = combined.split(",");
  if (xy.length!=2) {
    throw new RuntimeException("Bad Point input: " + combined);
  }
  
  double x = Double.parseDouble(xy[0]);
  double y = Double.parseDouble(xy[1]);
  return new ScaledPoint(x, y);
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

int myParseOptionalHex(String input) {
  int radix = 10;
  if (input.startsWith("0x")) {
    input = input.substring(2);
    radix = 0x10;
  }
  return Integer.parseInt(input, radix);
}



//////


class ScaledPoint {
  double origX;
  double origY;
  double scaledX;
  double scaledY;
  
  ScaledPoint(double x, double y) {
    origX = x;
    origY = y;
  }
  
  double getX() {
    return scaledX;
  }
  double getY() {
    return height - scaledY;
  }
 
  void scaleTo(double[] xBounds, double[] yBounds) {
    scaledX = scalePointForDisplay(origX, xBounds);
    scaledY = scalePointForDisplay(origY, yBounds);
  }

  int scalePointForDisplay(double orig, double[] bounds) {
    double percent = (orig - bounds[0]) / (bounds[1] - bounds[0]);
    int scaledMagnitude = ((int)bounds[2] * BUFFER) / 100;
    int newVal = (int)(percent * scaledMagnitude) + ((int)bounds[2]-scaledMagnitude)/2; //Slightly easier to view.
    return newVal;
  }
  
}



interface ToggleAction {
  public void doAction(ToggleButton src);
}


class ToggleButton 
{
  int x, y, w, h;
  
  boolean isDown;
  PImage up;
  PImage down;
  PImage currentimage;
  ToggleAction onButtonDown;

  ToggleButton(int ix, int iy, int isz, String downPath, String upPath, ToggleAction onButtonDown_i) 
  {
    x = ix;
    y = iy;
    w = isz;
    h = isz;
    up = loadImage(upPath);
    down = loadImage(downPath);
    onButtonDown = onButtonDown_i;
    
    setIsDown(false);
  }
  
  void setIsDown(boolean value) {
    isDown = value;
    currentimage = isDown?down:up;
  }
  
  boolean getIsDown() {
    return isDown;
  }
  
  boolean update() 
  {
    //Should we react
    if(overRect(x, y, w, h)) {
      //Button down/up state change
      setIsDown(!isDown);
        
      //Allow the user to react
      if (onButtonDown!=null) {
        onButtonDown.doAction(this);
      }
      return true;
    }
    return false;
  }
    
  void display() 
  {
    imageMode(CORNER);
    image(currentimage, x, y);
  }
  
  boolean overRect(int x, int y, int width, int height) {
  if (mouseX >= x && mouseX <= x+width && mouseY >= y && mouseY <= y+height) {
      return true;
    } else {
      return false;
    }
  }
}


//Turn our circulars into simple-to-display labels.
Circular scaleAndMakeCirc(ScaledPoint from, MySeg seg, boolean isFwd, int printNumber) {
  //Constants
  double DIST_FROM_SRC = 3.0; //In Meters
  double DIST_FROM_LINE = 0.2; //In Meters
  
  //First, get the "other" point.
  ScaledPoint to = isFwd ? decoratedNodes.get(seg.fromNodeID) : decoratedNodes.get(seg.toNodeID);
  
  //Now create a vector between them, make a unit vector, and scale it to a certain distance.
  Vec ray = new Vec(from.origX, from.origY, to.origX, to.origY);
  ray.makeUnit();
  ray.scaleVect(DIST_FROM_SRC);

  //Now, get a vector normal to this one (the direction depends on "isFwd")
  ray.translateVect(ray);
  ray.flipVecNormal(isFwd);
  ray.scaleVect(DIST_FROM_LINE); 

//  String sp = printNumber%2==0?"          ":"";
  String sp = "";
  return new Circular(ray.getEndX(), ray.getEndY(), sp+printNumber+(isFwd?"F":"R"));
}


//Vector-related classes/methods
class Vec {
  double posX;
  double posY;
  double magX;
  double magY;
  Vec(double fromX, double fromY, double toX, double toY) {
    this.posX = fromX;
    this.posY = fromY;
    this.magX = toX-fromX;
    this.magY = toY-fromY;
  }
  
  Vec(double magX, double magY) {
    this(0, 0, magX, magY);
  }
  
  Vec(Vec other) {
    this(other.posX, other.posY, other.magX, other.magY);
  }
  
  double getEndX() {
    return posX + magX;
  }
  
  double getEndY() {
    return posY + magY;
  }
  
  double getMagnitude() {
    return Math.sqrt(magX*magX + magY*magY);
  }
  
  void makeUnit()  {
    scaleVect(1/getMagnitude());
  }
  
  void scaleVect(double val) {
    magX *= val;
    magY *= val;
  }

  void translateVect(double dX, double dY) {
    posX += dX;
    posY += dY;
  }
  
  void translateVect(Vec magnitude) {
    translateVect(magnitude.magX, magnitude.magY);
  }
  
  void flipVecNormal(boolean direction) {
    int sign = direction ? 1 : -1;
    double newX = -magY*sign;
    double newY = magX*sign;
    magX = newX;
    magY = newY;
  }
}












