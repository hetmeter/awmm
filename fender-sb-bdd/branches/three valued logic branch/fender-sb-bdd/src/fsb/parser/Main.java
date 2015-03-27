package fsb.parser;

import ags.constraints.AtomicPredicateDictionary;
import fsb.ast.MProgram;
import fsb.ast.tvl.ArithValue;
import fsb.explore.Explorer;
import fsb.explore.SBState;
import fsb.explore.State;
import fsb.explore.Validator;
import fsb.semantics.Semantics;
import fsb.semantics.naivepso.NaiveAbsSemantics;
import fsb.semantics.eagerpso.EagerAbsSemantics;
import fsb.semantics.empeagerpso.EmpEagerAbsSemantics;
import fsb.semantics.pso.*;
import fsb.semantics.sc.*;

@SuppressWarnings("unused")
public class Main {

	/**
	 * @param args
	 */
	public static void main(String[] argv) throws java.io.IOException,
			java.lang.Exception 
	{

		Lexer scanner = null;
		Semantics sem = null;

		scanner = getScannerFromInput(argv);
		sem = parseSemantics(argv);
		System.out.println("--- Commencing parsing ---");

		try {
			parser p = new parser(scanner);
			MProgram mp = (MProgram) p.parse().value;
			System.out.println("--- Finished parsing successfully. ---\nPrinting out the parsed program:");
			System.out.println(mp);
			
			//The next line constracts the initial state and adds a validator(that says if a state is an error state) to the run.
			// possible validators are defined below as private static variables.
			//if a validator is not given an assertion is needed at the end of the test.
			//State init = sem.getStateFactory().newState(mp, dekkerCritSec,
				//mp.getProcs());
			State init = sem.getStateFactory().newState(mp, null,
					mp.getProcs());

			Explorer e = new Explorer(sem);
			e.setFileName(argv[0]);
			e.SetInitial(init);
			e.explore();
			return;
		} catch (java.io.IOException e) {
			System.out.println("An I/O error occured while parsing : \n" + e);
			System.exit(1);
		}
	}

	private static Lexer getScannerFromInput(String[] argv) {
		 Lexer scanner = null;
		try {
			scanner = new Lexer(new java.io.FileReader(argv[0]));
		} catch (java.io.FileNotFoundException e) {
			System.out.println("File not found : \"" + argv[0] + "\"");
			System.exit(1);
		} catch (java.io.IOException e) {
			System.out.println("Error opening file \"" + argv[0] + "\"");
			System.exit(1);
		}
		return scanner;
	}

	private static Semantics parseSemantics(String[] argv) {
		Semantics sem = null;
		try {
			if (argv[1].equals("sc"))
				sem = new SCSemantics();
			else if (argv[1].equals("pso"))
				sem = new PSOSemantics(Integer.parseInt(argv[2]));
			else if (argv[1].equals("set"))
				sem = new NaiveAbsSemantics();
			else if (argv[1].equals("must"))
				sem = new EagerAbsSemantics(Integer.parseInt(argv[2]));
			else if (argv[1].equals("may"))
				sem = new EmpEagerAbsSemantics(Integer.parseInt(argv[2]));
			else {
				System.out.println("Invalid argument " + argv[1] + "!");
				System.exit(1);
			}

		} catch (ArrayIndexOutOfBoundsException e) {
			System.out
					.println("Usage : java Main <inputfile> <abstraction> <k>");
			System.out
					.println("Possible values for abstraction: sc/pso/set/must/may");
			System.exit(1);
		}
		return sem;
		
	}
	

	private static Validator alwaysOk = new Validator() {
		public boolean isErr(State s) {
			return false;
		}
	};
	
	private static Validator dekkerCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 13
					&& ((SBState) s).getPC(2) == 13;
		}
	};

	private static Validator peterCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 10
					&& ((SBState) s).getPC(2) == 10;
		}
	};

	private static Validator lamportCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 28
					&& ((SBState) s).getPC(2) == 28;
		}
	};

	private static Validator mcsCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 8
					&& ((SBState) s).getPC(2) == 8;
		}
	};

	private static Validator clhCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 9
					&& ((SBState) s).getPC(2) == 9;
		}
	};

	private static Validator fastCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 22
					&& ((SBState) s).getPC(2) == 22;
		}
	};

	private static Validator senseCritSec = new Validator() {
		public boolean isErr(State s) {
			return ((SBState) s).getPC(1) == 2
					&& ((SBState) s).getPC(2) == 12
					|| ((SBState) s).getPC(1) == 12
					&& ((SBState) s).getPC(2) == 2;
		}
	};

	private static Validator stupidValidator = new Validator() {
		public boolean isErr(State s) {
			return s.isFinal()
					&& (((SBState) s).getLocal(1, "r1").greaterThen( ((SBState) s)
							.getLocal(1, "r2")).or(((SBState) s)
							.getLocal(1, "r2").greaterThen(((SBState) s)
							.getLocal(1, "r3")))).isTrue();
		}
	};

	private static Validator ey1Validator = new Validator() {
		public boolean isErr(State s) {
			SBState sbState = (SBState) s;
			ArithValue local_r1 = sbState.getLocal(1, "r1");
			ArithValue local_r2 = sbState.getLocal(1, "r2");
			ArithValue local_r3 = sbState.getLocal(1, "r3");
			ArithValue local_r4 = sbState.getLocal(1, "r4");
			return s.isFinal()
					&& local_r1.greaterThen(local_r2)
					  .or(local_r2.greaterThen(local_r3))
					  .or(local_r3.greaterThen(local_r4))
					  .isTrue();
		}
	};
	
	private static Validator ey2Validator = new Validator() {
		public boolean isErr(State s) {
			SBState sbState = (SBState) s;
			ArithValue local_r4 = sbState
					.getLocal(1, "r4");
			ArithValue local_r3 = sbState.getLocal(1, "r3");
			return s.isFinal()
					&& local_r3.greaterThen(local_r4).isTrue();
		}
	};

	private static Validator p0Validator = new Validator()
	{
		public boolean isErr(State s)
		{
			ArithValue local0r = ((SBState) s).getLocal(0, "r");
			ArithValue local1r = ((SBState) s).getLocal(1, "r");
			return s.isFinal() && 
			 local0r.eq(2)
			 .and(local1r.eq(1))
			 .and(((SBState) s).getLocal(2, "r").eq(1))
			 .and(((SBState) s).getShared("y").eq(2))
			 .isTrue();
		}
	};

}
