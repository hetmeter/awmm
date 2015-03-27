package fsb.explore.statespace;

import java.util.Set;

import fsb.ast.SymbolTable;
import fsb.explore.State;
import fsb.semantics.sc.SCState;
import fsb.trie.Trie;
import fsb.trie.TrieInterface;
import fsb.utils.Options;
import gnu.trove.map.hash.TIntObjectHashMap;

public class TrieMapStateSpace extends TIntObjectHashMap<Trie<State>> implements StateSpaceInterface{
	private static int m_num_bits = -1;
	private int size = 0;
	public enum optimization_level_t {NONE,OPT1,OPT2,OPT3};
	public static optimization_level_t OPTIMIZATION_LEVEL = optimization_level_t.OPT3;
	/* (non-Javadoc)
	 * @see fsb.explore.statespace.StateSpaceInterface#size()
	 */
	@Override
	public int size() {
		
		return size;
	}
	

	public TrieMapStateSpace(){
		super();
		
		
		
		m_num_bits = (State.program.getProcs()+1) * State.program.getShared().size();
		
		Trie.cleanup(Options.MAX_RETURNED_FOR_SUBSUME_IN_ELEM_SEARCH, m_num_bits);
	}
	
	/* (non-Javadoc)
	 * @see fsb.explore.statespace.StateSpaceInterface#putInSubSpaceAndRemoveSubsumed(fsb.explore.State)
	 */
	@Override
	public Set<State> putInSubSpaceAndRemoveSubsumed(State next) {
		size++;
		
		Trie<State> subSpace = this.get(next.hashCode_subs());
		if(null == subSpace){
			
			subSpace = new Trie<State> (m_num_bits);
			this.put(next.hashCode_subs(), subSpace);
		}
		Set<State> ret;
		switch(OPTIMIZATION_LEVEL){
		case NONE: {
			ret = subSpace.addAndRemoveSubsumed(next.getKeyString(), next);
			break;
		}
		case OPT1: {
			ret = subSpace.optimization1AddAndRemoveSubsumed(next.getKeyString(), next);
			break;
		}
		case OPT2: {
			ret = subSpace.optimization2AddAndRemoveSubsumed(next.getKeyBitSet(), 0,0,next);
			break;
		}

		case OPT3: {
			ret = subSpace.optimization3AddAndRemoveSubsumed(next.getKeyBitSet(), next);
			break;
		}
		default:{
			throw new RuntimeException("wrong  level");
			}
		}
		size -= ret.size();
		return ret;
		
	}
	/* (non-Javadoc)
	 * @see fsb.explore.statespace.StateSpaceInterface#removeFromSubSpaceAllSubsumedBy(fsb.explore.State)
	 */
	@Override
	public Set<State> removeFromSubSpaceAllSubsumedBy(State found) {
		Set<State> s2;
		switch(OPTIMIZATION_LEVEL){
		case NONE: {
			s2 =  this.get(found.hashCode_subs()).deleteSubsumedBy(found.getKeyString());
			break;
		}
		case OPT1: {
			s2 =  this.get(found.hashCode_subs()).optimization1DeleteSubsumedBy(found.getKeyString());
			break;
		}
		case OPT2: {
			s2 =  this.get(found.hashCode_subs()).optimization2DeleteSubsumedBy(found.getKeyBitSet(),0,0);
			break;
		}
		case OPT3: {
			s2 =  this.get(found.hashCode_subs()).optimization3DeleteSubsumedBy(found.getKeyBitSet());
			break;
		}
		default:{
			throw new RuntimeException("wrong  level");
			}
		}
		if(this.get(found.hashCode_subs()).isEmpty()){
			this.remove(found.hashCode_subs());
		}
		size-= s2.size();
		return s2;

	}

	@Override
public Set<State> myFindStateThatSubsume(State next) {
	
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != 1){
			throw new RuntimeException("currently cannot run without Options.USE_STATE_SUBSUMPTIUON_LEVEL = 1");
		}
		
	TrieInterface<State> subSpace = this.get(next.hashCode_subs());
	if(subSpace == null){
		return null;
	}
	Set<State> ret;
	switch(OPTIMIZATION_LEVEL){
	case NONE: {
		ret = subSpace.allThatSubsume(next.getKeyString());
		break;
	}
	case OPT1: {
		ret = subSpace.optimization1AllThatSubsume(next.getKeyString());
		break;
	}
	case OPT2: {
		ret = subSpace.optimization2AllThatSubsume(next.getKeyBitSet(),0,0);
		break;
	}

	case OPT3: {
		ret = subSpace.optimization3AllThatSubsume(next.getKeyBitSet());
		break;
	}
	default:{
		throw new RuntimeException("wrong  level");
		}
	}
	//ret.addAll(subSpace.allSubsumedBy(next.getKeyString()));
	
	int ret_s = ret.size();
	if(ret_s > 1){
		throw new RuntimeException("currently should find only upto 1 state");
	}
		
//	if(ret == null && Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
//		for(State s: subSpace.keySet()){
//			if(s.equals_subs(next)){
//				return s;
//			}
//		}
//	}
//	return ret;
	return ret;
			
	}
	
	/**
	 * @param space
	 * @param next
	 * @return
	 */
	@Deprecated
	private Set<State> myFindStateSubsumesOrSubsumed2(TrieMapStateSpace space, State next) {
	
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != 1){
			throw new RuntimeException("currently cannot run without Options.USE_STATE_SUBSUMPTIUON_LEVEL = 1");
		}
		
	TrieInterface<State> subSpace = space.get(next.hashCode_subs());
	if(subSpace == null){
		return null;
	}
	Set<State> ret = subSpace.allSubsumedBy(next.getKeyString());
	ret.addAll(subSpace.allThatSubsume(next.getKeyString()));
	
	int ret_s = ret.size();
		
//	if(ret == null && Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
//		for(State s: subSpace.keySet()){
//			if(s.equals_subs(next)){
//				return s;
//			}
//		}
//	}
//	return ret;
	ret = subSpace.allSubsumedBy(next.getKeyString());
	ret.addAll(subSpace.allThatSubsume(next.getKeyString()));
	return ret;
			
	}
	/**
	 * @param space
	 * @param next
	 * @return
	 */
	@Override
	public State myFindStateExact(State next) {
	
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != 1){
			throw new RuntimeException("currently cannot run without Options.USE_STATE_SUBSUMPTIUON_LEVEL = 1");
		}
		
	TrieInterface<State> subSpace = this.get(next.hashCode_subs());
	if(subSpace == null){
		return null;
	}
	State ret ;
	switch(OPTIMIZATION_LEVEL){
	case NONE: {
		ret = subSpace.findExact(next.getKeyString());
		break;
	}
	case OPT1: {
		ret = subSpace.optimization1FindExact(next.getKeyString());
		break;
	}
	case OPT2: {
		ret = subSpace.optimization2FindExact(next.getKeyBitSet(),0,0);
		break;
	}
	case OPT3: {
		ret = subSpace.optimization3FindExact(next.getKeyBitSet());
		break;
	}
	default:{
		throw new RuntimeException("wrong  level");
		}
	}
		
//	if(ret == null && Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
//		for(State s: subSpace.keySet()){
//			if(s.equals_subs(next)){
//				return s;
//			}
//		}
//	}
//	return ret;
		return ret;
			
	}
	
	@Override
	public State myFindStateExactOrSubsume(State next) {
		State ret = myFindStateExact(next);
		if(null == ret){
			Set<State>  r2 = myFindStateThatSubsume(next);
			if(null != r2 && !r2.isEmpty()){
				ret = r2.iterator().next();
			}
		}
		return ret;
	}
}
