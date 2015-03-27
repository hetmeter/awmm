package fsb.ast;

public abstract class Statement {
	public enum StatType {LOAD, STORE, BARRIER, BRANCH, ASSIGN, ALLOCATE, END, NOP, CAS, SWAP, ATOMICDEC, BEGINATOMIC, ENDATOMIC, ASSUME, CHOOSE_ASSIGN, CHOOSE_STORE, C_COMMENT};	
	StatType type;
	int label;	
	
	public Statement(StatType type)
	{
		this.type = type;
	}
	
	public void setLabel(Integer label) {
		this.label = label;
	}
	
	public int getLabel()
	{
		return label;
	}

	public boolean isLast() {
		return false;
	}

	public StatType getType() {
		// TODO Auto-generated method stub
		return type;
	}

	public abstract boolean isLocal();
}
