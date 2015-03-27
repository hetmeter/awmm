package fsb.semantics;

public class BufVal
{
	public int contents;
	public int writingLabel;
	
	public BufVal(int contents, int writingLabel)
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
		int result = prime * contents + writingLabel;
		return result;
	}
	
	@Override
	public String toString()
	{
		return "[" + contents + "," + writingLabel +"]";
	}
}