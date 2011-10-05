/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

//Super simple visualizer that just prompts you for a shape and then displays it. Also zooms the viewport to match.
float[] minMaxX;
float[] minMaxY;

int BAR_HEIGHT = 50;
int selectedItem = 0;
String[] currStrs = {"", "", ""};
PFont strFont;


//Everything we draw is called an "Item"
class Item {
  ScaledPoint start;
  ScaledPoint end;
  int lineColor;
  float lineThickness;
};
ArrayList<Item> items = new ArrayList<Item>();



void setup() 
{
  //Windows are always 800 X 600
  size(800, 600);
  
  strFont = createFont("Trebuchet MS", 16, true);
    
  //Track the global zoom stuff
  minMaxX = new float[]{Float.MAX_VALUE, Float.MIN_VALUE, width};
  minMaxY = new float[]{Float.MAX_VALUE, Float.MIN_VALUE, height-BAR_HEIGHT};
}


void draw()
{
  //Turn on smooth-scaling. Paint the background.
  smooth();
  background(0xFF);
  
  //Draw all existing shapes.
  for (Item item : items) {
    stroke(item.lineColor);
    strokeWeight(item.lineThickness);
    noFill();
    
    //The "start" always has a small X.
    int buff = 3;
    line(item.start.getX()-buff, item.start.getY(), item.start.getX()+buff, item.start.getY());
    line(item.start.getX(), item.start.getY()-buff, item.start.getX(), item.start.getY()+buff);
    
    //Now draw the line
    line(item.start.getX(), item.start.getY(), item.end.getX(), item.end.getY());
  }
  
  //Draw our status bar
  int BAR_BUFFER = 2;
  int oneThird = width/3;
  fill(0x00);
  noStroke();
  rect(0, 0, width, BAR_HEIGHT);
  
  for (int i=0; i<3; i++) {
    if (selectedItem==i) {
      fill(0xFF, 0xDD, 0xDD);
    } else {
      fill(0xFF);
    }
    rect(i*oneThird+BAR_BUFFER, BAR_BUFFER, oneThird-BAR_BUFFER, BAR_HEIGHT-2*BAR_BUFFER);
    fill(0x00);
    textAlign(LEFT, BASELINE);
    textFont(strFont);
    text(currStrs[i], i*oneThird+BAR_BUFFER+BAR_BUFFER, BAR_BUFFER+BAR_HEIGHT/2+textDescent());
  }
}



void addItem() {
  //Read start(x,y), end(x,y), stroke
  Item newItem = new Item();
  for (int i=0; i<2; i++) {
    String[] pos = currStrs[i].split(",");
    if (pos.length!=2) { return; }
    try {
      float x = Float.parseFloat(pos[0]);
      float y = Float.parseFloat(pos[1]);
      if (i==0) {
        newItem.start = new ScaledPoint(x, y);
      } else {
        newItem.end = new ScaledPoint(x, y);
      }
    } catch (Exception ex) { return; }
  }
  try {
    float strokeW = Float.parseFloat(currStrs[2]);
    newItem.lineThickness = strokeW;
  } catch (Exception ex) { return; }
  newItem.lineColor = color(0x00, 0x66, 0x00);
  
  //Add the item. Either scale it, or scale all points.
  items.add(newItem);
  println("Point added");
  updateMinMaxBoundsAndScale(newItem);
  currStrs = new String[]{"", "", ""};
  selectedItem = 0;
}



void keyPressed() {
  if (key==TAB) {
    selectedItem = (selectedItem+1)%3;
  } else if (key==ENTER) {
    addItem();
  } else if (key==BACKSPACE) {
    if (currStrs[selectedItem].length()>0) {
      currStrs[selectedItem] = currStrs[selectedItem].substring(0, currStrs[selectedItem].length()-1);
    }
  } else if (key==DELETE) {
    currStrs[selectedItem] = "";
  } else if (key==CODED) {
    //???
  } else {
    currStrs[selectedItem] += key;
  }
}



void updateMinMaxBoundsAndScale(Item newItem) {
  float[] oldValX = new float[]{minMaxX[0], minMaxX[1]};
  float[] oldValY = new float[]{minMaxY[0], minMaxY[1]};
  minMaxX[0] = Math.min(minMaxX[0], newItem.start.getOrigX());
  minMaxX[0] = Math.min(minMaxX[0], newItem.end.getOrigX());
  minMaxX[1] = Math.max(minMaxX[1], newItem.start.getOrigX());
  minMaxX[1] = Math.max(minMaxX[1], newItem.end.getOrigX());
  
  minMaxY[0] = Math.min(minMaxY[0], newItem.start.getOrigY());
  minMaxY[0] = Math.min(minMaxY[0], newItem.end.getOrigY());
  minMaxY[1] = Math.max(minMaxY[1], newItem.start.getOrigY());
  minMaxY[1] = Math.max(minMaxY[1], newItem.end.getOrigY());
  

  if (oldValX[0]!=minMaxX[0] || oldValX[1]!=minMaxX[1] || oldValY[0]!=minMaxY[0] || oldValY[1]!=minMaxY[1]) {
    println("All points scaled");
    scaleAllPoints();
  } else {
    println("Point scaled");
    scaleAPoint(newItem.start);
    scaleAPoint(newItem.end);
  }
}


void scaleAllPoints() {
  for (Item item : items) {
    scaleAPoint(item.start);
    scaleAPoint(item.end);
  }
}

void scaleAPoint(ScaledPoint p) {
  p.scaleTo(minMaxX, minMaxY);
}



class ScaledPoint {
  double origX;
  double origY;
  double scaledX;
  double scaledY;
  
  final int BUFFER = 95;
  
  ScaledPoint(double x, double y) {
    origX = x;
    origY = y;
  }
  
  float getX() {
    return (float)scaledX;
  }
  float getY() {
    return (height - BAR_HEIGHT - (float)scaledY) + BAR_HEIGHT;
  }
  
  float getOrigX() {
    return (float)origX;
  }
  float getOrigY() {
    return (float)origY;
  }
 
  void scaleTo(float[] xBounds, float[] yBounds) {
    scaledX = scalePointForDisplay((float)origX, xBounds);
    scaledY = scalePointForDisplay((float)origY, yBounds);
  }

  int scalePointForDisplay(float orig, float[] bounds) {
    double percent = (orig - bounds[0]) / (bounds[1] - bounds[0]);
    int scaledMagnitude = ((int)bounds[2] * BUFFER) / 100;
    int newVal = (int)(percent * scaledMagnitude) + ((int)bounds[2]-scaledMagnitude)/2; //Slightly easier to view.
    return newVal;
  }
  
}



