package sim_mob.vis.network;

import java.awt.Graphics2D;

import edu.umd.cs.piccolo.nodes.PPath;
import edu.umd.cs.piccolo.util.PPaintContext;
import sim_mob.vis.MainFrame;
import sim_mob.vis.network.basic.LocalPoint;

public class Node extends PPath{

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private static final int NODE_SIZE = 5;
	private boolean isUni;   //Rather than having multiple classes....
	private Integer id;

	//Testing
	private LocalPoint localPos;
	
	public LocalPoint getLocalPos(){
		return localPos;
	}
        
	public Node(double x, double y, boolean isUni, Integer id) {
		
		this.localPos = new LocalPoint(x,y);		
		this.isUni = isUni;
		this.id = id;
		
		this.setPathToEllipse(0,0,0,0);
		//this.setPathToEllipse((float)pos.getX(),(float) pos.getY(), NODE_SIZE, NODE_SIZE);
		
//		repaint();
	}
	

	public boolean getIsUni() {
		return isUni;
	}
	public Integer getID(){
		return id;
	}

    
	protected void paint(PPaintContext paintContext){
		
		this.setPathToEllipse((float)localPos.getX(),(float) localPos.getY(), NODE_SIZE, NODE_SIZE);
		
        Graphics2D g = paintContext.getGraphics();
        
        int[] coords = new int[]{(int)localPos.getX()-NODE_SIZE/2, (int)localPos.getY()-NODE_SIZE/2};

        g.setColor(MainFrame.Config.getBackground("node"));

		g.fillOval(coords[0], coords[1], NODE_SIZE, NODE_SIZE);
		
		
		if (isUni) {
			g.setStroke(MainFrame.Config.getLineStroke("uninode"));
			g.setColor(MainFrame.Config.getLineColor("uninode"));
		} else {
			g.setStroke(MainFrame.Config.getLineStroke("multinode"));
			g.setColor(MainFrame.Config.getLineColor("multinode"));
		}
	
		g.drawOval(coords[0], coords[1], NODE_SIZE	, NODE_SIZE);
		//g.setColor(Color.white);
	}	
}
