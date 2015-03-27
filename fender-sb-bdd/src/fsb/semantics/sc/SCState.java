package fsb.semantics.sc;


import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.WeakHashMap;

import ags.constraints.AtomicPredicateDictionary;
import ags.constraints.Formula;
import fsb.ast.AssertBoolExpr;
import fsb.ast.AssertComplexBool;
import fsb.ast.AssertNotBool;
import fsb.ast.Assume;
import fsb.ast.Statement;
import fsb.explore.Action;
import fsb.explore.SBState;
import fsb.explore.State;
import fsb.explore.StateVariables;
import fsb.semantics.Semantics;
import fsb.tvl.ArithValue;
import fsb.tvl.BoolValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;
import fsb.utils.Options;
import gnu.trove.map.TMap;
import gnu.trove.map.hash.THashMap;
import gnu.trove.set.hash.THashSet;

/**
 * TODO: this class should implement join and a different equality function
 * for the binary program option.
 * the equality should be a one way subset relation.
 * a \equals b
 * if a\subset b or b\subset a
 * 
 * the join should take two states and return the overapproximation of both.
 * Q: what about (1,0) (1,1) are they equal? they can be joined into one state..
 * A: (maybe) No. since joining them will cause to lose precision but not gain information.
 * and will not reduce the amount of information to be calculated.
 * 
 * 
 */public class SCState extends SBState {
	//protected class SharedResMap extends TreeMap<String, ArithValue> {}
//	protected class SharedResMap extends StateVariables{
//	
//		public SharedResMap(int sizeLocals) {
//			super(sizeLocals,true);
//			// TODO Auto-generated constructor stub
//		}
//		public SharedResMap(SharedResMap other) {
//			super(other);
//		}
//		public SharedResMap(StateVariables other) {
//			super(other);
//		}
//		@Override
//		public SharedResMap clone() {
//			return new SharedResMap(this);
//		}
//	}
	
	//protected static THashMap<SharedResMap,SharedResMap> m_cachedShared = new THashMap<SharedResMap,SharedResMap>();
	protected static WeakHashMap<StateVariables,StateVariables> m_cachedShared = new WeakHashMap<StateVariables,StateVariables>();
	protected StateVariables m_shared;
	// protected Formula avoidFormula = null;
	// protected Set<Action> incoming = new HashSet<Action>();
	// protected Set<Action> outgoing = new HashSet<Action>();
	/**
	 * this field shoulnd't be static,
	 * it's a hack to reduce statespace,
	 * because for s
	 */
	//protected int lastMem;

	public SCState() {
		super();
		List<Integer> program_shared_vars = program.getShared();
		this.m_shared = new StateVariables(program_shared_vars.size(),true);
		for (int var : program_shared_vars)
			addShared(var, DetArithValue.getInstance(0));
	}

	public void registerAction(Action in) {
		// this.inComing.add(in);
		// ((SCState) (in.getSource())).out_going.add(in);
	}

	public SCState(SCState s) {
		super(s);
		this.m_shared = s.m_shared;
		

		// if (!Options.LAZY_COPY)
		{
			this.m_shared = new StateVariables(s.m_shared.size(),true);
			this.m_shared.putAll(s.m_shared);
		}

	}

	@Override
	public State copy(){
		return new SCState(this);
	}
	
	@Override
	public boolean equals(Object obj) {

		if (!super.equals(obj))
			return false;

		if (!(obj instanceof SCState))
			return false;
		SCState other = (SCState) obj;

		if (!m_shared.equals(other.m_shared))
			return false;
		
		if(!(this.assumeFailed == other.assumeFailed))
			return false;
		
		if(!(this.inAtomicSection == other.inAtomicSection))
			throw new RuntimeException("two states that should be equal one in atomic one is not" + this + other);
		
		if(!(this.bad == other.bad))
			throw new RuntimeException("two states that should be equal one is bad one is not" + this + other);
		return true;
	}

	/**
	 * this >= obj
	 */
	@Override
	public boolean equals_subsumes(Object obj) {
		if (!super.equals_subsumes(obj))
			return false;

		if (!(obj instanceof SCState))
			return false;
		SCState other = (SCState) obj;

		if (!m_shared.equals_subsumes(other.m_shared))
			return false;
		
		if(!(this.assumeFailed == other.assumeFailed))
			return false;
		
		//sanity checks
		if(!(this.inAtomicSection == other.inAtomicSection))
			throw new RuntimeException("two states that should be equal one in atomic one is not" + this + other);
		
		if(!(this.bad == other.bad))
			throw new RuntimeException("two states that should be equal one is bad one is not" + this + other);
		
		return true;
	}
/**
 * this <= obj
 */
	@Override
	public boolean equals_subsumed(Object obj) {
		if (!super.equals_subsumed(obj))
			return false;

		if (!(obj instanceof SCState))
			return false;
		SCState other = (SCState) obj;

		if (!m_shared.equals_subsumed(other.m_shared))
			return false;
		
		if(!(this.assumeFailed == other.assumeFailed))
			return false;
		
		if(!(this.inAtomicSection == other.inAtomicSection))
			throw new RuntimeException("two states that should be equal one in atomic one is not" + this + other);
		
		if(!(this.bad == other.bad))
			throw new RuntimeException("two states that should be equal one is bad one is not" + this + other);
		return true;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + m_shared.hashCode();
		result = prime * result + inAtomicSection;
		result = prime * result + ((assumeFailed)?1:0);
		return result;
	}

	@Override
	public int hashCode_subs() {
		
			final int prime = 31;
		int	hashVal_subs =  super.hashCode_subs();
			//result = prime * result + m_shared.hashCode();
			hashVal_subs = prime * hashVal_subs + inAtomicSection;
			hashVal_subs = prime * hashVal_subs + ((assumeFailed)?1:0);
		
		return hashVal_subs;
	}
	
	public String toString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<Shared:" + m_shared.toString());
		for (int i = 0; i < numProcs; i++) {
			sb.append(",<pid" + i + ", " + "PC="
					+ program.getListing(i).get(pc[i]).getLabel() + ", ");
			sb.append(locals[i].toString() + "> ");
		}
		sb.append(">\n");

		return sb.toString();
	}

	public boolean isFinal() {
		for (int i = 0; i < numProcs; i++)
			if (program.getListing(i).get(pc[i]).isLast() == false)
				return false;

		return true;
	}

	@Override
	public void addShared(int shared_var, ArithValue initval) {
		StateVariables shared_clone = m_shared.clone(); 
		shared_clone.put(shared_var, initval);
		StateVariables cached = m_cachedShared.get(shared_clone);
		try{
		if (cached == null){
			m_cachedShared.put(shared_clone, shared_clone);
			m_shared = shared_clone;			
		}else
			m_shared = cached;
		} catch (java.lang.IllegalArgumentException e){
			for(StateVariables c : m_cachedShared.keySet()){
				System.out.println(c);
				System.out.println(" " + c.hashCode() + " \n");
				System.out.println(" " + (c.equals(m_shared)?"":"not ") + "equals to m_shared");
				System.out.println(" hashCode " + ((c.hashCode()== m_shared.hashCode())?"":"not ") + "equals to m_shared hashCode\n");
			}
			System.out.println("------------------------------------");
			System.out.println(m_shared);
			System.out.println(" " + m_shared.hashCode() + " \n");
			cached = m_cachedShared.get(m_shared);
			throw e;
		}
	}

	@Override
	public ArithValue getShared(int res) {
		return m_shared.get(res);
	}

	public void setShared(int res, ArithValue val) {
		StateVariables shared_clone = m_shared.clone(); 
		shared_clone.put(res, val);
		StateVariables cached = m_cachedShared.get(shared_clone);
		if (cached == null){
			m_cachedShared.put(shared_clone, shared_clone);
			m_shared = shared_clone;
		}else{	
			if(cached.get(res) != val){
				throw new RuntimeException("cach variables error in SCState.setShared");
			}
			m_shared = cached;
			
		}

	}

	@Override
	public boolean disjPredicates() {

		return false;
	};

	/**
	 * given an action coming <b>out</b> of the current state returns a formula
	 * to avoid it.
	 */
	@Override
	public Formula getAvoidFormula(Action taken) {

		// this is temporary hack to define it when there's no field [EY]
		Formula avoidFormula = null;

		if (avoidFormula != null)
			return avoidFormula;

		Formula model = Formula.makeConjunction();

		Map<Action, Action> alternatives = CalculateAlternatives(taken);
		Statement st = taken.getStatement();
		int taken_pid = taken.getPid();

		if (st == null || taken_pid == -1 || inAtomicSection() != -1)// inAtomicSection
																		// != -1
																		// since
																		// if in
																		// atomic
																		// section
																		// not
																		// possible
																		// to
																		// prevent
			return avoidFormula = model;
		if (alternatives.isEmpty()) {
			return avoidFormula = Formula.falseValue();
		}

		for (Map.Entry<Action, Action> entrya : alternatives.entrySet()) {
			Action prev_action = entrya.getKey();
			Action couple_with_action = entrya.getValue();
			SCState predecessor = (SCState) prev_action.getSource();

			Formula avoid_predecessor_formula = predecessor
					.getAvoidFormula(prev_action);
			if (couple_with_action == null) {
				// entering arc with same pid as then one to avoid
				// only option is to try to avoid the predessesor
				model = model.and(avoid_predecessor_formula);
			} else {
				Formula atomicity_pred = AtomicPredicateDictionary
						.makeAtomicPredicate(prev_action.getStatement()
								.getLabel(), couple_with_action.getStatement()
								.getLabel());

				// avoiding the transition to this state(atomicity_pred) or the
				// previous state.
				model = model.and(atomicity_pred.or(avoid_predecessor_formula));
			}
			// model.add(AtomicPredicateDictionary.makeAtomicPredicate(buf.writingLabel,
			// taken.getStatement().getLabel()));

		}

		return avoidFormula = model;
	}

	@Override
	public Set<Formula> getPredicates(Action taken) {
		return null;
	}

	/**
	 * calculates all the possible coupling(atomic section placements) of action
	 * that could of been done to prevent the current transition.<br>
	 * if one of the incoming actions belongs to a thread with the same pid as
	 * the one to be prevented, it is coupled with null !
	 * 
	 * @param predState
	 * @param taken
	 * @return
	 */
	private TMap<Action, Action> CalculateAlternatives(Action taken) {
		// TODO Auto-generated method stub
		TMap<Action, Action> coupling = new THashMap<Action, Action>();

		// temporary hack [EY]
		Set<Action> inComing = new HashSet<Action>();

		for (Action into_predecessor : inComing) {
			Set<Action> coupleWith = new HashSet<Action>();
			int out_action_pid = into_predecessor.getPid();
			if (out_action_pid != taken.getPid()) {
				// revive later [EY]
				// coupleWith = getActionsWithSamePid(out_going,
				// out_action_pid);

				for (Action partner : coupleWith) {
					coupling.put(into_predecessor, partner);
				}
			} else { // couples the outFromPredecessor with null
				coupling.put(into_predecessor, null);
			}
		}
		return coupling;
	}

	// private Set<Action> getActionsWithSamePid(
	// Set<Action> potential_to_couple_with, int pid) {
	// Set<Action> mates = new HashSet<Action>();
	//
	// for (Action a : potential_to_couple_with) {
	// if (a.getPid() == pid) {
	// mates.add(a);
	// }
	// }
	// return mates;
	// }
/**
 * rudamentrary focus implementation,
 * assuming the assume is over shared variables solely.
 * 
 */
//	@Override
//	public THashSet<SBState> focus(Assume assume, int pid) {
//		THashSet<SBState> ret = Collections.EMPTY_LIST;
//		if(assume.cond.isNegatedDNF()){
//		Set<AssertBoolExpr> cubes = assume.cond.allCubes();
//		for(AssertBoolExpr cube: cubes){
//			
//			AssertBoolExpr neg_cube = new AssertNotBool(cube) ;
//			Assume assume_of_cube = new Assume(neg_cube);
//				
//			ret = inner_focus(assume_of_cube,pid);
//		}
//		
//		}else{
//			ret = inner_focus(assume,pid);
//		}
//		
//		return ret;
//	}

	@Override
	public List<SBState> focus(Assume assume, int pid) {
		THashSet<SBState> ret = new THashSet<SBState>();
		ret.add(new SCState(this));
		
		Assume partialy_assigned_assume = assume.assignConcreteValuesFromState(this);
		
		if(fsb.utils.Options.BREAK_ASSUMES==true && partialy_assigned_assume.cond.isNegatedDNF()){
			Set<AssertBoolExpr> cubes = partialy_assigned_assume.cond.allCubes();
			System.out.println("DEBUG: breaking the assume into " + cubes.size() + " smaller assumes ");
			for(AssertBoolExpr cube: cubes){
				AssertBoolExpr neg_cube = new AssertNotBool(cube) ;
				Assume assume_of_cube = new Assume(neg_cube);
				System.out.println("DEBUG: assume " + assume_of_cube + " number of states " + ret.size());
				ret = inner_focus(assume_of_cube,pid, ret);
			}
		
		}else{
			
			ret =  inner_focus(partialy_assigned_assume,pid, ret);
		}
		
		return new ArrayList<SBState>(ret);
	}
	protected THashSet<SBState> inner_focus(Assume assume, int pid, THashSet<SBState> ret) {
		
		Assume partialy_assigned_assume = assume.assignConcreteValuesFromState(this);
		//Should be made a set
		Set<Integer> interesting_variables = assume.getVariablesInvolved();
		
		//ret = focusOnLocal(assume, pid, ret, interesting_variables);
		ret = focusOnGlobal(partialy_assigned_assume, pid, ret, interesting_variables, assume);

		// now to make it general again
		ret = generalize(pid, ret, interesting_variables);
		//ret = generalize2(pid, ret, interesting_variables, assume);

	
		return ret;
	}
	
	protected THashSet<SBState> generalize(int pid, THashSet<SBState> ret,
			Set<Integer> interesting_variables) {
		//ret = generelizeOnLocal(pid, ret, interesting_variables);

		return generalizeOnGlobal(pid, ret, interesting_variables);
	}
	
	
	protected THashSet<SBState> generalize2(int pid, THashSet<SBState> ret,
			Set<Integer> interesting_variables, Assume assume) {
		//ret = generelizeOnLocal(pid, ret, interesting_variables);

		return generalizeOnGlobal2(pid, ret, interesting_variables, assume);
	}

	protected THashSet<SBState> generalizeOnGlobal(int pid, THashSet<SBState> ret,
			Set<Integer> interesting_variables) {
		for (int res = 0; res < this.m_shared.size(); ++res) {
			if (!interesting_variables.contains(res)) {
				continue;
			}
			THashSet<SBState> tmp_ret = new THashSet<SBState>();
			// tmp_ret.add(ret.get(0));
			for (SBState s : ret) {
				if (!((SCState) s).m_shared.get(res).isDetermined()) {
					tmp_ret.add(s);
					continue;
				}
				SCState new_s = new SCState((SCState) s);
				new_s.m_shared.put(res, s.getShared(res).not());
				if (ret.contains(new_s)) {
					new_s.m_shared.put(res, NondetArithValue.getInstance());
					if (!tmp_ret.contains(new_s)) {
						tmp_ret.add(new_s);
					}
				} else {
					tmp_ret.add(s);
				}
			}
			ret = tmp_ret;
		}

		return ret;
	}
	protected THashSet<SBState> generalizeOnGlobal2(int pid, THashSet<SBState> ret,
			Set<Integer> interesting_variables, Assume assume) {
		for (int res = 0; res < this.m_shared.size(); ++res) {
			if (!interesting_variables.contains(res)) {
				continue;
			}
			THashSet<SBState> tmp_ret = new THashSet<SBState>();
			// tmp_ret.add(ret.get(0));
			for (SBState s : ret) {
				if (!((SCState) s).m_shared.get(res).isDetermined()) {
					tmp_ret.add(s);
					continue;
				}
				SCState new_s = new SCState((SCState) s);
				new_s.m_shared.put(res, s.getShared(res).not());
				if (ret.contains(new_s)) {
					new_s.m_shared.put(res, NondetArithValue.getInstance());
					if (!tmp_ret.contains(new_s)) {
						tmp_ret.add(new_s);
					}
				} else {
					tmp_ret.add(s);
				}
			}
			ret = tmp_ret;
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
			 for (SBState s : ret) {
				if (!s.getLocal(pid, res).isDetermined()) {
					tmp_ret.add(s);
					continue;
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
		}
		return ret;
	}

	private THashSet<SBState> focusOnGlobal(Assume assume, int pid,
			THashSet<SBState> ret, Set<Integer> interesting_variables, Assume origAssume) {
		boolean fixpoint = true;
		THashSet<SBState> concrete = new THashSet<SBState>();
		for (int sharedLbl = 0; sharedLbl < this.m_shared.size(); ++sharedLbl) {
			if (this.m_shared.get(sharedLbl).isDetermined()
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
				BoolValue newState1Assume = assume.cond.evaluate_partVisb(newstate1);
				BoolValue newState2Assume = assume.cond.evaluate_partVisb(newstate2);

				if (!newState1Assume.isFalse()){
					if(newState1Assume.isDetermined()){
						if(Options.PRINT_USED_CUBES){
							//assume1 is true
							Semantics.addAssertCubesToSet(origAssume.cond.allSatisfiedCubes(newstate1),
									origAssume.cond.allCubes(),Semantics.cubesAssertTrueTogetherMap);
							AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allSatisfiedCubes(newstate1));
							AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
						}
						concrete.add(newstate1);
					}else{
						tmp_ret.add(newstate1);
					}
				}else{//assume1 is false
					if(Options.PRINT_USED_CUBES){
						Semantics.addAssertCubesToSet(origAssume.cond.allNegationSatisfiedCubes(newstate1),
								origAssume.cond.allCubes(),Semantics.cubesAssertFalseTogetherMap);
						AssertComplexBool.m_usefullCubesGlobal.addAll(origAssume.cond.allNegationSatisfiedCubes(newstate1));
						AssertComplexBool.m_allCubesGlobal.addAll(origAssume.cond.allCubes());
					}
				}
				if (!newState2Assume.isFalse()){
					if(newState2Assume.isDetermined()){//assume2 is true
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
				}else{//assume2 is false
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
			
			if (fixpoint)
				break;
			//generelizeOnGlobal(pid, tmp_ret, interesting_variables);

		}
		if (!fixpoint) {
			String str = "Interesting vars = " + interesting_variables + "\n";
			str += assume + "\n";
			for(State s : ret){
				BoolValue stateAssume = assume.cond.evaluate_partVisb(s);
				if(!stateAssume.isDetermined()){
					str+=s;
				}
			}
				
			throw new RuntimeException(
					"We made the state concrete and still the assume is undetermined\n" + str);
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
			if (fixpoint)
				break;
			//generelizeOnLocal(pid, tmp_ret, interesting_variables);
		}
		return ret;
	}
	
	
	/**
	 * performing a join of the global variables 
	 * if {@link Options#USE_STATE_SUBSUMPTIUON_LEVEL} is true 
	 */
	@Override
	public boolean join(State other_s) {
			// TODO Auto-generated method stub

		Boolean ret = super.join(other_s);
		
		if(Options.USE_STATE_SUBSUMPTIUON_LEVEL == -1){
			return ret;
		}
		
		if(! (other_s instanceof SCState)){
			throw new RuntimeException("joining an SCState with something that is not an SCState how did that happen?");
		}
		
		SCState other = (SCState)other_s; 
		
		StateVariables joined = this.m_shared.join(other.m_shared);
		if(joined != null){
			ret = true;
			m_shared = new StateVariables(joined);
		}
		
		
		return ret;
	}
	
	@Override
	public String getKeyString(){
		String ret = super.getKeyString();
		
		ret = this.m_shared.getKeyString() + "  " + ret;
		
		return ret;
	}

	public static State topOfSet(Set<State> foundSet) {
		if(null == foundSet){
			throw new RuntimeException(" topOfSet conot except a null set");
		}
		int sizeSet = foundSet.size();
		Iterator<State> it = foundSet.iterator();
		if(!it.hasNext()){
			throw new RuntimeException(" topOfSet conot except an empty set");
		}
		State top = it.next().copy();
		while(it.hasNext()){
			top.join(it.next());
		}
		return top;
	}
	
	@Override
	public StateVariables[] getKeyBitSet(){
		StateVariables[] res = new StateVariables[super.locals.length+1];
		res[0] = m_shared;
		for(int i=0;i<locals.length;++i){
			res[i+1] = locals[i];
		}
		return res;
	}

	public void resetLocal(int pid) {
		locals[pid] = new StateVariables(locals[pid]);
		locals[pid].reset();
		
	}

}
