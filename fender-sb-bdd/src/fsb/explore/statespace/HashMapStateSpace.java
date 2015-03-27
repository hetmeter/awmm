package fsb.explore.statespace;

import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import fsb.explore.State;
import fsb.utils.Options;
import gnu.trove.map.TMap;
import gnu.trove.map.hash.THashMap;
import gnu.trove.map.hash.TIntObjectHashMap;
import gnu.trove.set.hash.THashSet;

/**
 * @author user
 *
 */
public class HashMapStateSpace extends TIntObjectHashMap<THashMap<State,State>> 
implements StateSpaceInterface{
	
	private int size = 0;
	
	@Override
	public int size() {
		
		return size;
	}
	
	
	public void putInSubSpace(State next) {
		size++;
		try{
			THashMap<State, State> subSpace = this.get(next.hashCode_subs());
			if(null == subSpace){
				subSpace = new THashMap<State, State> ();
				this.put(next.hashCode_subs(), subSpace);
			}
			subSpace.put(next, next);
			}catch(java.lang.IllegalArgumentException e){
				throw e;
			}
	}
	
	public void removeFromSubSpace(State found) {
		this.get(found.hashCode_subs()).remove(found);
		if(this.get(found.hashCode_subs()).isEmpty()){
			this.remove(found.hashCode_subs());
		}
		size--;

	}


	@Override
	public Set<State> putInSubSpaceAndRemoveSubsumed(State next) {

		Set<State> del = Collections.EMPTY_SET;
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
			 del = removeFromSubSpaceAllSubsumedBy(next);
		}
		
		this.putInSubSpace(next);
		
		return del;
	}


	@Override
	public Set<State> removeFromSubSpaceAllSubsumedBy(State next) {
		
		THashSet<State> del = new THashSet<State>();
		TMap<State,State> subSpace = this.get(next.hashCode_subs());
		
		if(null == subSpace){
			return del; 
		}
		
		for(State s: subSpace.keySet()){
			if(s.equals_subsumed(next)){
				del.add(s);
			}
		}
		int del_s = del.size();
		
		for(State s : del){
			this.removeFromSubSpace(s);
		}
		return del;
	}
	
//	/**
//	 * @param space
//	 * @param found
//	 * @param f_chngd
//	 */
//	public void replace_joined2(HashMapStateSpace space, State found, State f_chngd) {
//		TMap<State,State> subSpace = space.get(f_chngd.hashCode_subs());
//		THashSet<State> del = new THashSet<State>();
//		for(State s: subSpace.keySet()){
//			if(s.equals_subs(f_chngd)){
//				del.add(s);
//			}
//		}
//		int del_s = del.size();
//		
//		for(State s : del){
//			space.removeFromSubSpace(s);
//		}
//		space.putInSubSpace(f_chngd);
//	}


	@Override
	public State myFindStateExact(State next) {
		
		Map<State,State> subSpace = this.get(next.hashCode_subs());
		if(subSpace == null){
			return null;
		}
		State ret = subSpace.get(next);

		return ret;
				
	}
	
	@Override
	public Set<State> myFindStateThatSubsume(State next) {
	
	Set<State> ret = new HashSet<State>();
	Map<State,State> subSpace = this.get(next.hashCode_subs());
	if(subSpace == null){
		return null;
	}
	State retState = subSpace.get(next);
	
	if(retState != null){
		ret.add(retState);
	}
	
	if(Options.USE_STATE_SUBSUMPTIUON_LEVEL != -1){
		for(State s: subSpace.keySet()){
			if(Options.MAX_RETURNED_FOR_SUBSUME_IN_ELEM_SEARCH == ret.size()){
				return ret;
				
			}
			if(s.equals_subsumes(next)){
				ret.add(s);
			}
		}
	}
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
