import java.awt.geom.Point2D;

import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;


/**
 * The purpose of this class is to allow spatial indexing on graphical objects using their 
 * bounding boxes while being as simple to understand as possible.
 *  
 * Some of its key qualities are:
 *   1) Able to search within a rectangle for anything that matches.
 *   2) Good performance on insertion/removal (fast for dynamic entities like Agents).
 *   3) Good performance for retrieval (fast for searching entities).
 *   4) Simple to grasp; easy to maintain.
 *   
 * \note
 * This class is not good for things like z-ordering, but it's easy to assign an arbitrary
 *   order while searching for things to draw, since the search function takes an "action" to
 *   be performed when an item is matches.
 *   
 * The spatial index works by using a TreeMap to hold the "start" and "end" points of each shape
 *   for each axis (so, a TreeSet for the x-component, and one for the y-component). Then, when 
 *   asked for a set of points within a given bounding box, this class simply iterates over the
 *   (pre-sorted) x and y-components as stored in the TreeSets and checks which match. 
 *   
 * Some consideration is made for very "long" items (whose start/end points may not be within a 
 *   very zoomed-in range); for these, see the "health" of the index. This is not really a problem
 *   unless the disparity is huge. 
 *   
 * There's also the problem of having two points at exactly the same start or end index (x or y). 
 *   We could solve this by using ArrayLists of elements, or by bumping the x/y value very slightly 
 *   in one direction (or, by using something like Google's TreeMultiMap). 
 *  
 * \author Seth N. Hetu
 */
public class LazySpatialIndex<ItemType> {
	//Helper class for storing object data
	class AxisPoint {
		ItemType item;
		boolean isStart;
		boolean isEnd() { return !isStart; }
		double size; //start+size = end
		AxisPoint(ItemType item, boolean isStart, double size) {
			this.item = item;
			this.isStart = isStart;
			this.size = size;
		}
	}
	
	
	//Actual objects
	TreeMap<Double, ArrayList<AxisPoint>> axis_x;
	TreeMap<Double, ArrayList<AxisPoint>> axis_y;
	
	//Bookkeeping
	double maxWidth;
	double maxHeight;
	
	//What to do when we "find" an item
	public static interface Action<ItemType>  {
		public void doAction(ItemType item);
	}
	
	public LazySpatialIndex() {
		axis_x = new TreeMap<Double, ArrayList<AxisPoint>>();
		axis_y = new TreeMap<Double, ArrayList<AxisPoint>>();
		maxWidth = 0;
		maxHeight = 0;
	}
	
	
	//Return the bounds of the entire set
	public Rectangle2D getBounds() {
		return new Rectangle2D.Double(
			axis_x.firstKey(), axis_y.firstKey(), 
			axis_x.lastKey()-axis_x.firstKey(), 
			axis_y.lastKey()-axis_y.firstKey());
	}
	
	//Includes "estimate" factor.
	public Rectangle2D getBoundsExpanded() { 
		Rectangle2D res = getBounds();
		expandRectangle(res, 0.001);
		return res;
	}
	
	//Get an estimate of the health of this lookup, based on the difference between the 
	//  average width/height and the maximum. The result has an x and a y component, ranging
	//  from 0.0 (bad) to 1.0 (good).
	public Point2D estimateHealth() {
		return new Point2D.Double(1.0-getNegHealth(axis_x, maxWidth), 1.0-getNegHealth(axis_y, maxHeight));
	}
	
	
  	private void expandRectangle(Rectangle2D rect, double expandBy) {
  		resizeRectangle(rect, 
  			rect.getWidth() + rect.getWidth()*expandBy, 
  			rect.getHeight() + rect.getHeight()*expandBy);
  	}
	
	private void resizeRectangle(Rectangle2D rect, double newWidth, double newHeight) {
		if ((rect.getWidth()==newWidth) && (rect.getHeight()==newHeight)) { return; }
  		rect.setRect(
  			rect.getCenterX()-newWidth/2, 
  			rect.getCenterY()-newHeight/2, 
  			newWidth, newHeight);
  	}
	
	
	//Helper: Add, but deal with arrays
	private void add_to_axis(TreeMap<Double, ArrayList<AxisPoint>> axis, double key, AxisPoint value) {
		if (!axis.containsKey(key)) {
			axis.put(key, new ArrayList<AxisPoint>());
		}
		axis.get(key).add(value);
	}
	
	
	public void addItem(ItemType item, Rectangle2D bounds) {
		//We can easily support this later, if required.
		if (bounds.getWidth()==0 || bounds.getHeight()==0) { throw new RuntimeException("width/height must be non-zero."); }
		//temp_check_bounds(bounds);
		
		//Insert start/end points into both the x and y axis.
		add_to_axis(axis_x, bounds.getMinX(), new AxisPoint(item, true, bounds.getWidth()));
		add_to_axis(axis_x, bounds.getMaxX(), new AxisPoint(item, false, bounds.getWidth()));
		add_to_axis(axis_y, bounds.getMinY(), new AxisPoint(item, true, bounds.getHeight()));
		add_to_axis(axis_y, bounds.getMaxY(), new AxisPoint(item, false, bounds.getHeight()));
		
		//Update the maximum width/height
		maxWidth = Math.max(maxWidth, bounds.getWidth());
		maxHeight = Math.max(maxHeight, bounds.getHeight());
	}
	
	//BoundsHint can be null; searching is faster if it's not.
	//TODO: We aren't using the bounds correctly right now; we need to search for MAX (whatever)
	//      using a backwards-indexed key set, and MIN (whatever) using a forwards-indexed one.
	//NOTE: Our current approach is still valid; it's just not as fast as it could be for wide items.
	//      And I really don't care, since the only items we regularly remove are Agents, which are small.
	public void removeItem(ItemType item, Rectangle2D boundsHint) {
		//Search the whole area if no boundsHint is included.
		if (boundsHint==null) {
			boundsHint = getBounds();
		}
		
		//Expand the bounding box slightly, to avoid possible double/float inaccuracies.
		expandRectangle(boundsHint, 0.001); //0.1%
		
		//Now, search all points within this rectangle. We need to find start/end points for x/y, or
		//  it's an error (this happens inside search_for_item).
		Double[] res = new Double[] {null, null, null, null}; //startX, endX, startY, endY
		search_and_remove_item(axis_x, boundsHint.getMinX(), boundsHint.getMaxX(), item, res, 0, 1);
		search_and_remove_item(axis_y, boundsHint.getMinY(), boundsHint.getMaxY(), item, res, 2, 3);
		
		//We are guaranteed 4 non-null results. If their corresponding arrays are empty, remove them.
		for (int i=0; i<4; i++) {
			TreeMap<Double, ArrayList<AxisPoint>> axis = i<2 ? axis_x : axis_y;
			if (axis.get(res[i]).isEmpty()) {
				axis.remove(res[i]);
			}
		}
		
		//Update the maximum width/height
		double currWidth = res[1] - res[0];
		maxWidth = update_maximum(currWidth, maxWidth, axis_x);
		double currHeight = res[3] - res[2];
		maxHeight = update_maximum(currHeight, maxHeight, axis_y);
	}
	
	
	//Move = remove + add. Don't just re-add it; this will likely cause all sorts of nasty errors.
	public void moveItem(ItemType item, Rectangle2D newBounds, Rectangle2D oldBounds) {
		removeItem(item, oldBounds);
		addItem(item, newBounds);
	}
	
	//In case you want all of them.
	public void forAllItems(Action<ItemType> toDo) {
		//When scanning the entire axis, we only need to respond to "start" points.
		int foundPoints = 0;
		for (ArrayList<AxisPoint> ap : axis_x.values()) {
			for (AxisPoint it : ap) {
				//Avoid firing twice:
				if (it.isStart) {
					//"do" this action.
					toDo.doAction(it.item);
					foundPoints++;
				}
				
				//We can stop early if we're processed half of all points on this axis.
				if (foundPoints >= axis_x.size()/2) {
					break;
				}
			}
		}
	}
	
	
	//Helper class for matching
	private class AxisMatch {
		boolean matchX;
		boolean isFalsePos; //This must be set before disptach.
		boolean matchY;     //If "true", we've already dispatched this action()
			
		AxisMatch() { matchX=false; matchY=false; isFalsePos = false;}
	}
	
	//Return the "actual" rectangle used for searching.
	public Rectangle2D getActualSearchRectangle(Rectangle2D src) {
		if (src.isEmpty()) { return src; }
		Rectangle2D res = new Rectangle2D.Double(src.getX(), src.getY(), src.getWidth(), src.getHeight());
		expandRectangle(res, 0.001);
		return res;
	}
	
	
	//Perform an action on all items within a given range
	//toDo and doOnFalsePositives can be null; the first is the action to perform on a given
	//  match; the second is related to the "health" of the set.
	public void forAllItemsInRange(Rectangle2D orig_range, Action<ItemType> toDo, Action<ItemType> doOnFalsePositives) {
		//Sanity check
		if (orig_range.isEmpty()) { return; }
				
		//Expand range slightly, just to avoid boundary issues.
		Rectangle2D range = getActualSearchRectangle(orig_range);
 		
 		//Our algorithm will skip long segments entirely (unless a single start or end point is matched). 
 		// There are several solutions to this, but we will simply expand the search box.
 		boolean possibleFP = (range.getWidth()<maxWidth || range.getHeight()<maxHeight);
 		Rectangle2D match_range = new Rectangle2D.Double(range.getX(), range.getY(), range.getWidth(), range.getHeight());
 		if (possibleFP) {
 			resizeRectangle(match_range, Math.max(range.getWidth(), maxWidth), Math.max(range.getHeight(), maxHeight));
   		} 		
		
 		//Because each point stores whether it is a start or end point (as well as its total size), we can determine
 		// whether an item matches *directly*, is a *false positive*, or *doesn't match* at all. 
 		//This allows us to dispatch the toDo() and doOnFalsePositives() actions immediately upon encountering a point 
 		// in the y-direction.
 		//We don't strictly need to "save" which points have already been dispatched (we can recalculate it), but it 
 		// makes for a much simpler algorithm (and we need to save data from the x-axis anyway, so it's not very wasteful).
 		Hashtable<ItemType, AxisMatch> matchedItems = new Hashtable<ItemType, AxisMatch>();
 		
 		//Add items on the x-axis, detecting whether they're false-positives or not.
		NavigableMap<Double, ArrayList<AxisPoint>> axis = axis_x.subMap(match_range.getMinX(), true, match_range.getMaxX(), true);
		for (Entry<Double, ArrayList<AxisPoint>> ent : axis.entrySet()) {
			for (AxisPoint ap : ent.getValue()) {
				//Expand the hashtable as required.
				ItemType item = ap.item;
				if (!matchedItems.containsKey(item)) {
					matchedItems.put(item, new AxisMatch());
				}
				
				//If we've already determined that this macthes, there's no need for further math.
				AxisMatch match = matchedItems.get(item);
				if (match.matchX) { continue; } 
				
				//Determine if this is actually a false-positive. Essentially, the shape is false if it doesn't fall
				//   into the original range rectangle requested.
				if (possibleFP && !match.isFalsePos) {
					double startPt = 0;
					double endPt = 0;
					if (ap.isStart) {
						startPt = ent.getKey();
						endPt = startPt + ap.size;
					} else if (ap.isEnd()) { 
						endPt = ent.getKey();
						startPt = endPt - ap.size;
					}
					match.isFalsePos = !(range.intersects(startPt, range.getCenterY(), endPt-startPt, 1));
				}
							
				//Matched
				match.matchX = true;
			}
		}
		
		//Now match on the y-axis. Same logic, but this time we call the relevant function.
		//TODO: We might want to put this code into a shared subroutine.
		axis = axis_y.subMap(match_range.getMinY(), true, match_range.getMaxY(), true);
		for (Entry<Double, ArrayList<AxisPoint>> ent : axis.entrySet()) {
			for (AxisPoint ap : ent.getValue()) {
				//Skip if already matched, or if there's no potential for a match (x didn't match)
				ItemType item = ap.item;
				if (!matchedItems.containsKey(item)) { continue; }
				AxisMatch match = matchedItems.get(item);
				if (match.matchY) { continue; }
						
				//Determine if this is actually a false-positive. Essentially, the shape is false if it doesn't fall
				//   into the original range rectangle requested.
				if (possibleFP && !match.isFalsePos) {
					double startPt = 0;
					double endPt = 0;
					if (ap.isStart) {
						startPt = ent.getKey();
						endPt = startPt + ap.size;
					} else if (ap.isEnd()) { 
						endPt = ent.getKey();
						startPt = endPt - ap.size;
					}
					match.isFalsePos = !(range.intersects(range.getCenterX(), startPt, 1, endPt-startPt));
				}
				
				//Fire
				if (match.isFalsePos) {
					if (doOnFalsePositives!=null) {
						doOnFalsePositives.doAction(item);
					}
				} else {
					if (toDo!=null) {
						toDo.doAction(item);
					}
				}
				
				//Matched
				match.matchY = true;
			}
		}
	}

	
	
	//Helper
	//TODO: This probably needs to be modified if we want to support "point" items.
	private void search_and_remove_item(TreeMap<Double, ArrayList<AxisPoint>> axisMap, double minVal, double maxVal, ItemType searchFor, Double[] keyResults, int minKeyID, int maxKeyID) {
		NavigableMap<Double, ArrayList<AxisPoint>> partialRes = axisMap.subMap(minVal, true, maxVal, true);
		int currResID = minKeyID;
		for (Entry<Double, ArrayList<AxisPoint>> point : partialRes.entrySet()) {
			if (point.getValue().remove(searchFor)) {
				if (currResID > maxKeyID) { throw new RuntimeException("Error: Possible duplicates"); }
				if (keyResults[currResID] != null) { throw new RuntimeException("Error: Key overlap (unexpected)."); }
				keyResults[currResID++] = point.getKey();
			}
		}
		if (currResID != maxKeyID+1) { throw new RuntimeException("Error: Couldn't find both keys, missing " + (maxKeyID+1 - currResID)  + "."); }
	}


	//Helper: If we are removing the current maximum, we need to search for the new maximum.
	private double update_maximum(double currVal, double maxVal, TreeMap<Double, ArrayList<AxisPoint>> axis) {
		//TODO: We should actually perform a search. However, since we never actually remove "static"
		//      network items (and these are the ones with large width/heights), we can just keep the "old"
		//      maximum value.
		return maxVal;
	}
	
	//Helper: get the inverse of the health
	private double getNegHealth(TreeMap<Double, ArrayList<AxisPoint>> axis, double max_size) {
		//Sanity check
		if (axis.size()%2!=0) { throw new RuntimeException("Axis pair imbalance: " + axis.size()); }
		
		//Normalize
		double size = axis.lastKey() - axis.firstKey();
		int numPairs = axis.size() / 2;
		
		//Iterate, compute the average
		double average = 0.0;
		Map<ItemType, Double> startPoints = new HashMap<ItemType, Double>();
		for (Entry<Double, ArrayList<AxisPoint>> entry : axis.entrySet()) {
			for (AxisPoint ap : entry.getValue()) {
				ItemType item = ap.item;
				if (!startPoints.containsKey(item)) {
					startPoints.put(item, entry.getKey());
				} else {
					//Get the normalized size.
					double norm_size = entry.getKey() - startPoints.get(item);
					norm_size /= size;
					startPoints.remove(item); //Allows us to check consistency after.
					
					//Add it to the average
					average += norm_size / numPairs;
				}
			}
		}
		
		//Ensure we have no leftovers
		if (!startPoints.isEmpty()) {
			throw new RuntimeException("Error: " + startPoints.size() + " leftover points.");
		}
		
		//Return the difference between the normalized average and the normalized max size
		return Math.abs((max_size/size) - average);
	}
	
	
	//Remove this restriction as soon as it's stable.
	/*private void temp_check_bounds(Rectangle2D bounds) {
		if (
			axis_x.containsKey(bounds.getMinX()) ||
			axis_x.containsKey(bounds.getMaxX()) ||
			axis_y.containsKey(bounds.getMinY()) ||
			axis_y.containsKey(bounds.getMaxY())
		) {
			throw new RuntimeException("TODO: Need to allow point collisions.");
		}
	}*/
	
}

