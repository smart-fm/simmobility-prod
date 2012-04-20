package sim_mob.vis.simultion;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.Path2D;

import sim_mob.vis.network.Node;
import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;

public class VehicleTrafficSignal extends PPath{
	
	private Node fromNode;
	private Node toNode;
	
	private final int ARR_SIZE = 6; 
	
	public Node getFromNode (){return fromNode;}
	public Node getToNode (){return toNode;}
	
	float [] x  = new float[]{0,0,0,0};
	float [] y  = new float[]{0,0,0,0};
	
	public VehicleTrafficSignal(Node fromNode,Node toNode){
		this.fromNode = fromNode;
		this.toNode = toNode;
		
		this.createPolyline(x, y);
	}
	public void paint(PPaintContext paintContext){

		Graphics2D g = paintContext.getGraphics();
		double x1 = fromNode.getX();
		double y1 = fromNode.getY();

		double x2 =	toNode.getX();
		double y2 = toNode.getY();
		
		AffineTransform oldAt = g.getTransform();

        double dx = x2 - x1, dy = y2 - y1;
        double angle = Math.atan2(dy, dx);
        int len = (int) Math.sqrt(dx*dx + dy*dy);
        
        
        this.getTransform();
		AffineTransform at = AffineTransform.getTranslateInstance(x1, y1);
		at.concatenate(AffineTransform.getRotateInstance(angle));
        g.setTransform(at);

        // Draw horizontal arrow starting in (0, 0)
        g.setColor(Color.RED);
        g.drawLine(0, 0, (int) len, 0);
        x = new float[]{len, len-ARR_SIZE, len-ARR_SIZE, len};
        y = new float[]{0, -ARR_SIZE, ARR_SIZE, 0};
        
        this.setPaint(Color.RED);
        this.setPathToPolyline(x,y);

       
       // g.fillPolygon(new int[] {len, len-ARR_SIZE, len-ARR_SIZE, len},
         //             new int[] {0, -ARR_SIZE, ARR_SIZE, 0}, 4);
        
        //Restore
        g.setTransform(oldAt);
		

	}
	
	

}
