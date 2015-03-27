package fsb.utils;

import fsb.explore.statespace.StateQueue;
import fsb.explore.statespace.StateStack;
import fsb.trie.Trie;

public class Options {
	/**
	 * YM: have no idea what setting it to true does - I think it's deprecated (the written file is usually empty) 
	 */
	public static boolean WRITE_TS = false;
	/**
	 * if {@link #DELETE_STATES_IN_ATOMICS} is true
	 * will be set to false and not keep the TS system.
	 * 
	 */
	public static boolean USE_TS = false;
	
	public static enum StateSpaceStructT{
		TRIE,HASHMAP,BDD
	};
	public static StateSpaceStructT USE_FOR_STATE_SPACE = StateSpaceStructT.HASHMAP;
	
	/*  ***************************
	 * exploration methodology options
	 ******************************/
	public static boolean LAZY_COPY = true;
	/**
	 * sets if the system treats "nop" as local instruction,
	 * meaning via optimization - it is not an interesting 
	 * place for a context switch and checking of assertion breaches
	 */
	public static boolean LOCAL_NOP = false;	
	/**
	 * if the exploration reached and found an error trace,
	 * should it halt? or go on and find&collect all the error states?
	 */
	public static boolean AVOID_EXPLORING_FROM_ERR = true;
	/**
	 * various optimization for the case where all the variables save
	 * the program counter can contain 0,1(or *).<br>
	 * currently the system probably doesn't work when this var is not set to true.
	 */
	public static boolean IS_BOOLEAN_PROGRAM = true;
	/**
	 * an optimization attempt of one point where the long assume
	 * statements were broken down to subsequent short ones.
	 * this might increase the state space unless the connection
	 * of the small assumes is remembered and that the states can be joined again.
	 * 
	 */
	public static boolean BREAK_ASSUMES = false;

	/**
	 * should (*,1) and (1,1) be joined?<br>
	 * should (1,1) and (0,1) be joined?<br>
	 * should (1,1) and (0,0)?<br>
	 */
	public static int USE_STATE_SUBSUMPTIUON_LEVEL = -1;
	
	/**
	 * when set to true will delete states of atomics sections
	 * once those are no longer needed.
	 * Setting it to true should set {@link #USE_TS} 
	 * and {@link #PRINT_FIRST_ERROR_TRACE} to false.
	 * 
	 */
	public static boolean DELETE_STATES_IN_ATOMICS = true;
	
	/**
	 * currently used for the {@link Trie} only,
	 * probably will work together with {@link #USE_STATE_SUBSUMPTIUON_LEVEL}.
	 */
	public static int MAX_RETURNED_FOR_SUBSUME_IN_ELEM_SEARCH = 1;
	
	public static boolean RESET_LOCALS_AT_END_ATOMICS = false;
	
	/**
	 * chooses if the exploration method will be DFS or BFS. <br>
	 * If the states to explore stract will be {@link StateQueue} or {@link StateStack}
	 */
	public static boolean EXPLORATION_METHOD_DFS = true;
	
	/* ***************************
	 * Error trace options
	 *****************************/
	/**
	 * should several error traces be search (note it differs from {@link #AVOID_EXPLORING_FROM_ERR}
	 * in that even once an error state is found several paths might lead to it.
	 */
	public static boolean HALT_AT_FIRST_ERROR_TRACE = true;

	/**
	 * should be set to false if {@link #DELETE_STATES_IN_ATOMICS} is true.
	 * and if {@link #USE_TS} is set to false.
	 */
	public static boolean PRINT_FIRST_ERROR_TRACE = false;
	
	/**relevant only if PRINT_FIRST_ERROR_TRACE is true
	 * set to null if you want to print the error trace only to the screen
	 */
	public static String fileNameForErrorTrace = "errorTrace.tmp";
	
	/**relevant only if PRINT_FIRST_ERROR_TRACE is true
	 * set to true if you don't want to print the statements in error trace only the comments
	 */
	public static boolean PRINT_ONLY_COMMENTS_IN_ERROR_TRACE = true;
	
	/**relevant only if PRINT_FIRST_ERROR_TRACE is true and PRINT_ONLY_COMMENTS_IN_ERROR_TRACE=true
	 * set to true if you want the comments to be printed without spaces and without the states.
	 */
	public static boolean SUCCINT_ERROR_TRACE = true;
	
	/* *****************************
	 * Statistics options
	 ******************************/
	public static boolean PRINT_USED_CUBES = false;
	
	public static boolean PRINT_FOCUS_STATISTICS = false;
	
	public static boolean INVOKE_GC_BEFORE_END_STATISTICS = false;
	
	/**
	 * A finer option of the above PRINT_USED_CUBES which will print cubes 
	 * per statement at the end of the execution
	 */
	public static boolean PRINT_USED_CUBES_PER_STATEMENT = false;
	
	public static boolean PRINT_INTERMEDIATE_MAX_MEM = false;

	/* *****************************
	 * debugging
	 ******************************/
	/**
	 * should be used with {@link #PRINT_STATE_AT_PROCESS}
	 * print the program state each time the execution reaches label 
	 * PRINT_STATE_AT_LABEL at program id PRINT_STATE_AT_PROCESS
	 * controlled from execution line with -trackLabel 
	 */
	public static int PRINT_STATE_AT_LABEL = -1;
	/**
	 * goes with {@link #PRINT_STATE_AT_LABEL}
	 * print the program state each time the execution reaches label 
	 * PRINT_STATE_AT_LABEL at program id PRINT_STATE_AT_PROCESS
	 * controlled from execution line with -trackLabel 
	 */
	public static int PRINT_STATE_AT_PROCESS = -1;
	
	
	/* ***************************
	 * fender synthesis
	 ************************** */
	/**
	set to false if want to synthsize a solution and not just verify
	 */
	public static boolean ONLY_MODEL_CHECKER = true;
	
	/**
	set to true if want to break long assume statements in negation of DNF into
	small assumes. 
		 */
	public static enum debugMechanismType {ATOMIC_SECTIONS,FENCES};
	public static debugMechanismType  DEBUG_MECHANISM = debugMechanismType.ATOMIC_SECTIONS;
	

	
	
	
}
