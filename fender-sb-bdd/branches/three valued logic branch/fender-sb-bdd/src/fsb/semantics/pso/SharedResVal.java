package fsb.semantics.pso;

import java.util.Arrays;
import java.util.LinkedList;

import fsb.semantics.AbsSharedResVal;
import fsb.semantics.BufVal;

public class SharedResVal implements Cloneable, AbsSharedResVal
{	
	int value;
	LinkedList<BufVal> buffer[];
	
	@SuppressWarnings("unchecked")
	public SharedResVal(int numProcs, int init)
	{
		value = init;
		buffer = new LinkedList[numProcs];
		for (int i = 0; i < numProcs; i++)
			buffer[i] = new LinkedList<BufVal>();
	}
	
	@SuppressWarnings("unchecked")
	public SharedResVal(SharedResVal other) {
		value = other.value;
		//TODO: LAZY_COPY!!!
		buffer = other.buffer.clone();
		for (int i = 0; i < buffer.length; i++)
			buffer[i] = (LinkedList<BufVal>) buffer[i].clone();
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
		StringBuffer sb = new StringBuffer();
		sb.append(value);
		sb.append(", [[");
		for (int i = 0; i < buffer.length; i++)
		{
			sb.append(i);
			sb.append(": {");
			sb.append(buffer[i]);
			sb.append("} ");
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
	
	public int getVal(int pid)
	{
		if (buffer[pid].isEmpty())
			return value;
		else
			return buffer[pid].getFirst().contents;
	}
	
	public void writeVal(int pid, int label, int val)
	{
		buffer[pid].addFirst(new BufVal(val, label));
	}
	
	public int getGlobalVal()
	{
		return value;
	}
}
