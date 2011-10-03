/* Copyright Singapore-MIT Alliance for Research and Technology */

import java.awt.geom.*;

//Super simple visualizer that just prompts you for a shape and then displays it. Also zooms the viewport to match.
float[] minMaxX;
float[] minMaxY;



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
    
  //Track the global zoom stuff
  minMaxX = new float[]{Float.MAX_VALUE, Float.MIN_VALUE};
  minMaxY = new float[]{Float.MAX_VALUE, Float.MIN_VALUE};
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
    return height - (float)scaledY;
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



