package fsb.ast;

import java.util.ArrayList;
import java.util.List;

public class Declarations {
	List<Integer> shared, local;
	
	public Declarations(ArrayList<String> shared, ArrayList<String> local) {
		this.shared = new ArrayList<Integer>();
		this.local = new ArrayList<Integer>();
		for(String sh : shared){
			int sh_indx = SymbolTable.getOrCreateGlobalVariable(sh);
			this.shared.add(sh_indx);
		}
		
		for(String lc : local){
			int lc_indx = SymbolTable.getOrCreateLocalVariable(lc);
			this.local.add(lc_indx);
		}
	}

	public String toString()
	{
		String ret = "Shared: ";
		boolean first = true;
		for(Integer sh : shared){
			String sh_name = SymbolTable.getGlobalVariableName(sh);
			if(!first){
				ret += ", ";
			}else{
				first = false;
			}
			ret += sh_name;
		}

		ret +=  "\n" + "Local: ";
		first = true;
		for(int lc : local){
			String lc_name = SymbolTable.getLocalVariableName(lc);
			if(!first){
				ret += ", ";
			}else{
				first = false;
			}
			ret += lc_name;
		}
		
		return ret;
	}
}
