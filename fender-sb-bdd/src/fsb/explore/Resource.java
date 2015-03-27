package fsb.explore;

abstract public class Resource implements Comparable<Resource> {
	private String name;
	
	protected Resource(String name)
	{
		this.name = name;
	}	
	
	protected Resource(Resource r)
	{
		this.name = r.name;
	}	
	
	public String getName()
	{
		return name;
	}

	public boolean equals(Object obj)
	{
		if (this == obj)
			return true;
		if (!(obj instanceof Resource)) return false;	
		Resource r = (Resource) obj;
		
		return (r.name.equals(name));
	}
	
	public int hashCode()
	{
		return name.hashCode();
	}
	
	@Override
	public String toString()
	{
		return name;
	}
	
	@Override
	public int compareTo(Resource other)
	{
		return this.name.compareTo(other.name);
	}
}
