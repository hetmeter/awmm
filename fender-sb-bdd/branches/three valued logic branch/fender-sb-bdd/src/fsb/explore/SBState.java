package fsb.explore;

import java.util.Arrays;
import java.util.TreeMap;
import java.util.WeakHashMap;

import fsb.ast.MProgram;
import fsb.ast.tvl.ArithValue;
import fsb.ast.tvl.DeterArithValue;

import fsb.utils.Options;

public abstract class SBState extends State {

	@SuppressWarnings("serial")
	protected class LocalResMap extends TreeMap<String, ArithValue> {}
	private static WeakHashMap<LocalResMap, LocalResMap> cachedLocals = new WeakHashMap<LocalResMap, LocalResMap>();
	
	protected final LocalResMap locals[];
	protected int pc[];
	protected int lastMem;
	
	public SBState(MProgram program, Validator validator, int numProcs) {
		super(program, validator, numProcs);
		
		this.locals = new LocalResMap[numProcs]; 
		this.pc = new int[numProcs];
		
		for (int i = 0; i < numProcs; i++)
		{
			this.locals[i] = new LocalResMap();
			for(String var : program.getLocals())
			{
				this.locals[i].put(var, DeterArithValue.getInstance(0));
			}
			this.pc[i] = 0;
		}
		this.lastMem = 1;
	}
	
	public SBState(SBState s)
	{
		super(s);
		this.locals = new LocalResMap[numProcs];
		this.pc = new int[numProcs];
		this.lastMem = s.lastMem;
		
		for (int i = 0; i < numProcs; i++)
		{
			this.locals[i] = (LocalResMap) s.locals[i];
			this.pc[i] = s.pc[i];
		}
		
		if (!Options.LAZY_COPY)
		{
			for (int i = 0; i < numProcs; i++)
			{
				this.locals[i] = (LocalResMap) s.locals[i].clone();				
			}		
		}
	}
	
	public void advancePC(int pid) {
		pc[pid]++;		
	}
	
	public void setPC(int pid, int toLabel) {
		pc[pid] = getLabelIndex(pid, toLabel);		
	}
	
	public ArithValue getLocal(int pid, String res)
	{	
		ArithValue val = locals[pid].get(res);		
		if (val == null)
			throw new RuntimeException("Local " + res + " not found");
		
		return val;
	}	
	
	public void setLocal(int pid, String res, ArithValue val)
	{
		if (!locals[pid].containsKey(res))
			throw new RuntimeException("Local " + res + " not found");
		
		if (Options.LAZY_COPY)
			locals[pid] = (LocalResMap)locals[pid].clone();
		
		locals[pid].put(res, val);
		LocalResMap cached = cachedLocals.get(locals[pid]);
		if (cached == null)
		{
			cachedLocals.put(locals[pid], locals[pid]);
		}
		else
		{
			locals[pid] = cached;
		}
				
	}

	public int getPC(int pid)
	{
		return pc[pid];
	}
	
	@Override
	public int hashCode()
	{
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + lastMem;
		result = prime * result + Arrays.hashCode(locals);
		result = prime * result + Arrays.hashCode(pc);
		
		return result;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (!super.equals(obj))
			return false;
		
		if (!(obj instanceof SBState)) return false;	
		SBState other = (SBState) obj;

		if (lastMem != other.lastMem)
			return false;		
		if (!Arrays.equals(locals, other.locals))
			return false;
		if (!Arrays.equals(pc, other.pc))
			return false;
		
		return true;
	}
	
	public int allocate(int num)
	{
		int start = lastMem;
		for (;lastMem < start + num; lastMem++)
		{
			addShared(getMemResource(lastMem),DeterArithValue.getInstance(0));
		}
		return start;
	}
		
	public boolean isInitFinished()
	{
		//return program.getInitListing().get(initpc).isLast();
		return program.getListing(0).get(pc[0]).isLast();
	}
	
	static public String getMemResource(ArithValue num)
	{
		return "MEM" + num;
	}
	
	static public String getMemResource(int num)
	{
		return "MEM" + num;
	}
	public abstract void addShared(String str, ArithValue initval);
	public abstract ArithValue getShared(String res);
}
