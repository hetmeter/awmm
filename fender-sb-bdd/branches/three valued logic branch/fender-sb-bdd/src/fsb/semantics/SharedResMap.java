package fsb.semantics;

import java.util.TreeMap;
import java.util.Map.Entry;

@SuppressWarnings("serial")
public class SharedResMap<T extends AbsSharedResVal> extends TreeMap<String, T> 
{
	@SuppressWarnings("unchecked")
	@Override
	public Object clone()
	{
		SharedResMap<T> newmap = (SharedResMap<T>) super.clone();
		
		//TODO: EEK
		for (Entry<String, T> e : newmap.entrySet())
		{
			T val = newmap.get(e.getKey());
			newmap.put(e.getKey(), (T) val.clone());
		}
		return newmap;
	}
	
	@Override 
	public int hashCode()
	{
		return super.hashCode();
	}
}
