package fsb.semantics;

import fsb.tvl.ArithValue;

public class BufVal
{
	public ArithValue contents;
	public int writingLabel;
	
	public BufVal(ArithValue contents, int writingLabel)
	{
		this.contents = contents;
		this.writingLabel = writingLabel;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!(obj instanceof BufVal)) return false;
		BufVal other = (BufVal) obj;
		return contents == other.contents && writingLabel == other.writingLabel;
	}
	
	@Override
	public int hashCode()
	{
		final int prime = 31;
		int result = prime * contents.hashCode() + writingLabel;
		return result;
	}
	
	@Override
	public String toString()
	{
		return "[" + contents + "," + writingLabel +"]";
	}
}