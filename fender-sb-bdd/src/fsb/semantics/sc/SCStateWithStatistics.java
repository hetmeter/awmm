/**
 * 
 */
package fsb.semantics.sc;

import java.util.Set;

import fsb.ast.AssertComplexBool;
import fsb.ast.Assume;
import fsb.explore.SBState;
import fsb.semantics.Semantics;
import fsb.tvl.BoolValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;
import fsb.utils.Debug;
import fsb.utils.Options;
import gnu.trove.map.hash.TIntIntHashMap;
import gnu.trove.set.hash.THashSet;

/**
 * @author user
 *
 */
public class SCStateWithStatistics extends SCState {

	static protected TIntIntHashMap statisticsForFocusBeforeGeneralize = new TIntIntHashMap();
	static protected TIntIntHashMap statisticsForFocusAfterGenerelize = new TIntIntHashMap();

	private static int m_ret_size_so_far = 0;

	/**
	 * 
	 */
	public SCStateWithStatistics() {
		// TODO Auto-generated constructor stub
	}

	/**
	 * @param s
	 */
	public SCStateWithStatistics(SCState s) {
		super(s);
		// TODO Auto-generated constructor stub
	}
	
	/**
	 * rudamentrary focus implementation,
	 * assuming the assume is over shared variables solely.
	 * 
	 */
		@Override
		protected THashSet<SBState> inner_focus(Assume assume, int pid,  THashSet<SBState> ret) {
			
			Assume smallerAssume = assume.assignConcreteValuesFromState(this);
			Set<Integer> interesting_variables = assume.getVariablesInvolved();
			if (fsb.utils.Debug.DEBUG_LEVEL > 1) {
				System.out.println("Starting focus with current state, "+interesting_variables.size()+" variables to focus and "
						+ assume );
				m_ret_size_so_far = 0;
			}
			
			//ret = focusOnLocal(assume, pid, ret, interesting_variables);
			ret = focusOnGlobal(smallerAssume, pid, ret, interesting_variables, assume);

			if (fsb.utils.Debug.DEBUG_LEVEL > 0) {
				if(ret.size() > 10){
					int i =0;
					i= i+1;
				}
				statisticsForFocusBeforeGeneralize.putIfAbsent(ret.size(), 0);
				int prev_val = statisticsForFocusBeforeGeneralize.get(ret.size());
				if(prev_val <10000){
				statisticsForFocusBeforeGeneralize.adjustValue(ret.size(), prev_val+1);
				}
				//System.out.println("Finished focus concretization with "
					//	+ ret.size() + " states will try to generalize");
			}
			// now to make it general again
			//ret = generalize2(pid, ret, interesting_variables,assume);
			ret = generalize(pid, ret, interesting_variables);
			if (fsb.utils.Debug.DEBUG_LEVEL > 0) {
				statisticsForFocusAfterGenerelize.putIfAbsent(ret.size(), 0);
				int prev_val = statisticsForFocusAfterGenerelize.get(ret.size());
				if(prev_val <10000){
					statisticsForFocusAfterGenerelize.adjustValue(ret.size(), prev_val+1);
				}
				//System.out.println("Finished focus with " + ret.size() + " states");
			}
			return ret;
		}
		protected THashSet<SBState> generalizeOnGlobal(int pid, THashSet<SBState> ret,
				Set<Integer> interesting_variables) {
			int num_joins_for_curr_var = 0;
			for (int res = 0; res < this.m_shared.size(); ++res) {
				if (!interesting_variables.contains(res)) {
					continue;
				}
				THashSet<SBState> tmp_ret = new THashSet<SBState>();
				// tmp_ret.add(ret.get(0));
				if(Debug.DEBUG_LEVEL>2){
					System.out.println("DEBUG: generalizing for varaible: " + res +
							" statespace " + ret.size());
				}
				for (SBState s : ret) {
					if (!((SCState) s).m_shared.get(res).isDetermined()) {
						if(Debug.DEBUG_LEVEL>2){
									 System.out.println("DEBUG: it is undetermined for curr state");
									 System.out.println("DEBUG: focus generalize on statespace: "
											 + ret.size() + " states");
						}

						tmp_ret.add(s);
						continue;
					}
					SCState new_s = new SCState((SCState) s);
					new_s.m_shared.put(res, s.getShared(res).not());
					if (ret.contains(new_s)) {
						if(Debug.DEBUG_LEVEL>2){
							System.out.println("DEBUG: had both the state and same state "
									+"with the said variable negated var="+res
									+" joint "+ num_joins_for_curr_var++ +" vars"	);
						}
						new_s.m_shared.put(res, NondetArithValue.getInstance());
						if (!tmp_ret.contains(new_s)) {
							tmp_ret.add(new_s);
						}
					} else {
						tmp_ret.add(s);
					}
				}
				ret = tmp_ret;
				if (Debug.DEBUG_LEVEL>1
						&& m_ret_size_so_far - ret.size() > 1000) {
					m_ret_size_so_far = ret.size();
					num_joins_for_curr_var = 0;
					System.out.println("focus generalize on statespace: "
							+ m_ret_size_so_far + " states");
				}
			}

			return ret;
		}
		protected THashSet<SBState> generalizeOnGlobal2(int pid, THashSet<SBState> ret,
				Set<Integer> interesting_variables, Assume assume) {
				THashSet<SBState> tmp_ret = new THashSet<SBState>();
				// tmp_ret.add(ret.get(0));
				for (SBState s : ret) {
					for(int res: interesting_variables){
					if (!((SCState) s).m_shared.get(res).isDetermined()) {
						if(Debug.DEBUG_LEVEL>1){
									 System.out.println("DEBUG: "+fsb.ast.SymbolTable.getGlobalVariableName(res)+" is undetermined for curr state");
									 System.out.println("DEBUG: focus generalize on statespace: "
											 + ret.size() + " states");
						}

						tmp_ret.add(s);
						continue;
					}
					SCState new_s = new SCState((SCState) s);
					new_s.m_shared.put(res, s.getShared(res).not());
					if (assume.cond.evaluate(s).isDetermined()) {
						if(Debug.DEBUG_LEVEL>1){
							System.out.println("DEBUG: had both the state and same state with the said varialbe negated");
						}
						new_s.m_shared.put(res, NondetArithValue.getInstance());
						if (!tmp_ret.contains(new_s)) {
							tmp_ret.add(new_s);
						}
					} else {
						tmp_ret.add(s);
					}
				}
				ret = tmp_ret;
				if (Debug.DEBUG_LEVEL>1
						&& m_ret_size_so_far - ret.size() > 1000) {
					m_ret_size_so_far = ret.size();
					System.out.println("focus generalize on statespace: "
							+ m_ret_size_so_far + " states");
				}
			}

			return ret;
		}
		protected THashSet<SBState> generelizeOnLocal(int pid, THashSet<SBState> ret,
				Set<Integer> interesting_variables) {
			for (Integer res = 0; res < this.locals[pid].size(); res++) {
				if (!interesting_variables.contains(res)) {
					continue;
				}
				THashSet<SBState> tmp_ret = new THashSet<SBState>();
				if(Debug.DEBUG_LEVEL>1){
				 System.out.println("DEBUG: generalizing for varaible: " + res +
				 " statespace " + ret.size());
				}
				 int counter_debug = 0;
				for (SBState s : ret) {
					if (!s.getLocal(pid, res).isDetermined()) {
						if(Debug.DEBUG_LEVEL>1){
									 System.out.println("DEBUG: its is undetermined for curr state");
									 System.out.println("DEBUG: focus generalize on statespace: "
											 + ret.size() + " states");
						}

						tmp_ret.add(s);
						continue;
					}
					if(Debug.DEBUG_LEVEL>1){
						System.out.println(" "+ ++counter_debug);
					}
					SCState new_s = new SCState((SCState) s);
					new_s.setLocal(pid, res, s.getLocal(pid, res).not());
					if (ret.contains(new_s)) {
						// System.out.println("DEBUG: had both the state and same state with the said varialbe negated");

						new_s.setLocal(pid, res, NondetArithValue.getInstance());
						if (!tmp_ret.contains(new_s)) {
							tmp_ret.add(new_s);
						}
					} else {
						tmp_ret.add(s);
					}
				}
				ret = tmp_ret;
				if (Debug.DEBUG_LEVEL>1
						&& m_ret_size_so_far - ret.size() > 1000) {
					m_ret_size_so_far = ret.size();
					System.out.println("focus generalize on statespace: "
							+ m_ret_size_so_far + " states");
				}
			}
			return ret;
		}
		private THashSet<SBState> focusOnGlobal(Assume assume, int pid,
				THashSet<SBState> ret, Set<Integer> interesting_variables, Assume origAssume) {
			boolean fixpoint = true;
			THashSet<SBState> concrete = new THashSet<SBState>();
			for (int sharedLbl = 0; sharedLbl < this.m_shared.size(); ++sharedLbl) {// was made a pass through all the shared and not only the interesting variables,
				if (this.m_shared.get(sharedLbl).isDetermined() // with the intention of maybe using the local variables too
						|| !interesting_variables.contains(sharedLbl))
					continue;
				// int sharedLbl = e.getKey();
				THashSet<SBState> tmp_ret = new THashSet<SBState>();
				fixpoint = true;
				for (SBState s : ret) {

					SCState newstate1 = new SCState((SCState) s);
					SCState newstate2 = new SCState((SCState) s);
					newstate1.m_shared.put(sharedLbl, DetArithValue.getInstance(1));
					newstate2.m_shared.put(sharedLbl, DetArithValue.getInstance(0));
					BoolValue newState1Assume = assume.cond.evaluate(newstate1);
					BoolValue newState2Assume = assume.cond.evaluate(newstate2);

					if (!newState1Assume.isFalse()){
						if(newState1Assume.isDetermined()){
							if(Options.PRINT_USED_CUBES){
								Semantics.addAssertCubesToSet(origAssume.cond.allSatisfiedCubes(newstate1),
										origAssume.cond.allCubes(),Semantics.cubesAssertTrueTogetherMap);
								AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allSatisfiedCubes(newstate1));
								AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
							}
							concrete.add(newstate1);
						}else{
							tmp_ret.add(newstate1);
						}
					}else{
						if(Options.PRINT_USED_CUBES){
							Semantics.addAssertCubesToSet(origAssume.cond.allNegationSatisfiedCubes(newstate1),
									origAssume.cond.allCubes(),Semantics.cubesAssertFalseTogetherMap);
							AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allNegationSatisfiedCubes(newstate1));
							AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
						}
					}
					if (!newState2Assume.isFalse()){
						if(newState2Assume.isDetermined()){
							if(Options.PRINT_USED_CUBES){
								Semantics.addAssertCubesToSet(origAssume.cond.allSatisfiedCubes(newstate2),
										origAssume.cond.allCubes(),Semantics.cubesAssertTrueTogetherMap);
								AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allSatisfiedCubes(newstate2));
								AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
							}
							concrete.add(newstate2);
						}else{
							tmp_ret.add(newstate2);
						}
					}else{
						if(Options.PRINT_USED_CUBES){
							Semantics.addAssertCubesToSet(origAssume.cond.allNegationSatisfiedCubes(newstate2),
									origAssume.cond.allCubes(),Semantics.cubesAssertFalseTogetherMap);
							AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allNegationSatisfiedCubes(newstate2));
							AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
						}
					}
					if (!newState1Assume.isDetermined()
							|| !newState2Assume.isDetermined()) {
						// then we have put a state for which the assume is "*", so
						// no fixpoint yet and we have to continue
						fixpoint = false;
					}
				}
				ret = tmp_ret;
				if (Debug.DEBUG_LEVEL>1
						&& m_ret_size_so_far - ret.size() < -1000) {
					m_ret_size_so_far = ret.size();
					System.out.println("focus generalize on statespace: "
							+ m_ret_size_so_far + " states");
				}
				if (fixpoint)
					break;
				//generelizeOnGlobal(pid, tmp_ret, interesting_variables);

			}
			if (!fixpoint) {
				throw new RuntimeException(
						"We made the state concrete and still the assume is undetermined");
			}
			ret.addAll(concrete);
			return ret;
		}
		@SuppressWarnings("unused")
		private THashSet<SBState> focusOnLocal(Assume assume, int pid,
				THashSet<SBState> ret, Set<Integer> interesting_variables) {
			for (int localLbl = 0; localLbl < this.locals[pid].size(); ++localLbl) {
				if (this.locals[pid].get(localLbl).isDetermined()
						|| !interesting_variables.contains(localLbl))
					continue;
				THashSet<SBState> tmp_ret = new THashSet<SBState>();
				boolean fixpoint = true;
				for (SBState s : ret) {

					SBState newstate1 = new SCState((SCState) s);
					SBState newstate2 = new SCState((SCState) s);
					newstate1.setLocal(pid, localLbl, DetArithValue.getInstance(1));
					newstate2.setLocal(pid, localLbl, DetArithValue.getInstance(0));
					BoolValue newState1Assume = assume.cond.evaluate(newstate1);
					BoolValue newState2Assume = assume.cond.evaluate(newstate2);

					if (!newState1Assume.isFalse())
						tmp_ret.add(newstate1);
					if (!newState2Assume.isFalse())
						tmp_ret.add(newstate2);
					if (!newState1Assume.isDetermined()
							|| !newState2Assume.isDetermined()) {
						// then we have put a state for which the assume is "*", so
						// no fixpoint yet and we have to continue
						fixpoint = false;
					}
					// debugging printouts
					
				}
				ret = tmp_ret;
				if (!fsb.utils.Options.SUCCINT_ERROR_TRACE
						&& m_ret_size_so_far - ret.size() < -1000) {
					m_ret_size_so_far = ret.size();
					System.out.println("focus on statespace: " + m_ret_size_so_far
							+ " states");
				}
				if (fixpoint)
					break;
				//generelizeOnLocal(pid, tmp_ret, interesting_variables);
			}
			return ret;
		}
		public static void printFocusStatistics(){
			System.out.println("statisticsForFocusBeforeGeneralize (value,times) "+statisticsForFocusBeforeGeneralize);
			System.out.println("statisticsForFocusAfterGenerelize (value,times) "+statisticsForFocusAfterGenerelize);
		}


}
