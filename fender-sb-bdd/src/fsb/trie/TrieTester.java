/**
 * 
 */
package fsb.trie;

import static org.junit.Assert.*;

import java.math.BigInteger;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.TreeMap;

import junit.framework.TestCase;

import org.junit.Test;

import fsb.explore.StateVariables;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;

/**
 * @author user
 *
 */

	
public class TrieTester {


	protected class SharedResMapForTrieTester extends StateVariables{
	
		public SharedResMapForTrieTester(int sizeLocals) {
			super(sizeLocals,true);
			// TODO Auto-generated constructor stub
		}
		public SharedResMapForTrieTester(SharedResMapForTrieTester other) {
			super(other);
		}
		public SharedResMapForTrieTester(StateVariables other) {
			super(other);
		}
		@Override
		public SharedResMapForTrieTester clone() {
			return new SharedResMapForTrieTester(this);
		}
	}

	@Test
	public void sanity() {
		TrieInterface<Integer> t = new Trie<Integer>(MAX_KEY_SIZE);
		t.simpleAdd("11 00", 3);
		t.simpleAdd("10 00", 4);
		t.allSubsumedBy("1* 00");
		t.allThatSubsume("10 00");
		t.deleteSubsumedBy("11 00");
		t.isEmpty();
		t.hasAnElementSubsumedBy("10 00");
		t.toString();
		t.addAndRemoveSubsumed("10 00", 2);
		t.optimization1AddAndRemoveSubsumed("11 11", 5);
		t.optimization1AllThatSubsume("11 **");
		t.optimization1FindExact("1* **");
		t.optimization1SimpleAdd("** **", 7);
		
		Random r = new Random(SEED);
		StateVariables[] key = getRandomBitSetKey(r);
		
		t.optimization2AllThatSubsume(key,0,0);
		
		key = getRandomBitSetKey(r);
		t.optimization2AddAndRemoveSubsumed(key, 0, 0, 12);
		
		key = getRandomBitSetKey(r);
		t.optimization2DeleteSubsumedBy(key, 0, 0);
		
		key = getRandomBitSetKey(r);
		t.optimization2FindExact(key, 0, 0);
		
		assertTrue(true);
	}

	/**
	 * @param r
	 * @return
	 */
	private StateVariables[] getRandomBitSetKey(Random r) {
		StateVariables[] key = new StateVariables[1];
		key[0] = new SharedResMapForTrieTester(MAX_KEY_SIZE);
		
		for(int i=0;i<MAX_KEY_SIZE;++i){
			if( r.nextBoolean()){
				key[0].put(i, DetArithValue.getInstance(r.nextInt(2)));
			}else{
				key[0].put(i, NondetArithValue.getInstance());
			}
		}
		return key;
	}
	
//	
//	@Test
//	public void randomStressTestOptimization2(){
//		TrieInterface<Integer> t = new Trie<Integer>(MAX_KEY_SIZE);
//		Random r = new Random(SEED);
//		
//		for(int i=0;i<SIZE;++i){
//		
//			StateVariables[] key = getRandomBitSetKey(r);
//		
//			t.optimization2AllThatSubsume(key,0,0);
//		
//			key = getRandomBitSetKey(r);
//			t.optimization2AddAndRemoveSubsumed(key, 0, 0, r.nextInt());
//		
//			key = getRandomBitSetKey(r);
//			t.optimization2DeleteSubsumedBy(key, 0, 0);
//		
//			key = getRandomBitSetKey(r);
//			t.optimization2findExact(key, 0, 0);
//		}
//	}
	
	/**
	 * @param r
	 * @return
	 */
	private String getRandomStringKey(Random r) {
		String key = "";
		
		for(int i=0;i<MAX_KEY_SIZE;++i){
			switch( r.nextInt(3)){
			case 0:{ key+="1"; break;}
			case 1:{key+="0"; break;}
			case 2:{key+="*"; break;}
			}
		}
		return key;
	}
	
	
	@Test
	public void iterateTest(){
		int key_length = 12;
		int num_elems = (int)Math.pow(3,key_length);
		Trie.cleanup(1,key_length);
		Trie<Integer> t = new Trie<Integer>(10);
		for(int i=0;i<num_elems;i++){
			String key = "";
			int i_div3=i;
			for(int j=0;j<key_length;++j){
				key += (i_div3%3 == 2)?"*":Integer.toString(i_div3%3);
				i_div3 = i_div3/3;
			}
			
			t.simpleAdd(key, i);
		}
		assertTrue(t.size() == num_elems);
		int count = 0;
		HashSet<Integer> st = new HashSet<Integer>();
		Iterator<Integer> it = t.iterator();
		while(it.hasNext()){
			Integer i = it.next();
			assertFalse(st.contains(i));
			st.add(i);
			count++;
		}
		assertTrue(count == num_elems);
	}
	@Test
	public void randomStressTestOptimization1(){
		Trie.cleanup(1, MAX_KEY_SIZE);
		TrieInterface<Integer> t = new Trie<Integer>(MAX_KEY_SIZE);
		TrieInterface<Integer> t2 = new Trie<Integer>(MAX_KEY_SIZE);
		
		Random r = new Random(SEED);
		
		Set<Integer> s1; 
		Set<Integer> s2;
		
		for(int i=0;i<SIZE;++i){
		
			String key = getRandomStringKey(r);
			t.toString();
		if(r.nextBoolean()){
			if(key.equals("100*1111*0100110*0001**0*01*011")){
				assertTrue(t.findExact("100*1111*0100110*0001**0*01*011") == null);
				assertTrue(t2.findExact("100*1111*0100110*0001**0*01*011") == null);
			}
			s1 = t.optimization1AllThatSubsume(key);
			s2 = t2.allThatSubsume(key);
			if(!s1.equals(s2)){
				int ts=0;
				ts++;
			}
			assertTrue(s1.equals(s2));
		}
			
		if(r.nextBoolean()){
			key = getRandomStringKey(r);
			Integer obj =  r.nextInt();
			s1 = t.optimization1AddAndRemoveSubsumed(key, obj);
			s2 = t2.addAndRemoveSubsumed(key, obj);
			if(!s1.equals(s2)){
				int ts=0;
				ts++;
			}
			assertTrue(s1.equals(s2));
		}
		if(r.nextBoolean()){
			key = getRandomStringKey(r);
			s1 = t.optimization1DeleteSubsumedBy(key);
			s2 = t2.deleteSubsumedBy(key);
			assertTrue(s1.equals(s2));
		}
		if(r.nextBoolean()){	
			key = getRandomStringKey(r);
			Integer i1 = t.optimization1FindExact(key);
			Integer i2 = t2.findExact(key);
			assertTrue(i1 == i2);
		}
		}
	}
	@Test
	public void simpleAddTest() {
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		assertFalse(t.simpleAdd("1* 00",-3));
		assertFalse(t.simpleAdd("10 00",-4));
		assertFalse(t.simpleAdd("10 10",-5));
	}
	
	@Test
	public void optimization1SimpleAddTest() {
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		assertFalse(t.optimization1SimpleAdd("1* 00",-3));
		assertFalse(t.optimization1SimpleAdd("10 00",-4));
		assertFalse(t.optimization1SimpleAdd("10 10",-5));
	}

	@Test
	public void allSubsumedByTest() {
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		Set<Integer> s1 = t.allSubsumedBy("1* 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertTrue(s1.contains(4));
		assertFalse(s1.contains(5));
		
		
		Set<Integer> s2 = t.allSubsumedBy("** *1");
		assertTrue(s2.isEmpty());
		
	}
	
	@Test
	public void allThatSubsumeTest() {
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		t.toString();		
		Set<Integer> s1 = t.allThatSubsume("10 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertTrue(s1.contains(4));
		assertFalse(s1.contains(5));
		
		
		Set<Integer> s2 = t.allThatSubsume("** *1");
		assertTrue(s2.isEmpty());
		
	}
	
	@Test
	public void deleteSubsumedByTest(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		Set<Integer> s1 = t.deleteSubsumedBy("10 10");
		assertTrue(s1.remove(5));
		assertTrue(s1.isEmpty());
		
		Set<Integer> s2 = t.deleteSubsumedBy("1* 00");
		assertTrue(s2.remove(3));
		assertTrue(s2.remove(4));
		assertTrue(s2.isEmpty());
		
		Set<Integer> s3 = t.deleteSubsumedBy("10 10");
		assertTrue(s3.isEmpty());
		
		Set<Integer> s4 = t.deleteSubsumedBy("1* 10");
		assertTrue(s4.isEmpty());
		
		
		assertTrue(t.isEmpty());
	
	}
	
	@Test
	public void hasAnElementSubsumedByTest(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUp();
		
		assertTrue(t.hasAnElementSubsumedBy("10 00"));
		assertTrue(t.hasAnElementSubsumedBy("1* 00"));
		assertTrue(t.hasAnElementSubsumedBy("****"));
		assertFalse(t.hasAnElementSubsumedBy("***1"));
		
	}

	/**
	 * sets up an Integer trie that contains ("1* 00",3),("10 00",4),("10 10",5)
	 * @return
	 */
	private TrieInterface<Integer> simpleSetUp() {
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = new Trie<Integer>(4);
		
		assertTrue(t.simpleAdd("1* 00", 3));
		assertFalse(t.isEmpty());
		
		assertTrue(t.simpleAdd("10 00", 4));
		
		assertTrue(t.simpleAdd("10 10", 5));
		return t;
	}

	
	@Test
	public void simpleAddTest_AddRemoveSubsumed(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		assertFalse(t.simpleAdd("1* 00",-3));
		assertFalse(t.simpleAdd("10 10",-4));
		assertFalse(t.simpleAdd("10 0*",-5));
	}

	@Test
	public void optimization1SimpleAddTest_AddRemoveSubsumed(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		assertFalse(t.optimization1SimpleAdd("1* 00",-3));
		assertFalse(t.optimization1SimpleAdd("10 10",-4));
		assertFalse(t.optimization1SimpleAdd("10 0*",-5));
	}
	
	@Test
	public void allSubsumedByTest_AddRemoveSubsumed(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		Set<Integer> s1 = t.allSubsumedBy("1* 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertFalse(s1.contains(6));
		assertFalse(s1.contains(5));
		
		Set<Integer> s2 = t.allSubsumedBy("10 10");
		assertFalse(s2.isEmpty());
		assertFalse(s2.contains(3));
		assertFalse(s2.contains(6));
		assertTrue(s2.contains(5));
		
		Set<Integer> s3 = t.allSubsumedBy("0* **");
		assertTrue(s3.isEmpty());
		
	}
	
	@Test
	public void allThatSubsumeTest_AddRemoveSubsumed(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		Set<Integer> s1 = t.allThatSubsume("10 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertTrue(s1.contains(6));
		assertFalse(s1.contains(5));
		
		
		Set<Integer> s2 = t.allThatSubsume("0* **");
		assertTrue(s2.isEmpty());
		
	}

	@Test
	public void deleteSubsumedByTest_AddRemoveSubsumed(){
		Trie.cleanup(1, 4);
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		
		Set<Integer> s1 = t.deleteSubsumedBy("10 10");
		assertTrue(s1.remove(5));
		assertTrue(s1.isEmpty());
		
		Set<Integer> s2 = t.deleteSubsumedBy("1* 0*");
		assertTrue(s2.remove(3));
		assertTrue(s2.remove(6));
		assertTrue(s2.isEmpty());
		
		Set<Integer> s3 = t.deleteSubsumedBy("10 10");
		assertTrue(s3.isEmpty());
		
		Set<Integer> s4 = t.deleteSubsumedBy("** **");
		assertTrue(s4.isEmpty());
		
		
		assertTrue(t.isEmpty());
	
	}
	
	@Test
	public void hasAnElementSubsumedByTest_AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithSubsumedRemoveAdd();
		
		assertTrue(t.hasAnElementSubsumedBy("10 10"));
		assertTrue(t.hasAnElementSubsumedBy("1* 0*"));
		assertFalse(t.hasAnElementSubsumedBy("10 00"));
		assertTrue(t.hasAnElementSubsumedBy("****"));
		assertFalse(t.hasAnElementSubsumedBy("***1"));
		
	}

	
	/**
	 * sets up an Integer trie that contains ("1* 00",3),("10 10",5),("10 0*",6) 
	
	 * @return
	 */
	private TrieInterface<Integer> simpleSetUpWithSubsumedRemoveAdd() {
		TrieInterface<Integer> t = new Trie<Integer>(4);
		
		Set<Integer> s = t.addAndRemoveSubsumed("10 00", 4);
		assertTrue(s.isEmpty());
		assertFalse(t.isEmpty());
		
		s = t.addAndRemoveSubsumed("1* 00", 3);
		assertTrue(s.remove(4));
		assertTrue(s.isEmpty());
		
		assertFalse(t.isEmpty());
		
		s=t.addAndRemoveSubsumed("10 10", 5);
		assertTrue(s.isEmpty());
		
		s=t.addAndRemoveSubsumed("10 0*",6);
		assertTrue(s.isEmpty());
		
		return t;
	}

	@Test
	public void simpleAddTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		assertFalse(t.simpleAdd("1* 00",-3));
		assertFalse(t.simpleAdd("10 10",-4));
		assertFalse(t.simpleAdd("10 0*",-5));
	}
	
	@Test
	public void optimization1SimpleAddTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		assertFalse(t.optimization1SimpleAdd("1* 00",-3));
		assertFalse(t.optimization1SimpleAdd("10 10",-4));
		assertFalse(t.optimization1SimpleAdd("10 0*",-5));
	}
	
	@Test
	public void allSubsumedByTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		
		Set<Integer> s1 = t.allSubsumedBy("1* 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertFalse(s1.contains(6));
		assertFalse(s1.contains(5));
		
		Set<Integer> s2 = t.allSubsumedBy("10 10");
		assertFalse(s2.isEmpty());
		assertFalse(s2.contains(3));
		assertFalse(s2.contains(6));
		assertTrue(s2.contains(5));
		
		Set<Integer> s3 = t.allSubsumedBy("0* **");
		assertTrue(s3.isEmpty());
		
	}
	
	@Test
	public void allThatSubsumeTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		
		Set<Integer> s1 = t.allThatSubsume("10 00");
		assertFalse(s1.isEmpty());
		assertTrue(s1.contains(3));
		assertTrue(s1.contains(6));
		assertFalse(s1.contains(5));
		
		
		Set<Integer> s2 = t.allThatSubsume("0* **");
		assertTrue(s2.isEmpty());
		
	}
	@Test
	public void deleteSubsumedByTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		Set<Integer> s1 = t.deleteSubsumedBy("10 10");
		assertTrue(s1.remove(5));
		assertTrue(s1.isEmpty());
		
		Set<Integer> s2 = t.deleteSubsumedBy("1* 0*");
		assertTrue(s2.remove(3));
		assertTrue(s2.remove(6));
		assertTrue(s2.isEmpty());
		
		Set<Integer> s3 = t.deleteSubsumedBy("10 10");
		assertTrue(s3.isEmpty());
		
		Set<Integer> s4 = t.deleteSubsumedBy("** **");
		assertTrue(s4.isEmpty());
		
		
		assertTrue(t.isEmpty());
	
	}
	
	@Test
	public void hasAnElementSubsumedByTest_Optimization1AddRemoveSubsumed(){
		TrieInterface<Integer> t = simpleSetUpWithOptimization1SubsumedRemoveAdd();
		
		assertTrue(t.hasAnElementSubsumedBy("10 10"));
		assertTrue(t.hasAnElementSubsumedBy("1* 0*"));
		assertFalse(t.hasAnElementSubsumedBy("10 00"));
		assertTrue(t.hasAnElementSubsumedBy("****"));
		assertFalse(t.hasAnElementSubsumedBy("***1"));
		
	}


	/**
	 * sets up an Integer trie that contains ("1* 00",3),("10 10",5),("10 0*",6) 
	
	 * @return
	 */
	private TrieInterface<Integer> simpleSetUpWithOptimization1SubsumedRemoveAdd() {
		TrieInterface<Integer> t = new Trie<Integer>(4);
		
		Set<Integer> s = t.optimization1AddAndRemoveSubsumed("10 00", 4);
		assertTrue(s.isEmpty());
		assertFalse(t.isEmpty());
		
		s = t.optimization1AddAndRemoveSubsumed("1* 00", 3);
		assertTrue(s.remove(4));
		assertTrue(s.isEmpty());
		
		assertFalse(t.isEmpty());
		
		s=t.optimization1AddAndRemoveSubsumed("10 10", 5);
		assertTrue(s.isEmpty());
		
		s=t.optimization1AddAndRemoveSubsumed("10 0*",6);
		assertTrue(s.isEmpty());
		
		return t;
	}
	
    @Test
    public void keys() {//0.821
        TrieInterface<BigInteger> trie
            = new Trie<BigInteger>((int)Math.log(SIZE));
        
        Map<String, BigInteger> map 
            = new TreeMap<String, BigInteger>();
        
        // Fill the Trie and the Map
        for (int i = 0; i < SIZE; i++) {
            BigInteger value = BigInteger.valueOf(i);
            
            
            TestCase.assertTrue(trie.simpleAdd(value.toString(2), value));
            
            map.put(value.toString(2), value);
        }
        
        TestCase.assertEquals(map.size(), trie.size());
        
        // Check if all values are there *AND* if they are in
        // the same order!
        for (String expected : map.keySet()) {
            BigInteger value = trie.findExact(expected);
            BigInteger expected_val = map.get(expected);
            
            TestCase.assertEquals(expected_val, value);
        }
    }
    @Test//0.758
    public void keysOptimization1Test() {
        TrieInterface<BigInteger> trie
            = new Trie<BigInteger>((int)Math.log(SIZE));
        
        Map<String, BigInteger> map 
            = new TreeMap<String, BigInteger>();
        
        // Fill the Trie and the Map
        for (int i = 0; i < SIZE; i++) {
            BigInteger value = BigInteger.valueOf(i);
            
            
            TestCase.assertTrue(trie.optimization1SimpleAdd(value.toString(2), value));
            
            map.put(value.toString(2), value);
        }
        
        TestCase.assertEquals(map.size(), trie.size());
        
        // Check if all values are there *AND* if they are in
        // the same order!
        for (String expected : map.keySet()) {
            BigInteger value = trie.findExact(expected);
            BigInteger expected_val = map.get(expected);
            
            TestCase.assertEquals(expected_val, value);
        }
    }

//        @Test //64.699 s
//    public void keysHeavyTest(){
//    		for(int j=0; j<100 ; ++j){
//    			keys();
//    		}
//    	}
//    
//    @Test //64.536
//    public void keysOptimization1TestHeavyTest(){
//    		for(int j=0; j<100 ; ++j){
//			keys();
//		}
//    }
    
    
    static final int MAX_KEY_SIZE=31;
    static final int SEED=63;
    private static final int SIZE = 20000;
    
    private static final int HUGE_SIZE = 2000000;

}
