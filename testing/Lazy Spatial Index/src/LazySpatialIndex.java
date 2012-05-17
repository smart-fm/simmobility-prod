import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.HashSet;
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
	//Actual objects
	TreeMap<Double, ItemType> axis_x;
	TreeMap<Double, ItemType> axis_y;
	
	//Bookkeeping
	double maxWidth;
	double maxHeight;
	
	//What to do when we "find" an item
	public static interface Action<ItemType>  {
		public void doAction(ItemType item);
	}
	
	public LazySpatialIndex() {
		axis_x = new TreeMap<Double, ItemType>();
		axis_y = new TreeMap<Double, ItemType>();
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
		Point2D buff = new Point2D.Double(rect.getWidth()*expandBy, rect.getHeight()*expandBy);
		rect.setRect(
			rect.getX()-buff.getX()/2, 
			rect.getY()-buff.getY()/2, 
			rect.getWidth()+buff.getX(), 
			rect.getHeight()+buff.getY());
	}
	
	
	public void addItem(ItemType item, Rectangle2D bounds) {
		//We can easily support this later, if required.
		if (bounds.getWidth()==0 || bounds.getHeight()==0) { throw new RuntimeException("width/height must be non-zero."); }
		temp_check_bounds(bounds);
		
		//Insert start/end points into both the x and y axis.
		axis_x.put(bounds.getMinX(), item);
		axis_x.put(bounds.getMaxX(), item);
		axis_y.put(bounds.getMinY(), item);
		axis_y.put(bounds.getMaxY(), item);
		
		//Update the maximum width/height
		maxWidth = Math.max(maxWidth, bounds.getWidth());
		maxHeight = Math.max(maxHeight, bounds.getHeight());
	}
	
	//BoundsHint can be null; searching is faster if it's not.
	//TODO: We aren't using the bounds correctly right now; we need to search for MAX (whatever)
	//      using a backwards-indexed key set, and MIN (whatever) using a forwards-indexed one.
	//NOTE: Our current approach is still valid; it's just not as fast as it could be for wide items.
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
		search_for_item(axis_x, boundsHint.getMinX(), boundsHint.getMaxX(), item, res, 0, 1);
		search_for_item(axis_y, boundsHint.getMinY(), boundsHint.getMaxY(), item, res, 2, 3);
		
		//We are guaranteed 4 non-null results; now, remove them:
		axis_x.remove(res[0]);
		axis_x.remove(res[1]);
		axis_y.remove(res[2]);
		axis_y.remove(res[3]);
		
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
	
	//In case you want all of them. Note that preventDuplicates will be faster but use slightly more memory.
	public void forAllItems(Action<ItemType> toDo, boolean preventDuplicates) {
		//Each item will have entries in both the x/y axes, so we only need to scan one.
		HashSet<ItemType> alreadyDone = new HashSet<ItemType>();
		for (ItemType it : axis_x.values()) {
			//Avoid firing twice:
			if (alreadyDone.contains(it)) {
				continue;
			}
			
			//"do" this action.
			toDo.doAction(it);
			
			//Save it to prevent duplicates. (By not saving it, we ensure that duplicate actions are taken.)
			if (preventDuplicates) {
				alreadyDone.add(it);
			}
			
			//We can stop early if we're processed half of all points on this axis. This only works if
			//  preventDuplicates is true.
			if (alreadyDone.size() >= axis_x.size()/2) {
				break;
			}
		}
	}
	
	
	//Perform an action on all items within a given range
	//toDo and doOnFalsePositives can be null; the first is the action to perform on a given
	//  match; the second is related to the "health" of the set.
	public void forAllItemsInRange(Rectangle2D range, Action<ItemType> toDo, Action<ItemType> doOnFalsePositives, boolean preventDuplicates) {
		//Sanity check
		if (range.isEmpty()) { return; }
		
		
	}
	
	
	
	//Helper
	private void search_for_item(TreeMap<Double, ItemType> axisMap, double minVal, double maxVal, ItemType searchFor, Double[] keyResults, int minKeyID, int maxKeyID) {
		NavigableMap<Double, ItemType> partialRes = axisMap.subMap(minVal, true, maxVal, true);
		int currResID = minKeyID;
		for (Entry<Double, ItemType> item : partialRes.entrySet()) {
			if (item.getValue() == searchFor) { //TODO: I think we want reference equality here vs. value equality.
				if (currResID > maxKeyID) { throw new RuntimeException("Error: Possible duplicates"); }
				if (keyResults[currResID] != null) { throw new RuntimeException("Error: Key overlap (unexpected)."); }
				keyResults[currResID++] = item.getKey();
			}
		}
		if (currResID != maxKeyID+1) { throw new RuntimeException("Error: Couldn't find both keys, missing " + (maxKeyID+1 - currResID)  + "."); }
	}


	//Helper: If we are removing the current maximum, we need to search for the new maximum.
	private double update_maximum(double currVal, double maxVal, TreeMap<Double, ItemType> axis) {
		//TODO: We should actually perform a search. However, since we never actually remove "static"
		//      network items (and these are the ones with large width/heights), we can just keep the "old"
		//      maximum value.
		return maxVal;
	}
	
	//Helper: get the inverse of the health
	private double getNegHealth(TreeMap<Double, ItemType> axis, double max_size) {
		//Normalize
		double size = axis.lastKey() - axis.firstKey();
		int numPairs = axis.size() / 2;
		
		//Iterate, compute the average
		double average = 0.0;
		Map<ItemType, Double> startPoints = new HashMap<ItemType, Double>();
		for (Entry<Double, ItemType> entry : axis.entrySet()) {
			if (!startPoints.containsKey(entry.getValue())) {
				startPoints.put(entry.getValue(), entry.getKey());
			} else {
				//Get the normalized size.
				double norm_size = entry.getKey() - startPoints.get(entry.getValue());
				norm_size /= size;
				startPoints.remove(entry.getValue()); //Allows us to check consistency after.
				
				//Add it to the average
				average += norm_size / numPairs;
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
	private void temp_check_bounds(Rectangle2D bounds) {
		if (
			axis_x.containsKey(bounds.getMinX()) ||
			axis_x.containsKey(bounds.getMaxX()) ||
			axis_y.containsKey(bounds.getMinY()) ||
			axis_y.containsKey(bounds.getMaxY())
		) {
			throw new RuntimeException("TODO: Need to allow point collisions.");
		}
	}
	
}

