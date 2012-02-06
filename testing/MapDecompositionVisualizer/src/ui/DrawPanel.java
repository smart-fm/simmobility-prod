package ui;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import javax.swing.JPanel;
import obj.DrawElement;

public class DrawPanel extends JPanel {

	private final int MAX_COLOR_SIZE = 9;
	private final int CRCLE_RADIUS = 10;
	private final int LINE_WIDTH = 5;
	private final int BORDER_WIDTH = 3;
	private Color[] colors;
	private ArrayList<DrawElement> all_elements = new ArrayList<DrawElement>();
	private double minimum_x;
	private double minimum_y;
	private double maximum_x;
	private double maximum_y;
	
	private int window_width;
	private int window_height;
	
	private double ratio_x;
	private double ratio_y;
	
	public DrawPanel() {
		colors = new Color[MAX_COLOR_SIZE];

		colors[0] = Color.RED;
		colors[1] = Color.YELLOW;
		colors[2] = Color.BLUE;
		colors[3] = Color.CYAN;
		colors[4] = Color.GRAY;
		colors[5] = Color.GREEN;
		colors[6] = Color.MAGENTA;
		colors[7] = Color.ORANGE;
		colors[8] = Color.PINK;
	}

	public void paintComponent(Graphics g) {
		super.paintComponent(g);

		try {
			loadInDataFromFile();
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}
		
		setVisualizerParameters();
		
		Graphics2D g2d = (Graphics2D) g;
		for(int i=0;i<all_elements.size();i++)
		{
			DrawElement one = all_elements.get(i);
			if(one.getPartition_id() < 0)
			{
				drawBorder(g2d, one.getFrom_x(), one.getFrom_y(), one.getTo_x(), one.getTo_y());
			}
			else
			{
				drawLine(g2d, one.getFrom_x(), one.getFrom_y(), one.getTo_x(), one.getTo_y(), one.getPartition_id());
			}
		}
	}

	private void drawBorder(Graphics2D g2d, double from_node_x, double from_node_y, double to_node_x, double to_node_y) {

		float[] dash = { 3f, 0f, 3f };
		BasicStroke stroke = new BasicStroke(BORDER_WIDTH, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND, 1.0f, dash, 2f);
		g2d.setStroke(stroke);

		g2d.setColor(Color.BLACK);
		
		from_node_x = (from_node_x - minimum_x) * ratio_x;
		from_node_y = window_height - (from_node_y - minimum_y) * ratio_y;
		to_node_x = (to_node_x - minimum_x) * ratio_x;
		to_node_y = window_height - (to_node_y - minimum_y) * ratio_y;
		
		g2d.drawLine((int) from_node_x, (int) from_node_y, (int) to_node_x, (int) to_node_y);
	}

	private void drawLine(Graphics2D g2d, double from_node_x, double from_node_y, double to_node_x, double to_node_y, int type) {

		BasicStroke stroke = new BasicStroke(LINE_WIDTH);
		g2d.setStroke(stroke);

		if (type >= MAX_COLOR_SIZE)
			return;

		g2d.setColor(colors[type]);
		
		from_node_x = (from_node_x - minimum_x) * ratio_x;
		from_node_y = window_height - (from_node_y - minimum_y) * ratio_y;
		to_node_x = (to_node_x - minimum_x) * ratio_x;
		to_node_y = window_height - (to_node_y - minimum_y) * ratio_y;
		
		g2d.fillOval((int) from_node_x - CRCLE_RADIUS / 2, (int) from_node_y - CRCLE_RADIUS / 2, CRCLE_RADIUS, CRCLE_RADIUS);
		g2d.fillOval((int) to_node_x - CRCLE_RADIUS / 2, (int) to_node_y - CRCLE_RADIUS / 2, CRCLE_RADIUS, CRCLE_RADIUS);
		
		g2d.drawLine((int) from_node_x, (int) from_node_y, (int) to_node_x, (int) to_node_y);
	}

	private void loadInDataFromFile() throws IOException {
		BufferedReader br = new BufferedReader(new FileReader("partition_result.txt"));
		String sCurrentLine;
		while ((sCurrentLine = br.readLine()) != null) {
			if(sCurrentLine.startsWith("#") || sCurrentLine.equals(""))
				continue;
			
			String[] values = sCurrentLine.split(",");
			DrawElement one = new DrawElement();
			one.setElement_id(Integer.parseInt(values[0]));
			one.setFrom_x(Double.parseDouble(values[1]));
			one.setFrom_y(Double.parseDouble(values[2]));
			one.setTo_x(Double.parseDouble(values[3]));
			one.setTo_y(Double.parseDouble(values[4]));
			one.setPartition_id(Integer.parseInt(values[5]));
			
			all_elements.add(one);
		}
	}
	
	private void setVisualizerParameters()
	{
		window_width = getSize().width;
		window_height = getSize().height;
		
		minimum_x = all_elements.get(0).getFrom_x();
		minimum_y = all_elements.get(0).getFrom_y();
		maximum_x = 0;
		maximum_y = 0;
			
		for(int i=0;i<all_elements.size();i++)
		{
			DrawElement one = all_elements.get(i);
			
			//minimum
			if(one.getFrom_x() < minimum_x)
			{
				minimum_x = one.getFrom_x();
			}
			if(one.getTo_x() < minimum_x)
			{
				minimum_x = one.getTo_x();
			}
			
			if(one.getFrom_y() < minimum_y)
			{
				minimum_y = one.getFrom_y();
			}
			if(one.getTo_y() < minimum_y)
			{
				minimum_y = one.getTo_y();
			}
			
			//maximum
			if(one.getFrom_x() > maximum_x)
			{
				maximum_x = one.getFrom_x();
			}
			if(one.getTo_x() > maximum_x)
			{
				maximum_x = one.getTo_x();
			}
			
			if(one.getFrom_y() > maximum_y)
			{
				maximum_y = one.getFrom_y();
			}
			if(one.getTo_y() > maximum_y)
			{
				maximum_y = one.getTo_y();
			}
		}
		
		ratio_x = 1.0 * window_width / (maximum_x - minimum_x);
		ratio_y = 1.0 * window_height / (maximum_y - minimum_y);
	}
}
