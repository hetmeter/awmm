package fsb.trie;

import java.util.HashSet;
import java.util.Set;

import fsb.explore.StateVariables;

public interface TrieInterface<TrieObjType> {

	/**
	 * A simple strait forward recursive insertion.
	 * searches for the key in the trie. and adds the
	 * new element in the place of the key
	 * 
	 * @param key
	 * @param o
	 * @return false if the object already exists in the trie.
	 */
	public abstract boolean simpleAdd(String key, TrieObjType o);
	public abstract boolean optimization1SimpleAdd(String key, TrieObjType o); 
	
	public abstract Set<TrieObjType> addAndRemoveSubsumed(String key, TrieObjType o);
	public abstract Set<TrieObjType> optimization1AddAndRemoveSubsumed(String key, TrieObjType o) ;
	public abstract Set<TrieObjType> optimization2AddAndRemoveSubsumed(StateVariables[] key, int i, int j, TrieObjType o);
	public abstract Set<TrieObjType> optimization3AddAndRemoveSubsumed(StateVariables[] key, TrieObjType o) ;
		
	public abstract boolean isEmpty();

	public abstract boolean hasAnElementSubsumedBy(String key);

	public abstract Set<TrieObjType> allSubsumedBy(String key);

	public abstract Set<TrieObjType> allThatSubsume(String key);	
	public abstract Set<TrieObjType> optimization1AllThatSubsume(String key);
	public abstract Set<TrieObjType> optimization2AllThatSubsume(StateVariables[] key, int i, int j) ;
	public abstract Set<TrieObjType> optimization3AllThatSubsume(StateVariables[] key);
	

	/**
	 * 
	 * @param key
	 * @return
	 */
	public abstract Set<TrieObjType> deleteSubsumedBy(String key);
	public abstract Set<TrieObjType> optimization1DeleteSubsumedBy(String key);
	public abstract Set<TrieObjType> optimization2DeleteSubsumedBy(StateVariables[] key, int i, int j) ;
	public abstract Set<TrieObjType> optimization3DeleteSubsumedBy(StateVariables[] key);
	
	public abstract TrieObjType findExact(String key);
	public abstract TrieObjType optimization1FindExact(String key);
	public abstract TrieObjType optimization2FindExact(StateVariables[] key,int i, int j) ;
	public abstract TrieObjType optimization3FindExact(StateVariables[] key);
	
	public abstract int size();

}