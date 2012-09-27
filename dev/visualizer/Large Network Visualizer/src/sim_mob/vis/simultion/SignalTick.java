package sim_mob.vis.simultion;

/**
 * For now, Traffic Signals are simply added to the top-level container, and are all drawn in a row.
 * 
 *  \author Seth N. Hetu
 *  \author Zhang Shuai
 */
/*public class SignalTick implements DrawableItem {
	private DPoint visPos;
	
	private static final int SIG_LARGE_SZ = 32;
	private static final int SIG_SMALL_SZ = 16;
	private static final int SIG_BUFFER = 4;
	
	//Signal states: for vehicles and pedestrians
	private Color[] vehicleLights;
	private Color[] pedestrianLights;
	
	
	public SignalTick(double xVis, double yVis, Color[] vehicleLights, Color[] pedestrianLights) {
	
		visPos = new DPoint(xVis, yVis);
		this.vehicleLights = vehicleLights;
		this.pedestrianLights = pedestrianLights;
		
		//Check
		if (vehicleLights.length!=4 || pedestrianLights.length!=4) {
			throw new RuntimeException("Signal created without 4 associated Links.");
		}
	}
	
	public static int EstVisualSize() {
		return SIG_LARGE_SZ*3 + SIG_BUFFER*4 + 2*2;
	}
	
	public void draw(Graphics2D g) {
		//Save old transformation.
		AffineTransform oldAT = g.getTransform();
		
		//Translate
		AffineTransform at = AffineTransform.getTranslateInstance(visPos.x, visPos.y);
		g.setTransform(at);
		
		//Draw: background rectangle
		int SQ_SIZE = EstVisualSize();
		g.setStroke(new BasicStroke(2.0F));
		g.setColor(Color.lightGray);
		g.fillRect(0, 0, SQ_SIZE, SQ_SIZE);
		g.setColor(Color.darkGray);
		g.drawRect(0, 0, SQ_SIZE, SQ_SIZE);
		
		//Draw each signal
		//NOTE: I'm sure these are not necessarily in the correct place.
		for (int i=0; i<vehicleLights.length; i++) {
			//Get the correct position
			//NOTE: This is very messy, but it's intended to be temporary code.
			int x = 0;
			int y = 0;
			if (i==0) {
				x = 2 + SIG_BUFFER;
			} else if (i==3) {
				x = SQ_SIZE - 2 - SIG_BUFFER - SIG_LARGE_SZ;
			} else {
				x = SQ_SIZE/2 - SIG_LARGE_SZ/2;
			}
			if (i==1) {
				y = 2 + SIG_BUFFER;
			} else if (i==2) {
				y = SQ_SIZE - 2 - SIG_BUFFER - SIG_LARGE_SZ;
			} else {
				y = SQ_SIZE/2 - SIG_LARGE_SZ/2;
			}
			
			//Draw the main light
			g.setColor(vehicleLights[i]);
			g.fillOval(x, y, SIG_LARGE_SZ, SIG_LARGE_SZ);
			g.setColor(Color.darkGray);
			g.setStroke(new BasicStroke(1.0F));
			g.drawOval(x, y, SIG_LARGE_SZ, SIG_LARGE_SZ);
			
			//The small light is always to the "right" of the main light
			if (i==0) {
				y+=SIG_LARGE_SZ;
			} else if (i==1) {
				x-=SIG_SMALL_SZ;
			} else if (i==2) {
				x+=SIG_LARGE_SZ;
				y+=(SIG_LARGE_SZ-SIG_SMALL_SZ);
			} else {
				x+=(SIG_LARGE_SZ-SIG_SMALL_SZ);
				y-=SIG_SMALL_SZ;
			}
			
			//Draw the smaller light
			g.setColor(pedestrianLights[i]);
			g.fillOval(x, y, SIG_SMALL_SZ, SIG_SMALL_SZ);
			g.setColor(Color.darkGray);
			g.setStroke(new BasicStroke(1.0F));
			g.drawOval(x, y, SIG_SMALL_SZ, SIG_SMALL_SZ);
		}
		
		
		//Restore old transformation matrix
		g.setTransform(oldAT);
		
	}
}*/

