package fsb.ast;

import java.util.ArrayList;
import java.util.List;

public class Declarations {
	List<String> shared, local;
	
	public Declarations(ArrayList<String> shared, ArrayList<String> local) {
		this.shared = shared;
		this.local = local;
	}

	public String toString()
	{
		return "Shared: " + shared + "\n" + "Local: " + local;
	}
}
