package fsb.ast;

import java.util.ArrayList;
import java.util.List;

public class Code {
	int pid;
	List<Statement> program;
	
	public Code(Integer pid, ArrayList<Statement> program) {
		this.pid = pid;
		this.program = program;
	}
	
	public String toString()
	{
		return "\npid " + pid + ":\n" + program + "";
	}

}
