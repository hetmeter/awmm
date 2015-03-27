package fsb.parser;

import static org.junit.Assert.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.PrintStream;

import org.junit.Test;

public class MainTest {	
	static public String m_run_parameters = "sc";

	@Test
	public void succ1_abp() throws Exception{
		test("bp-fender-benchmark/","succ1_abp.sc.bl","Error states: 0" );
	}//0.566 hash no join
	//1.454 trie no opt //~1.5s
	//1.189 trie opt1
	//1.003 trie opt2
	//0.638 trie opt3
	
	@Test
	public void succ2_abp2()throws Exception{
		test("bp-fender-benchmark/","succ2_abp2.bl","Error states: 0" );
	}//0.072 hash no join
	//0.118 trie no opt  
	//0.127 trie opt1 //~0.1s
	//0.075 trie opt2 //~0.1s
	//0.073 trie opt3
	
	@Test
	public void succ3_bakery_ver2()throws Exception{
		test("bp-fender-benchmark/","succ3_bakery_ver2.sc.bl","Error states: 0" );
	}//0.0797 hash no join
	//1.293 trie no opt // ~0.1s
	//7.922 trie opt1
	//7.3s trie opt2
	//1.847 trie opt3
	
	@Test
	public void succ4_dec_loop()throws Exception{
		test("bp-fender-benchmark/","succ4_dec.loop.sc.bl","Error states: 0" );
	}//5s hash no join
	//30 seconds
	//5.42 s trie opt3
	@Test
	public void succ5_dek_noloop()throws Exception{
		test("bp-fender-benchmark/","succ5_dek.noloop.sc.bl","Error states: 0" );
	}// 0.138 hash no join
	//1.47 tri no opt  //0.621 s
	//0.919s tri opt1
	//0.4 trie opt2
	//0.209 trie opt3
	
//	@Test
//	public void succ6_pet_loop_full_flush(){
//		test("succ6_pet.loop.k1.full_fences.pso.bl","Error states: 0" );
//	}//429 s
	@Test
	public void succ7_pet_noloop()throws Exception{
		test("bp-fender-benchmark/","succ7_pet.noloop.full_flush.pso.bl","Error states: 0" );
	}//0.861 hash no join
	//7s
	//30s trie opt1
	//15s trie opt2
	//6.201s trie opt3
	@Test
	public void succ8_pet_noloop_sc()throws Exception{
		test("bp-fender-benchmark/","succ8_pet.noloop.sc.bl","Error states: 0" );
	}//0.151 hash no join
	//0.3s
	//0.609 trie opt1
	//0.42 trie opt2
	//0.242 trie opt3
	
//	@Test
//	public void succ9_bak_1v(){
//		test("succ9_bak.1v.pso.bl.all_fences.rf.new_preds","Error states: 0" );
//	}
//	//-- long - several hours.
	
	@Test
	public void fail1_peterson_nofences()throws Exception{
		test("bp-fender-benchmark/","fail1_peterson_nofences.bl","Error states: 1" );
	}////0.114 hash no join
	// 0.21 trie no opt //0.15s
	// 0.124 trie opt1
	// 0.115 trie opt2
	// 0.146 trie opt3
	
	@Test
	public void fail2_bpTest1()throws Exception{
		test("bp-fender-benchmark/","fail2_bpTest1.in","Error states: 1" );
	}//0.069 hash no join
	//0.09 trie no opt  //0.06s
	// 0.067 trie opt1
	// 0.071 trie opt2
	// 0.072 trie opt3
	
	@Test
	public void fail3_bakery1()throws Exception{
		test("bp-fender-benchmark/","fail3_bakery1.in","Error states: 1" );
	}//10.8s hash no join
	//161s trie opt2
	//19s trie op3
	
	@Test
	public void fail4_first()throws Exception{
		test("bp-fender-benchmark/","fail4_first.in","Error states: 1" );
	}// 0.067 hash no join
	//0.116 trie no opt //0.07s
	//0.071 trie opt1
	//0.083 trie opt2
	//0.072 trie opt3
	
	@Test
	public void abp_test_no_fence()throws Exception{
		test("bp-fender-benchmark/abp-suite/","[0,_0].ENCODE_BUFFERS.2v.YF.encoded_buffers.bl","Error states: 0" );
	}//0.121 hash no join
	//1.66 trie no opt
	//0.802 trie opt1
	//0.606 trie opt2
	//0.235 trie opt3
	
	@Test
	public void abp_test_one_fence()throws Exception{
		test("bp-fender-benchmark/abp-suite/","[0,_1].ENCODE_BUFFERS.2v.YF.encoded_buffers.bl","Error states: 0" );
		test("bp-fender-benchmark/abp-suite/","[1,_0].ENCODE_BUFFERS.2v.YF.encoded_buffers.bl","Error states: 0" );
	}//0.241 hash no join
	//3.292 trie no opt
	//1.710 trie opt1
	// 1.240, 1.426 trie opt2
	//0.468 trie opt3
	@Test
	public void abp_test_two_fences()throws Exception{
		test("bp-fender-benchmark/abp-suite/","[1,_1].ENCODE_BUFFERS.2v.YF.encoded_buffers.bl","Error states: 0" );
	}//0.241 hash no join
	//1.660 Trie no opt
	//0.824 Trie opt1
	//0.628, 0.740 Trie opt2
	//0.227 Trie opt3
	
	
	public static void test(String path, String fileName, String messageInOutput) throws Exception{
		
		
		try {
			BufferedReader br;
			File file = new File(path + fileName + ".out");
			System.setOut(new PrintStream(new FileOutputStream(file)));
			String input = path+ fileName+" " +m_run_parameters;
			Main.main((input.split(" ")));
			System.setOut(null);
			br = new BufferedReader(new FileReader(file));
			Boolean testRes = false;
	        String line = br.readLine();
	        while (line != null) { 
				testRes |= line.contains(messageInOutput);
	            line = br.readLine();
			}
			
			assertTrue(fileName + " wrong output",testRes);
	        br.close();
			System.setOut(new PrintStream(new FileOutputStream(FileDescriptor.out)));;
		
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			fail("caught " + e + e.getMessage());
		}
		
	}

}
