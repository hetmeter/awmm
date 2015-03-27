package ags.constraints;

import fsb.utils.Options.debugMechanismType;

public class AtomicPredicateFactory {
	protected static AtomicPredicate makeAtomicPredicate(int lhsLabel,
			int rhsLabel) {
		if(fsb.utils.Options.DEBUG_MECHANISM == fsb.utils.Options.debugMechanismType.ATOMIC_SECTIONS){
		return new AtomicityAtomicPredicate(lhsLabel, rhsLabel);	
		}
		//else if if(fsb.utils.Options.DEBUG_MECHANISM == fsb.utils.Options.debugMechanismType.FENCES){
		return new LocationAtomicPredicate(lhsLabel, rhsLabel);
	}
}
