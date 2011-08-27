/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

//Fonts
PFont f;
PFont f2;

//Bit of a painting hack
boolean doRepaint = true;

//Bit of an action hack
boolean skipAction = false;

//Button background area
float[] btnBackgroundRect;

//Buttons
ToggleButton btnZoomIn;
ToggleButton btnZoomOut;
ToggleButton btnZoomFit;

//Some drawing constants
static final int BUFFER = 95;
static final int NODE_SIZE = 12;
static final int CROSS_POINT_SIZE = 4;

//Current scale matrix.
double[] scaleMatrix;   //CenterX, CenterY, WidthOfWindow_M, HeightOfWindow_M
double[] zoomAmounts;   //Width_M, Height_M to subtract/add on zoom in/out

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
  tagLaneID = 4235;

  //Restrict which road names have crossings drawn
  //restrictRoadName = "QUEEN STREET";
  //restrictRoadName = "MIDDLE ROAD";
}

//Colors
color taggedLaneColor = color(0xFF, 0x00, 0x00);
color nodeStroke = color(0xFF, 0x88, 0x22);
color nodeFill = color(0xFF, 0xFF, 0xFF);
color[] crossingColors = new color[] {
  color(0x00, 0x00, 0xFF),
  color(0x00, 0xFF, 0xFF),
  color(0x00, 0xFF, 0x00),
  color(0xFF, 0x00, 0xFF),
  color(0xFF, 0xFF, 0x00),
};

void checkBounds(double[] bounds, double newVal) {
  bounds[0] = Math.min(bounds[0], newVal);
  bounds[1] = Math.max(bounds[1], newVal);
}

ArrayList<Node> nodes = new ArrayList<Node>();
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
  String roadName;

  ScaledPoint pos;

  Section section;
};



void scaleAndZoom(double centerX, double centerY, double widthInM, double heightInM) {
  //Save for later.
  scaleMatrix = new double[]{centerX, centerY, widthInM, heightInM};
  
  //Prepare a simpler min-max scaling matrix
  double[] xBounds = new double[]{centerX-widthInM, centerX+widthInM, width};
  double[] yBounds = new double[]{centerY-heightInM, centerY+heightInM, height};
  
  //Scale all nodes
  for (Node n : nodes) {
    n.pos.scaleTo(xBounds, yBounds);
  }
  
  //Scale all crossings (which are saved by section)
  for (Section s : sections) {
    for (int i : s.crossings.keySet()) {
      ArrayList<Crossing> crossings = s.crossings.get(i);
      for (Crossing c : crossings) {
        c.pos.scaleTo(xBounds, yBounds);
      }
    }
  }
  
  //Repaint
  doRepaint = true;
}



void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  frameRate(30);
    
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
  double scaleWidth = xBounds[1]-xBounds[0];
  double scaleHeight = yBounds[1]-yBounds[0];
  double centerX = xBounds[0] + scaleWidth/2;
  double centerY = yBounds[0] + scaleHeight/2;
  scaleAndZoom(centerX, centerY, scaleWidth, scaleHeight);
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
  
  //Draw all sections
  strokeWeight(2.0);
  for (int secID=0; secID<sections.size(); secID++) {
    Section s = sections.get(secID);

    //Draw a simple line
    stroke(nodeStroke);
    strokeWeight(2.0);
    line((float)s.from.pos.getX(), (float)s.from.pos.getY(), (float)s.to.pos.getX(), (float)s.to.pos.getY());
    
    //Draw crossings
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
  
  //Draw button background
  fill(0x99);
  stroke(0x33);
  strokeWeight(2.5);
  rect(btnBackgroundRect[0], btnBackgroundRect[1], btnBackgroundRect[2], btnBackgroundRect[3]);
  

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
  
  //If we're clicking on the client area, perform the relevant zoom action
  if (btnZoomIn.getIsDown()) {
    println("Zoom in");
    doRepaint = true;
  } else if (btnZoomOut.getIsDown()) {
    println("Zoom out");
    doRepaint = true;
  } else if (btnZoomFit.getIsDown()) {
    println("Zoom fit");
    doRepaint = true;
  } else if (true) {    //Note: Any time when we don't want this behavior?
    println("Translate canvas");
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



