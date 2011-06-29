//Imports
import java.io.*;


int NUM_AGENTS = 11;
int AGENT_RADIUS = 10;
int currTick = 0;

PFont f, f2;
ArrayList<ArrayList<double[]>> ticks =  new ArrayList<ArrayList<double[]>>();

float[][] colors = {
  {0xDC, 0x14, 0x3C},
  {0xFF, 0x8C, 0x00},
  {0xF0, 0xE6, 0x8C},
  {0xFF, 0x69, 0xB4},
  {0x98, 0xFB, 0x98},
  {0x6B, 0x8E, 0x23},
  {0xAF, 0xEE, 0xEE},
  {0x64, 0x95, 0xED},
  {0x6A, 0x5A, 0xCD},
  {0xC0, 0xC0, 0xC0},  
  {0xA0, 0x52, 0x2D}
};


void setup() 
{
  //Windows are always 1000x400
  size(1000, 400);
  frameRate(3);
  
  //Font
  f = createFont("Arial",18,true); 
  f2 = createFont("Arial",72,true); 

  //Load sample input
  try {
    readInput("simple_input.txt");
  } catch (IOException ex) {
    throw new RuntimeException(ex);
  }
}


void draw()
{
  smooth();
  background(255);
  
  //The current tick ID
  textFont(f);
  textAlign(LEFT);
  fill(0x00);
  text(("Tick: " + currTick) ,10, 30); 
  
  //Each agent for the current tick
  for (int agentID=0; agentID<ticks.get(currTick).size(); agentID++) {
    //Retrieve position
    float xPos = (float)ticks.get(currTick).get(agentID)[0];
    float yPos = (float)ticks.get(currTick).get(agentID)[1];
    float[] agColor = colors[agentID];
    
    //Draw this agent
    stroke(0x00);
    fill(agColor[0], agColor[1], agColor[2]);
    ellipse(xPos, yPos, 2*AGENT_RADIUS, 2*AGENT_RADIUS);
  }
  

  //Done?
  if (currTick >= ticks.size()-1) {
    textFont(f2);
    textAlign(CENTER);
    fill(0x66, 0x00, 0x00);
    text("Done",width/2,height/2); 
  } else {
    currTick++;
  }
}


void readInput(String inFileName) throws IOException
{ 
  BufferedReader br = new BufferedReader(new FileReader(dataPath(inFileName)));
  boolean skip = true;
    
  //Read line-by-line
  String nextLine;
  while ((nextLine = br.readLine()) != null) {
    nextLine = nextLine.trim();
    
    //Skip this line?
    if (skip) {
      if (nextLine.equals("(Sanity Check Passed)")) {
        skip = false;
      }
      continue;
    }
    
    //Other reasons to skip
    if (!(nextLine.startsWith("(") && nextLine.endsWith(")"))) {
      continue;
    }
    if (nextLine.startsWith("(Warmup") || nextLine.startsWith("(Agent")) {
      continue;
    }
    
    //Parse: (agentID, tickID, xPos, yPos, phase)
    String[] points = nextLine.substring(1, nextLine.length()-1).split(",");
    if (points.length != 5) {
      throw new RuntimeException("Bad line: " + nextLine);
    }
    
    //Manage ticks, agents
    int agentID = Integer.parseInt(points[0]);
    int tickID = Integer.parseInt(points[1]);
    while (ticks.size() <= tickID) {
      ticks.add(new ArrayList<double[]>());
      for (int i=0; i<NUM_AGENTS; i++) {
        ticks.get(ticks.size()-1).add(new double[2]);
      }
    }
    if (agentID<0 || agentID>=NUM_AGENTS) {
     throw new RuntimeException("Bad agentID: " + agentID);
   }
      
    //Set X/Y
    double xPos = Double.parseDouble(points[2]);
    double yPos = Double.parseDouble(points[3]);
    ticks.get(tickID).get(agentID)[0] = xPos;
    ticks.get(tickID).get(agentID)[1] = yPos;
  }
    
  br.close();
}









