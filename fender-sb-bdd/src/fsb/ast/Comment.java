package fsb.ast;

import fsb.explore.State;

public class Comment  extends Statement {

		String s;
		
		public Comment(String s) {
			super(StatType.C_COMMENT);
			this.s = s;
		}

		public String toString()
		{
			return s ;
		}


		@Override
		public boolean isLocal() {
			return true;
		}
	

	public String evaluate(State s, int pid) {
		return this.s;
	}
	
}
