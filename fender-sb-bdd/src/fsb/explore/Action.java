package fsb.explore;

import java.util.WeakHashMap;

import fsb.ast.Statement;
import fsb.ast.Statement.StatType;
import fsb.utils.Debug;
import fsb.utils.Options;

public class Action {
	State src;
	State dst;
	Statement st;
	int pid;
	String name;
	
	private static WeakHashMap<String, String> cachedNames = new WeakHashMap<String, String>();
	
	/**
	 * YM: the other constructor created name is unassigned.
	 * @param src
	 * @param dst
	 * @param st
	 * @param pid
	 * @param name
	 */
	public Action(State src, State dst, Statement st, int pid)//, String name)
	{
		this.src = src;
		this.dst = dst;
		this.pid = pid;
		this.st = st;
		
		
	}
	/**
	 * YM: the name is a debugging mechanism to best of my understanding
	 * so the other constructor was created without it.
	 * @param src
	 * @param dst
	 * @param st
	 * @param pid
	 * @param name
	 */
	public Action(State src, State dst, Statement st, int pid, String name)
	{
		this.src = src;
		this.dst = dst;
		this.pid = pid;
		if (Debug.DEBUG_LEVEL > 1)
			this.name = name;
		this.st = st;
		
		if (Debug.DEBUG_LEVEL > 1)
		{
			String cached = cachedNames.get(name);
			if (cached == null)
				cached = cachedNames.put(name, name);	
			else
				this.name = cached;
		}
		
		
	}
	
	public State getSource() { return src; }
	public State getTarget() { return dst; }
	public int getPid() { return pid; }
	
	public String toString() {
			return "" + pid + "." + st.getLabel() + ": " + st.toString() ; 
		}
	
	public Statement getStatement() {return st;}

	public int compareTo(Action act) {		
		throw new RuntimeException("What is this for?");
	}

}
