package fsb.parser;


import java.io.File;
import java.io.FilenameFilter;

import junit.framework.JUnit4TestAdapter;
import junit.framework.TestSuite;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.AllTests;

@RunWith(AllTests.class)
public class MainHeavyBenchmarkTest {	
	static public String m_run_parameters = "sc -subsLev 1 -stateSpace HashMap";


	public static void run_benchmark(String path_succ,String path_fail ) throws Exception{
		MainTest.m_run_parameters = MainHeavyBenchmarkTest.m_run_parameters;
		FilenameFilter f_n_filter = new FilenameFilter() {
			public boolean accept(File directory, String fileName) {
			    return fileName.endsWith(".bl");
			}
		};
		File[] files_succ = new File(path_succ).listFiles(f_n_filter);
		for(File f : files_succ){
			MainTest.test(path_succ,f.getName(),"Error states: 0" );
		}
		File[] files_should_fail = new File(path_fail).listFiles(f_n_filter);
		for(File f : files_should_fail){
			MainTest.test(path_fail,f.getName(),"Error states: 1" );
		}
		
	}
	
//	@Test public void abp(){
//		run_benchmark("bp-fender-benchmark/abp-suite/should_succeed/","bp-fender-benchmark/abp-suite/should_fail/");
//	}


//	@Test public  void dekker(){
//		run_benchmark("bp-fender-benchmark/dekker-suite/should_succeed/","bp-fender-benchmark/dekker-suite/should_fail/");
//	
//	}
	
	    public static TestSuite suite() throws Exception
	    {
	    	TestSuite suite = new TestSuite();
	    	String path_succ ="bp-fender-benchmark/dekker-suite/should_succeed/";
	    	String path_fail = "bp-fender-benchmark/dekker-suite/should_fail/";
	    	
	        MainTest.m_run_parameters = MainHeavyBenchmarkTest.m_run_parameters;
			FilenameFilter f_n_filter = new FilenameFilter() {
				public boolean accept(File directory, String fileName) {
				    return fileName.endsWith(".bl");
				}
			};
			File[] files_succ = new File(path_succ).listFiles(f_n_filter);
			for(File f : files_succ){
				TestCase testCase = new TestCase(path_succ, f.getName()) {
					@Override
					@Test
					public void fun() throws Exception{
						MainTest.test(path_s,f_name,"Error states: 0" );
						}
			        };
				suite.addTest(new JUnit4TestAdapter(testCase.getClass()));
			}
			File[] files_should_fail = new File(path_fail).listFiles(f_n_filter);
			for(File f : files_should_fail){
				TestCase testCase = new TestCase(path_fail, f.getName()) {
					@Test
					public void fun()throws Exception{
						MainTest.test(path_s,f_name,"Error states: 1" );
						
					}
				};
				suite.addTest(new JUnit4TestAdapter(testCase.getClass()));
			}
	   
	    	return suite;
	     }
	    
	    public static abstract class TestCase{
	    	protected String path_s;
	    	protected String f_name;
	    	TestCase(String path, String file_name){
	    		path_s = path; 
	    	    f_name = file_name;
	    	}
	    	@Test
	    	public abstract void fun() throws Exception;
	    }
	
}
