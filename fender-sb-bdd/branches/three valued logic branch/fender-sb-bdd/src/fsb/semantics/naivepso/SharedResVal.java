package fsb.semantics.naivepso;

import java.util.Arrays;
import java.util.HashSet;
import java.util.WeakHashMap;

import fsb.semantics.AbsSharedResVal;
import fsb.utils.Options;

public class SharedResVal implements Cloneable, AbsSharedResVal
{
	public enum validity {VALID, NOTVALID, TOP};
	
	private static WeakHashMap<AbstractBuffer, AbstractBuffer> cachedBuffers = new WeakHashMap<AbstractBuffer, AbstractBuffer>();
	
	public class AbstractBuffer implements Cloneable
	{	
		HashSet<Integer> set;
				
		public AbstractBuffer()
		{
			set = new HashSet<Integer>(2);
		}
		
		@SuppressWarnings("unchecked")
		public AbstractBuffer(AbstractBuffer other)
		{
			set = (HashSet<Integer>) other.set.clone();
		}
		
		public Object clone()
		{
			AbstractBuffer newbuffer = new AbstractBuffer(this);
			return newbuffer;
		}
		
		public boolean isEmpty()
		{
			return set.isEmpty();
		}
		
		@Override
		public int hashCode()
		{
			return set.hashCode();
		}
		
		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (!(obj instanceof AbstractBuffer)) return false;	
			AbstractBuffer other = (AbstractBuffer) obj;
			
			return (set.equals(other.set));					
		}
	}
	
	private int value;
	//TODO: Should be private
	AbstractBuffer buffer[];
	
	SharedResVal(int numProcs, int init)
	{
		value = init;
		buffer = new AbstractBuffer[numProcs];
		for (int i = 0; i < numProcs; i++)
			buffer[i] = new AbstractBuffer();		
	}
	
	public SharedResVal(SharedResVal other) {
		value = other.value;
		//Note this is NOT a deep copy.
		buffer = other.buffer.clone();
		if (!Options.LAZY_COPY)
		{
			for (int i = 0; i < buffer.length; i++)
				buffer[i] = (AbstractBuffer) buffer[i].clone();
		}
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!(obj instanceof SharedResVal)) return false;	
		SharedResVal other = (SharedResVal) obj;	
		
		return (other.value == value) && (Arrays.equals(other.buffer, buffer));		
		
	}
	
	@Override
	public int hashCode()
	{
		final int prime = 31;
		int result = value;
		result = prime * result + Arrays.hashCode(buffer);
		return result;
	}
	
	@Override
	public String toString()
	{
		//TODO Fix up to use tail
		StringBuffer sb = new StringBuffer();
		sb.append(value);
		sb.append(", [[");
		for (int i = 0; i < buffer.length; i++)
		{
			if (!buffer[i].isEmpty())
			{
				sb.append(i);
				sb.append(": {");
				sb.append(buffer[i].set);
				sb.append("}");
			}
		}
		sb.append("]]");
		
		return sb.toString();
	}
	
	@Override
	public AbsSharedResVal clone()
	{
		SharedResVal newval = new SharedResVal(this);		
		return newval;
	}
	
	public int getGlobalVal()
	{
		return value;
	}
	
	public void store(int pid, int val)
	{
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();

		AbstractBuffer buf = buffer[pid];		
		buf.set.add(val);
		
        AbstractBuffer cached = cachedBuffers.get(buf);
		if (cached == null)
			cachedBuffers.put(buf, buf);
		else
			buffer[pid] = cached;
	}
		
	public void clearBuffer(int pid)
	{
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();

		buffer[pid].set.clear();
		
        AbstractBuffer cached = cachedBuffers.get(buffer[pid]);
		if (cached == null)
			cachedBuffers.put(buffer[pid], buffer[pid]);
		else
			buffer[pid] = cached;
	}
	
	public boolean isEmptySet(int pid)
	{
		return buffer[pid].set.isEmpty();
	}

	public void directSetValue(int val)
	{
		value = val;
	}
	
	public int flushDestructive(int pid, int val) {
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();
		
		buffer[pid].set.remove(val);
		
		AbstractBuffer cached = cachedBuffers.get(buffer[pid]);
		if (cached == null)
			cachedBuffers.put(buffer[pid], buffer[pid]);
		else
			buffer[pid] = cached;
		
		return value;
	}
}
