package fsb.semantics.eagerpso;

import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.WeakHashMap;
import fsb.semantics.AbsSharedResVal;
import fsb.semantics.BufVal;
import fsb.utils.Options;

public class SharedResVal implements Cloneable, AbsSharedResVal
{	
	private static WeakHashMap<AbstractBuffer, AbstractBuffer> cachedBuffers = new WeakHashMap<AbstractBuffer, AbstractBuffer>();
	
	public class AbstractBuffer implements Cloneable
	{	
		int last;
		HashSet<BufVal> set;
		LinkedList<BufVal> head;
				
		public AbstractBuffer()
		{
			//TODO: Create the set on-demand!
			set = new HashSet<BufVal>(2);
			head = new LinkedList<BufVal>();
		}
		
		@SuppressWarnings("unchecked")
		public AbstractBuffer(AbstractBuffer other)
		{
			last = other.last;
			set = (HashSet<BufVal>) other.set.clone();
			head = (LinkedList<BufVal>) other.head.clone();
		}
		
		public Object clone()
		{
			AbstractBuffer newbuffer = new AbstractBuffer(this);
			return newbuffer;
		}
		
		public boolean isEmpty()
		{
			return head.isEmpty() && set.isEmpty();
		}		
		
		@Override
		public int hashCode()
		{
			final int prime = 31;
			int result = last;
			result = prime * result + set.hashCode();
			result = prime * result + head.hashCode();
			return result;
		}
		
		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (!(obj instanceof AbstractBuffer)) return false;	
			AbstractBuffer other = (AbstractBuffer) obj;
			
			return (other.last == last && set.equals(other.set) && head.equals(other.head));					
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
				sb.append(": ");
				sb.append(buffer[i].last);
				sb.append(", {");
				sb.append(buffer[i].set);
				sb.append("}, (");
				sb.append(buffer[i].head);
				sb.append(") ");
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
	
	public int getLast(int pid)
	{		
		return buffer[pid].last;
	}
	
	public boolean isSingleton(int pid)
	{
		return buffer[pid].set.size() == 1;
	}	
	
	public int getGlobalVal()
	{
		return value;
	}
	
	public void store(int pid, int val, int writingLabel)
	{
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();

		AbstractBuffer buf = buffer[pid];
		buf.last = val;
		if (isEmptySet(pid) && !isFullHead(pid))
			buf.head.addLast(new BufVal(val, writingLabel));
		else
			buf.set.add(new BufVal(val, writingLabel));
		
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
		buffer[pid].head.clear();
		
        AbstractBuffer cached = cachedBuffers.get(buffer[pid]);
		if (cached == null)
			cachedBuffers.put(buffer[pid], buffer[pid]);
		else
			buffer[pid] = cached;
	}
	
	public boolean isEmptyHead(int pid)
	{
		return buffer[pid].head.isEmpty();
	}
	
	private boolean isFullHead(int pid)
	{
		return buffer[pid].head.size() == EagerAbsSemantics.headlen;
	}

	private boolean isEmptySet(int pid)
	{
		return buffer[pid].set.isEmpty();
	}
	
	public boolean nonEmpty(int pid)
	{
		return (!isEmptySet(pid)) || (!isEmptyHead(pid));
	}
	
	public void directSetValue(int val)
	{
		value = val;
	}

	public void flushFromHead(int pid) {
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();
		
		value = buffer[pid].head.pollFirst().contents;
		
		AbstractBuffer cached = cachedBuffers.get(buffer[pid]);
		if (cached == null)
			cachedBuffers.put(buffer[pid], buffer[pid]);
		else
			buffer[pid] = cached;
	}
	
	public void flushDestructive(int pid, BufVal bufval) {
		if (Options.LAZY_COPY)
			buffer[pid] = (AbstractBuffer) buffer[pid].clone();
		
		value = bufval.contents;
		if (!buffer[pid].set.remove(bufval))
			throw new RuntimeException("Sanity check - tried to flush a value that was never in the set!");
		
		AbstractBuffer cached = cachedBuffers.get(buffer[pid]);
		if (cached == null)
			cachedBuffers.put(buffer[pid], buffer[pid]);
		else
			buffer[pid] = cached;
		
	}
}
