package sim_mob.act;

import java.util.ArrayList;


//Provides a simple wait/notify model for activities.
public class SimpleThreadPool {
	ArrayList<Thread> activeThreads;
	int maxActiveThreads;
	
	public SimpleThreadPool(int maxActiveThreads) {
		this.maxActiveThreads = maxActiveThreads;
		activeThreads = new ArrayList<Thread>();
	}
	
	
	/**
	 * Start a new task. An Activity is performed as a thread, with the maxActiveThreads
	 * count respected. If the maximum number of threads has already been reached, the caller
	 * sleeps until a thread finishes.
	 * 
	 * \param force If true, always allocate a thread, even if the maximum has been reached.
	 * \param act The activity to spin off a thread for.
	 */
	public void newTask(Activity act) { newTask(act, false); }
	public void newTask(Activity act, boolean force) {
		synchronized(this) {
			//Start a thread now?
			boolean startNow = force || (activeThreads.size()<maxActiveThreads);
			if (!startNow) {
				try {
					this.wait();
				} catch (InterruptedException ex) { throw new RuntimeException(ex); }
			}
			
			//If we were woken, the means that *at least one* thread finished, so by definition we have enough
			//  threads left (unless force was called a lot, but we only expect to "force" the last call anyway).
			Thread t = new ActivityThread(this, act);
			activeThreads.add(t);
			t.start();
		}
	}
	
	public void joinAll() {
		for (;;) {
			Thread next = null;
			synchronized (this) {
				if (activeThreads.isEmpty()) {
					break;
				}  else {
					next = activeThreads.get(0);
				}
			}
			try {
				next.join();
			} catch (InterruptedException ex) { throw new RuntimeException(ex); }
		}
	}
	
	
	public void taskFinished(Thread task) {
		synchronized (this) {
			this.activeThreads.remove(task);
			this.notify(); //Should only ever be one item waiting on this.
		}
	}
	
	
	private static class ActivityThread extends Thread  {
		//
		SimpleThreadPool parent;
		Activity act;
		ActivityThread(SimpleThreadPool parent, Activity act) {
			this.parent = parent;
			this.act = act;
		}
		

		//Simple; just perform the action then notify
		public void run() {
			act.run();
			
			//Should only ever be one item (the parent) waiting at any given time.
			parent.taskFinished(this);
		}
	}
	
}



