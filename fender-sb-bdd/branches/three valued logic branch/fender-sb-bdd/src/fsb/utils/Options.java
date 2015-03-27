package fsb.utils;

public class Options {
	public static boolean AVOID_EXPLORING_FROM_ERR = true;
	public static boolean WRITE_TS = false;
	public static boolean LAZY_COPY = true;
	public static boolean LOCAL_NOP = true;	
	public static boolean PRINT_FIRST_ERROR_TRACE = true;
	
	/**relevant only if PRINT_FIRST_ERROR_TRACE is true
	 * set to null if you want to print the error trace only to the screen
	 */
	public static String fileNameForErrorTrace = "errorTrace.tmp";
	
	/**relevant only if PRINT_FIRST_ERROR_TRACE is true
	 * set to false if you don't want to print the statements in error trace only the comments
	 */
	public static boolean PRINT_ONLY_COMMENTS_IN_ERROR_TRACE = false;
	
	/**
set to false if want to synthsize a solution and not just verify
	 */
	public static boolean ONLY_MODEL_CHECKER = true;
	
	public static enum debugMechanismType {ATOMIC_SECTIONS,FENCES};
	public static debugMechanismType  DEBUG_MECHANISM = debugMechanismType.ATOMIC_SECTIONS;
	
	
	
}
