package fsb.explore;

import fsb.ast.AssertBoolExpr;

public class ExprValidator extends Validator {
	private AssertBoolExpr assertion;
	private boolean alwaysActive;
	
	public ExprValidator(AssertBoolExpr b, boolean alwaysActive)
	{
		assertion = b;
		this.alwaysActive = alwaysActive;
	}
	@Override
	
	public boolean isErr(State s) {
		// YURI: added that assert always is not evaluated during initialization
		return (s.isFinal() || (alwaysActive && s.isInitFinished())) && !assertion.evaluate(s);
	}

}
