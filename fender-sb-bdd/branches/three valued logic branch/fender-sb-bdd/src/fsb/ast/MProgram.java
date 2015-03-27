package fsb.ast;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import fsb.explore.ExprValidator;
import fsb.explore.Validator;

public class MProgram {
	Declarations decl;
	Vector<Code> programs;
	Validator validator;
	
	public MProgram(Declarations decl, ArrayList<Code> programs) {
		this.decl = decl;
		this.programs = new Vector<Code>(programs.size());
		this.programs.addAll(programs);
		this.validator = null;
	}
	
	public MProgram(Declarations decl, ArrayList<Code> programs, Assert ass) {
		this.decl = decl;
		this.programs = new Vector<Code>(programs.size());
		this.programs.addAll(programs);
		this.validator = new ExprValidator(ass.getExpr(), ass.alwaysActive());
	}

	public String toString()
	{
		return decl.toString() + "\n" + programs.toString();
	}
	
	public List<Statement> getListing(int pid)
	{
		return programs.get(pid).program;
	}
	
	public List<String> getShared()
	{
		return decl.shared;
	}
	
	public List<String> getLocals()
	{
		return decl.local;
	}

	public int getProcs() {
		// TODO Auto-generated method stub
		return programs.size();
	}
	
	public Validator getValidator()
	{
		return validator;
	}
}
