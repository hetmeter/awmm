package fsb.trie;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import javax.management.RuntimeErrorException;

import fsb.explore.StateVariables;
import fsb.tvl.ArithValue;
import fsb.tvl.DetArithValue;
import fsb.tvl.NondetArithValue;
import gnu.trove.map.hash.THashMap;

/**
 * given a set of ternary states, vectors of (1|0|*), where during a search of a
 * state (e.g. (1,0,*) ) I want the search to return elements which are subsumed
 * or subsume that state (e.g. (1,0,1), (*,0,*)) and then the state space to
 * retain only the top element of the group of found states and the state
 * searched. Though each entry of a vector originated from predicate value, the
 * system is actually unaware of that. That said, I believe that it could be
 * interpreted as a set of cubes of equal length, where a given cube represents
 * the state of the system. <br>
 * <br>
 * accepts a string of 0,1,*. <br>
 * also blanks spaces(" ") that will be ignored
 * 
 * @author <b> yuri </b> <br>
 *         disclaimer: this is a hideous implementation, full of bugs. if your
 *         system crashes or experiences any problems due to the use of this
 *         class, I take no responsibility for it. But I am interested in what
 *         you find, and to fix it, if I can. Please notify me at {@link syurim
 *         @gmail.com} of what is the error you found.
 * @param <TrieObjType>
 */
public class Trie<TrieObjType> implements TrieInterface<TrieObjType>,Iterable<TrieObjType>
{
	/**
	 * create a new Trie (characters 1,0,* also blanks)
	 */

	@SuppressWarnings("unchecked")
	public Trie(int key_length) {
		m_child = new Trie[LOCATIONS];
		m_father = null;
		if (null == m_temp_key_buffer) {
			// the buffer will have the key copied into it each time
			// asuming blanks will take up-to half the space.
			m_temp_key_buffer = new char[key_length * 3];
		}
	}

	static public void cleanup(int max_subsume_for_elem_search, int key_length) {
		m_temp_key_buffer = new char[key_length * 3];
		m_max_subsume_for_elem_search = max_subsume_for_elem_search;
		m_temp_key_buffer_lock = -1;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#add(java.lang.String, TrieObjType)
	 */

	public boolean optimization1SimpleAdd(String key, TrieObjType o) {

		// check chars of string 10M times using cbuff[i] method
		// for (int n = 0; n < 1_000_000; n++) {
		// int count = data.length();
		// data.getChars(0, count, cbuff, 0);
		// for (int i = 0; i < count; i++) {
		// if (cbuff[i] <= ' ') {
		// throw new IllegalDataException("Found whitespace");
		// }
		// }
		// }
		if (m_temp_key_buffer_lock != -1) {
			throw new RuntimeException("key_buffer_locked " + m_temp_key_buffer);
		}
		m_temp_key_buffer_lock = 1;
		int key_length = key.length();
		int location = -1;
		char key_char;
		key.getChars(0, key.length(), m_temp_key_buffer, 0);
		Trie<TrieObjType> temp = this;
		for (int i = 0; i < key_length; ++i) {
			key_char = m_temp_key_buffer[i];
			if (key_char == ' ') {
				continue;
			}

			location = charToLoc.get(key_char);
			if (temp.m_child[location] == null) {
				temp.m_child[location] = new Trie<TrieObjType>(0);
				temp.m_child[location].m_father = temp;
			}
			temp = temp.m_child[location];

		}
		m_temp_key_buffer_lock = -1;
		if (null != temp.m_data) {
			boolean eq = temp.m_data.equals(o);
			int hash1 = o.hashCode();
			int hash2 = temp.m_data.hashCode();
			return (hash1 == hash2) && eq;
		}
		temp.m_data = o;
		return true;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#add(java.lang.String, TrieObjType)
	 */
	@Override
	public boolean simpleAdd(String key, TrieObjType o) {
		if (key.isEmpty()) {
			if (null != m_data) {
				boolean eq = m_data.equals(o);
				int hash1 = o.hashCode();
				int hash2 = m_data.hashCode();
				return (hash1 == hash2) && eq;
			}
			this.m_data = o;
			return true;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);
		Integer childId = charToLoc.get(keyFirstChar);

		TrieInterface<TrieObjType> son = getOrAddTrieSonByKey(childId);
		return son.simpleAdd(keySuffix, o);
	}

	@Override
	public Set<TrieObjType> addAndRemoveSubsumed(String key, TrieObjType o) {

		if (key.isEmpty()) {
			if (null != m_data) {
				return Collections.EMPTY_SET;
			}
			this.m_data = o;
			return Collections.EMPTY_SET;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);
		Integer childId = charToLoc.get(keyFirstChar);

		TrieInterface<TrieObjType> son = getOrAddTrieSonByKey(childId);
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (keyFirstChar == '*') {
			deleteAllFromSonSubsumedBy(ret, '1', keySuffix);
			deleteAllFromSonSubsumedBy(ret, '0', keySuffix);
		}
		ret.addAll(son.addAndRemoveSubsumed(keySuffix, o));
		return ret;
	}

	/**
	 * @param keyFirstChar
	 * @param keySuffix
	 * @param son
	 * @return
	 * @throws RuntimeException
	 *             when a char in the key is not one of 0,1,*," "
	 */
	private TrieInterface<TrieObjType> getOrAddTrieSonByKey(Integer childId) {

		TrieInterface<TrieObjType> son = m_child[childId];
		if (null == son) {
			m_child[childId] = new Trie<TrieObjType>(m_temp_key_buffer.length);
			m_child[childId].m_father = this;
			son = m_child[childId];

		}

		return son;
	}

	/**
	 * @param keyFirstChar
	 * @param keySuffix
	 * @param son
	 * @return null if the son searched for is null
	 * @throws RuntimeException
	 *             when a char in the key is not one of 0,1,*,_
	 */
	private TrieInterface<TrieObjType> getOrNullTrieSonByKey(Integer childId) {

		return m_child[childId];
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#isEmpty()
	 */
	@Override
	public boolean isEmpty() {
		boolean ret = true;
		for (int i = 0; i < LOCATIONS; ++i) {
			ret = ret && (m_child[i] == null);
		}
		return ret && (m_data == null);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#hasAnElementSubsumedBy(java.lang.String)
	 */
	@Override
	public boolean hasAnElementSubsumedBy(String key) {

		if (key.isEmpty()) {
			return true;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);

		if (sonHasAnElementSubsumedBy(keyFirstChar, keySuffix)) {
			return true;
		}

		if (keyFirstChar == '*') {
			return sonHasAnElementSubsumedBy('1', keySuffix)
					|| sonHasAnElementSubsumedBy('0', keySuffix);
		}

		return false;
	}

	/**
	 * @param keyFirstChar
	 * @param keySuffix
	 * @return
	 */
	private boolean sonHasAnElementSubsumedBy(char keyFirstChar,
			String keySuffix) {
		Integer childId = charToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);
		if (son != null) {
			return son.hasAnElementSubsumedBy(keySuffix);
		}

		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#allSubsumedBy(java.lang.String)
	 */
	@Override
	public Set<TrieObjType> allSubsumedBy(String key) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (key.isEmpty()) {
			if (null != m_data) {
				ret.add(m_data);
			}
			return ret;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);

		allFromSonSubsumedBy(ret, keyFirstChar, keySuffix);

		if (keyFirstChar == '*') {
			allFromSonSubsumedBy(ret, '1', keySuffix);
			allFromSonSubsumedBy(ret, '0', keySuffix);

		}
		return ret;

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private void allFromSonSubsumedBy(HashSet<TrieObjType> ret,
			char keyFirstChar, String keySuffix) {
		Integer childId = charToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);
		if (son != null) {
			ret.addAll(son.allSubsumedBy(keySuffix));
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#allThatSubsume(java.lang.String)
	 */
	@Override
	public Set<TrieObjType> allThatSubsume(String key) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (key.isEmpty()) {
			ret.add(m_data);
			return ret;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);

		allFromSonThatSubsume(ret, keyFirstChar, keySuffix);

		if (keyFirstChar != '*') {
			allFromSonThatSubsume(ret, '*', keySuffix);
		}
		return ret;

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private void allFromSonThatSubsume(HashSet<TrieObjType> ret,
			char keyFirstChar, String keySuffix) {
		if (ret.size() == getMaxSubsumeForElemSearch()) {
			return;
		}
		Integer childId = charToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);

		if (son != null) {
			ret.addAll(son.allThatSubsume(keySuffix));
		}
	}

	@Override
	public Set<TrieObjType> optimization1AllThatSubsume(String key) {
		if (m_temp_key_buffer_lock != -1) {
			throw new RuntimeException(
					"should be -1 but.. m_temp_key_buffer_lock = "
							+ m_temp_key_buffer_lock);
		}
		m_temp_key_buffer_lock = 4;
		Set<TrieObjType> ret = new HashSet<TrieObjType>();
		int key_length = key.length();
		int location = -1;
		char key_char;
		key.getChars(0, key.length(), m_temp_key_buffer, 0);
		Stack<Trie<TrieObjType>> work_list = new Stack<Trie<TrieObjType>>();
		Stack<Trie<TrieObjType>> prev_stage = new Stack<Trie<TrieObjType>>();
		prev_stage.push(this);

		for (int i = 0; i < key_length; ++i) {

			key_char = m_temp_key_buffer[i];
			if (key_char == ' ') {
				continue;
			}
			location = charToLoc.get(key_char);
			while (!prev_stage.isEmpty()) {
				Trie<TrieObjType> trie_node = prev_stage.pop();

				if (trie_node.m_child[location] != null) {
					work_list.add(trie_node.m_child[location]);
				}
				if (key_char != '*') {
					int location2 = charToLoc.get('*');
					if (trie_node.m_child[location2] != null) {
						work_list.add(trie_node.m_child[location2]);
					}
				}// end of if key_char != '*'
			}// end of while!prev_stage.isEmpty
			Stack<Trie<TrieObjType>> tmp = prev_stage;
			prev_stage = work_list;
			work_list = tmp;
		}

		// found elements;
		while (!prev_stage.isEmpty()) {
			Trie<TrieObjType> tr = prev_stage.pop();
			if (tr.m_data != null) {
				ret.add(tr.m_data);
				if (ret.size() >= m_max_subsume_for_elem_search) {
					m_temp_key_buffer_lock = -1;
					return ret;
				}
			}
		}
		m_temp_key_buffer_lock = -1;
		return ret;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#allThatSubsume(java.lang.String)
	 */
	@Override
	public Set<TrieObjType> optimization2AllThatSubsume(StateVariables[] key,
			int i, int j) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (i >= key.length) {
			if (null != m_data) {
				ret.add(m_data);
			} else {
				throw new RuntimeException(
						"reached the end of key(I think) but the element is not here "
								+ key);
			}
			return ret;
		}
		if (j >= key[i].size()) {
			return optimization2AllThatSubsume(key, i + 1, 0);
		}

		ArithValue val = key[i].get(j);
		optimization2AllFromSonThatSubsume(ret, key, val, i, j);

		if (val.isDetermined()) {
			optimization2AllFromSonThatSubsume(ret, key,
					NondetArithValue.getInstance(), i, j);
		}
		return ret;

	}

	@Override
	public Set<TrieObjType> optimization3AllThatSubsume(StateVariables[] key) {

		Set<TrieObjType> ret = new HashSet<TrieObjType>();
		int location = -1;
		ArithValue key_val;

		Stack<Trie<TrieObjType>> work_list = new Stack<Trie<TrieObjType>>();
		Stack<Trie<TrieObjType>> prev_stage = new Stack<Trie<TrieObjType>>();
		prev_stage.push(this);

		for (int i = 0; i < key.length; ++i) {
			for (int j = 0; j < key[i].size(); ++j) {
				key_val = key[i].get(j);

				location = arithValToLoc.get(key_val);
				while (!prev_stage.isEmpty()) {
					Trie<TrieObjType> trie_node = prev_stage.pop();

					if (trie_node.m_child[location] != null) {
						work_list.add(trie_node.m_child[location]);
					}
					if (key_val.isDetermined()) {
						int location2 = arithValToLoc.get(NondetArithValue
								.getInstance());
						if (trie_node.m_child[location2] != null) {
							work_list.add(trie_node.m_child[location2]);
						}
					}// end of if key_char == '*'

				}// end of while!prev_stage.isEmpty
				Stack<Trie<TrieObjType>> tmp = prev_stage;
				prev_stage = work_list;
				work_list = tmp;
			}
		}// end of for(int i
			// found elements;
		while (!prev_stage.isEmpty()) {
			Trie<TrieObjType> tr = prev_stage.pop();
			if (tr.m_data != null) {
				ret.add(tr.m_data);
				if (ret.size() >= m_max_subsume_for_elem_search) {
					return ret;
				}
			}
		}

		return ret;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#deleteSubsumedBy(java.lang.String)
	 */
	@Override
	public Set<TrieObjType> deleteSubsumedBy(String key) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (key.isEmpty()) {
			if (m_data != null) {
				ret.add(m_data);
			}
			m_data = null;
			return ret;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);

		deleteAllFromSonSubsumedBy(ret, keyFirstChar, keySuffix);

		if (keyFirstChar == '*') {
			deleteAllFromSonSubsumedBy(ret, '1', keySuffix);
			deleteAllFromSonSubsumedBy(ret, '0', keySuffix);

		}

		cleanUp();

		return ret;

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private void deleteAllFromSonSubsumedBy(HashSet<TrieObjType> ret,
			char keyFirstChar, String keySuffix) {
		Integer childId = charToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);
		if (son != null) {
			ret.addAll(son.deleteSubsumedBy(keySuffix));
		}
	}

	/**
	 */
	@Override
	public HashSet<TrieObjType> optimization1DeleteSubsumedBy(String key) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (m_temp_key_buffer_lock != -1) {
			throw new RuntimeException(
					"should be -1 but.. m_temp_key_buffer_lock = "
							+ m_temp_key_buffer_lock);
		}
		m_temp_key_buffer_lock = 1;
		int key_length = key.length();
		int location = -1;
		char key_char;
		key.getChars(0, key.length(), m_temp_key_buffer, 0);
		Stack<Trie<TrieObjType>> prev_stage = new Stack<Trie<TrieObjType>>();
		Stack<Trie<TrieObjType>> work_list = new Stack<Trie<TrieObjType>>();
		prev_stage.add(this);

		for (int i = 0; i < key_length; ++i) {

			key_char = m_temp_key_buffer[i];
			if (key_char == ' ') {
				continue;
			}

			while (!prev_stage.isEmpty()) {

				Trie<TrieObjType> tri_node = prev_stage.pop();

				location = charToLoc.get(key_char);

				if (tri_node.m_child[location] != null) {
					work_list.add(tri_node.m_child[location]);
				}
				if (key_char == '*') {
					int location2 = charToLoc.get('1');
					if (tri_node.m_child[location2] != null) {
						work_list.add(tri_node.m_child[location2]);
					}
					int location3 = charToLoc.get('0');
					if (tri_node.m_child[location3] != null) {
						work_list.add(tri_node.m_child[location3]);
					}
				}
				tri_node = tri_node.m_child[location];
			}

			Stack<Trie<TrieObjType>> tmp_for_replacement = work_list;
			work_list = prev_stage;
			prev_stage = tmp_for_replacement;
		}
		// deletion stage
		for (Trie<TrieObjType> tr : prev_stage) {
			if (tr.m_data != null) {
				ret.add(tr.m_data);
				tr.m_data = null;
			}
		}
		// cleanup
		while (!prev_stage.isEmpty()) {
			Trie<TrieObjType> tr = prev_stage.pop();
			Trie<TrieObjType> father = tr.m_father;
			if (null != father) {
				father.cleanUp();
				prev_stage.push(father);
			}
		}
		m_temp_key_buffer_lock = -1;
		return ret;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#deleteSubsumedBy(java.lang.String)
	 */
	@Override
	public Set<TrieObjType> optimization2DeleteSubsumedBy(StateVariables[] key,
			int i, int j) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (key.length >= i) {
			if (m_data != null) {
				ret.add(m_data);
			}
			m_data = null;
			return ret;
		}

		if (key[i].size() >= j) {
			return optimization2DeleteSubsumedBy(key, i + 1, 0);
		}

		ArithValue keyFirstChar = key[i].get(j);

		optimization2DeleteAllFromSonSubsumedBy(ret, keyFirstChar, key, i, j);

		if (!keyFirstChar.isDetermined()) {
			optimization2DeleteAllFromSonSubsumedBy(ret,
					DetArithValue.getInstance(1), key, i, j);
			optimization2DeleteAllFromSonSubsumedBy(ret,
					DetArithValue.getInstance(0), key, i, j);

		}

		cleanUp();

		return ret;

	}

	/**
	 */
	@Override
	public HashSet<TrieObjType> optimization3DeleteSubsumedBy(
			StateVariables[] key) {
		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();

		int location = -1;
		ArithValue key_val;
		Stack<Trie<TrieObjType>> prev_stage = new Stack<Trie<TrieObjType>>();
		Stack<Trie<TrieObjType>> work_list = new Stack<Trie<TrieObjType>>();
		prev_stage.add(this);

		for (int i = 0; i < key.length; ++i) {
			for (int j = 0; j < key[i].size(); ++j) {
				key_val = key[i].get(j);

				while (!prev_stage.isEmpty()) {

					Trie<TrieObjType> tri_node = prev_stage.pop();

					location = arithValToLoc.get(key_val);

					if (tri_node.m_child[location] != null) {
						work_list.add(tri_node.m_child[location]);
					}
					if (!key_val.isDetermined()) {
						int location2 = arithValToLoc.get(DetArithValue
								.getInstance(1));
						if (tri_node.m_child[location2] != null) {
							work_list.add(tri_node.m_child[location2]);
						}
						int location3 = arithValToLoc.get(DetArithValue
								.getInstance(1));
						if (tri_node.m_child[location3] != null) {
							work_list.add(tri_node.m_child[location3]);
						}
					}
					tri_node = tri_node.m_child[location];
				}

				Stack<Trie<TrieObjType>> tmp_for_replacement = work_list;
				work_list = prev_stage;
				prev_stage = tmp_for_replacement;
			}// end of for(int j
		}
		// deletion stage
		for (Trie<TrieObjType> tr : prev_stage) {
			if (tr.m_data != null) {
				ret.add(tr.m_data);
				tr.m_data = null;
			}
		}
		// cleanup
		while (!prev_stage.isEmpty()) {
			Trie<TrieObjType> tr = prev_stage.pop();
			Trie<TrieObjType> father = tr.m_father;
			if (null != father) {
				father.cleanUp();
				prev_stage.push(father);
			}
		}
		return ret;

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private void optimization2DeleteAllFromSonSubsumedBy(
			HashSet<TrieObjType> ret, ArithValue keyFirstChar,
			StateVariables[] key, int i, int j) {
		Integer childId = arithValToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);
		if (son != null) {
			ret.addAll(son.optimization2DeleteSubsumedBy(key, i, j));
		}
	}

	/**
	 * 
	 */
	private void cleanUp() {

		for (int i = 0; i < LOCATIONS; ++i)
			if (m_child[i] != null) {
				if (m_child[i].isEmpty()) {
					m_child[i] = null;
				}
			}
	}

	@Override
	public Set<TrieObjType> optimization2AddAndRemoveSubsumed(
			StateVariables[] key, int i, int j, TrieObjType o) {

		HashSet<TrieObjType> ret = new HashSet<TrieObjType>();
		if (key.length <= i) {
			if (null == m_data) {
				this.m_data = o;
			}
			return ret;
		}
		if (key[i].size() <= j) {

			return optimization2AddAndRemoveSubsumed(key, i + 1, 0, o);
		}

		ArithValue keyFirstChar = key[i].get(j);

		Integer childId = arithValToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrAddTrieSonByKey(childId);

		if (!keyFirstChar.isDetermined()) {
			optimization2DeleteAllFromSonSubsumedBy(ret,
					DetArithValue.getInstance(1), key, i, j);
			optimization2DeleteAllFromSonSubsumedBy(ret,
					DetArithValue.getInstance(0), key, i, j);
		}
		ret.addAll(son.optimization2AddAndRemoveSubsumed(key, i, j + 1, o));
		return ret;
	}

	/**
	 * unwinding of the recursion of the
	 * {@link #addAndRemoveSubsumed(String, Object)} and in-lining of the
	 * {@link #deleteSubsumedBy(String)}
	 * 
	 * @param o
	 *            - the object to add
	 * @param key
	 *            - the search key
	 */
	public Set<TrieObjType> optimization1AddAndRemoveSubsumed(String key,
			TrieObjType o) {
		if (m_temp_key_buffer_lock != -1) {
			throw new RuntimeException(
					"should be -1 but.. m_temp_key_buffer_lock = "
							+ m_temp_key_buffer_lock);
		}
		m_temp_key_buffer_lock = 2;
		Set<TrieObjType> ret = new HashSet<TrieObjType>();
		int key_length = key.length();
		int location = -1;
		char key_char;
		if (key.length() > m_temp_key_buffer.length) {
			int i = 0;
			i++;
		}
		key.getChars(0, key.length(), m_temp_key_buffer, 0);
		Stack<Trie<TrieObjType>> work_list = new Stack<Trie<TrieObjType>>();
		Stack<Trie<TrieObjType>> prev_stage = new Stack<Trie<TrieObjType>>();
		Trie<TrieObjType> exact = this;

		for (int i = 0; i < key_length; ++i) {

			key_char = m_temp_key_buffer[i];
			if (key_char == ' ') {
				continue;
			}
			location = charToLoc.get(key_char);
			while (!prev_stage.isEmpty()) {
				Trie<TrieObjType> tri_node = prev_stage.pop();
				if (tri_node.m_child[location] != null) {
					work_list.add(tri_node.m_child[location]);
				}
				if (key_char == '*') {
					int location2 = charToLoc.get('1');
					if (tri_node.m_child[location2] != null) {
						work_list.add(tri_node.m_child[location2]);
					}
					int location3 = charToLoc.get('0');
					if (tri_node.m_child[location3] != null) {
						work_list.add(tri_node.m_child[location3]);
					}
				}
				// tri_node = tri_node.m_child[location];
			}
			location = charToLoc.get(key_char);
			if (exact.m_child[location] == null) {
				exact.m_child[location] = new Trie<TrieObjType>(0);
				exact.m_child[location].m_father = exact;
			}
			if (key_char == '*') {
				int location2 = charToLoc.get('1');
				if (exact.m_child[location2] != null) {
					work_list.add(exact.m_child[location2]);
				}
				location2 = charToLoc.get('0');
				if (exact.m_child[location2] != null) {
					work_list.add(exact.m_child[location2]);
				}
			}
			exact = exact.m_child[location];

			Stack<Trie<TrieObjType>> tmp = prev_stage;
			prev_stage = work_list;
			work_list = tmp;
		}
		// adding the element
		exact.m_data = o;

		// deletion stage
		for (Trie<TrieObjType> tr : prev_stage) {
			if (tr.m_data != null) {
				ret.add(tr.m_data);
				tr.m_data = null;
			}
		}

		// cleanup
		while (!prev_stage.isEmpty()) {
			Trie<TrieObjType> tr = prev_stage.pop();
			Trie<TrieObjType> father = tr.m_father;
			if (null != father) {
				father.cleanUp();
				prev_stage.add(father);
			}
		}

		m_temp_key_buffer_lock = -1;
		return ret;
	}

	/**
	 * unwinding of the recursion of the
	 * {@link #addAndRemoveSubsumed(String, Object)} and in-lining of the
	 * {@link #deleteSubsumedBy(String)}
	 * 
	 * @param o
	 *            - the object to add
	 * @param key
	 *            - the search key
	 */
	@Override
	public Set<TrieObjType> optimization3AddAndRemoveSubsumed(
			StateVariables[] key, TrieObjType o) {

		Set<TrieObjType> ret = new HashSet<TrieObjType>();
		int location = -1;
		Stack<Touple<Trie<TrieObjType>, Touple<Integer, Integer>>> work_list = new Stack<Touple<Trie<TrieObjType>, Touple<Integer, Integer>>>();
		Trie<TrieObjType> exact = this;

		for (int i = 0; i < key.length; ++i) {
			for (int j = 0; j < key[i].size(); ++j) {

				ArithValue key_val = key[i].get(j);
				location = arithValToLoc.get(key_val);
				if (exact.m_child[location] == null) {
					exact.m_child[location] = new Trie<TrieObjType>(0);
					exact.m_child[location].m_father = exact;
				}
				if (!key_val.isDetermined()) {
					int location2 = arithValToLoc.get(DetArithValue
							.getInstance(1));
					if (exact.m_child[location2] != null) {
						work_list.push(new Touple(exact.m_child[location2],
								new Touple(i, j)));
					}
					location2 = arithValToLoc.get(DetArithValue.getInstance(0));
					if (exact.m_child[location2] != null) {
						work_list.push(new Touple(exact.m_child[location2],
								new Touple(i, j)));
					}
				}
				exact = exact.m_child[location];
			}
		}
		while (!work_list.isEmpty()) {

			Touple<Trie<TrieObjType>, Touple<Integer, Integer>> elem = work_list
					.pop();
			Trie<TrieObjType> trie_elem = elem.t1;
			Integer start_loc_i = elem.t2.t1;
			Integer start_loc_j = elem.t2.t2;

			TrieObjType obj = optimization3AddAndRemoveSusbumedHeleper(key,
					trie_elem, start_loc_i, start_loc_j, work_list);
			if (obj != null) {
				ret.add(obj);
			}

		}
		// adding the element
		exact.m_data = o;

		return ret;
	}

	private TrieObjType optimization3AddAndRemoveSusbumedHeleper(
			StateVariables[] key, Trie<TrieObjType> input_elem,
			Integer start_loc_i, Integer start_loc_j,
			Stack<Touple<Trie<TrieObjType>, Touple<Integer, Integer>>> work_list) {
		TrieObjType ret = null;
		Trie<TrieObjType> trie_elem = input_elem;

		for (int i = start_loc_i; i < key.length; ++i) {
			for (int j = 0; j < key[i].size(); ++j) {
				if (i == start_loc_i && j <= start_loc_j) {
					continue;
				}
				ArithValue key_val = key[i].get(j);
				int location = arithValToLoc.get(key_val);

				if (!key_val.isDetermined()) {
					int location2 = arithValToLoc.get(DetArithValue
							.getInstance(1));
					if (trie_elem.m_child[location2] != null) {
						work_list.push(new Touple(trie_elem.m_child[location2],
								new Touple(i, j)));
					}
					location2 = arithValToLoc.get(DetArithValue.getInstance(0));
					if (trie_elem.m_child[location2] != null) {
						work_list.push(new Touple(trie_elem.m_child[location2],
								new Touple(i, j)));
					}
				}
				if (trie_elem.m_child[location] == null) {
					return null;
				}
				trie_elem = trie_elem.m_child[location];
			}
		}
		ret = trie_elem.m_data;
		trie_elem.m_data = null;
		while (trie_elem != null) {
			trie_elem.cleanUp();
			trie_elem = trie_elem.m_father;
		}
		return ret;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#findExact(java.lang.String)
	 */
	@Override
	public TrieObjType findExact(String key) {

		TrieObjType ret = null;
		if (key.isEmpty()) {
			if (null != m_data) {
				ret = m_data;
			}
			return ret;
		}

		key = key.trim();

		char keyFirstChar = key.charAt(0);
		String keySuffix = key.substring(1);

		return findAtSonExact(keyFirstChar, keySuffix);

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private TrieObjType findAtSonExact(char keyFirstChar, String keySuffix) {
		TrieObjType ret = null;
		Integer childId = charToLoc.get(keyFirstChar);
		TrieInterface<TrieObjType> son = getOrNullTrieSonByKey(childId);

		if (son != null) {
			ret = son.findExact(keySuffix);
		}
		return ret;
	}

	@Override
	public TrieObjType optimization1FindExact(String key) {
		if (m_temp_key_buffer_lock != -1) {
			throw new RuntimeException("key_buffer_locked "
					+ m_temp_key_buffer_lock);
		}
		m_temp_key_buffer_lock = 3;
		int key_length = key.length();
		int location = -1;
		char key_char;
		key.getChars(0, key.length(), m_temp_key_buffer, 0);
		Trie<TrieObjType> temp = this;
		for (int i = 0; i < key_length; ++i) {
			key_char = m_temp_key_buffer[i];
			if (key_char == ' ') {
				continue;
			}

			location = charToLoc.get(key_char);
			if (temp.m_child[location] == null) {
				m_temp_key_buffer_lock = -1;
				return null;
			}
			temp = temp.m_child[location];

		}
		m_temp_key_buffer_lock = -1;

		return temp.m_data;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#findExact(java.lang.String)
	 */
	@Override
	public TrieObjType optimization2FindExact(StateVariables[] key, int i, int j) {

		if (i >= key.length) {
			if (null != m_data) {
				return m_data;
			} else {
				throw new RuntimeException(
						"reached the end of key(I think) but the element is not here "
								+ key);
			}
		}

		if (j >= key[i].size()) {
			return optimization2FindExact(key, i + 1, 0);
		}

		return optimization2findAtSonExact(key, i, j);

	}

	/**
	 * @param ret
	 * @param keyFirstChar
	 * @param keySuffix
	 */
	private TrieObjType optimization2findAtSonExact(StateVariables[] key,
			int i, int j) {
		TrieObjType ret = null;
		TrieInterface<TrieObjType> son = m_child[arithValToLoc.get(key[i]
				.get(j))];
		if (son != null) {
			ret = son.optimization2FindExact(key, i, j + 1);
		}
		return ret;
	}

	@Override
	public TrieObjType optimization3FindExact(StateVariables[] key) {

		int location = -1;
		ArithValue key_val;
		Trie<TrieObjType> temp = this;
		for (int i = 0; i < key.length; ++i) {
			for (int j = 0; j < key[i].size(); ++j) {
				key_val = key[i].get(j);

				location = arithValToLoc.get(key_val);
				if (temp.m_child[location] == null) {
					return null;
				}
				temp = temp.m_child[location];

			}// end of for(int j
		}

		return temp.m_data;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see fsb.trie.TrieInterface#size()
	 */
	@Override
	public int size() {
		int ret = 0;
		if (isEmpty()) {
			throw new RuntimeException("why is this element empty?");
		}
		if (m_data != null) {
			ret = 1;
		}
		for (int i = 0; i < LOCATIONS; ++i) {
			ret += (m_child[i] == null) ? 0 : m_child[i].size();
		}

		return ret;

	}

	/**
	 * @param ret
	 * @param keyFirst
	 * @param keySuffix
	 */
	private void optimization2AllFromSonThatSubsume(HashSet<TrieObjType> ret,
			StateVariables[] key, ArithValue keyFirst, int i, int j) {
		if (ret.size() >= this.m_max_subsume_for_elem_search) {
			return;
		}
		TrieInterface<TrieObjType> son = m_child[arithValToLoc.get(keyFirst)];
		if (son != null) {
			ret.addAll(son.optimization2AllThatSubsume(key, i, j + 1));
		}
	}

	/**
	 * note that if this is changed {@link #charToLoc} and {@link #locToChar}
	 * should be also changed
	 */
	static final private int LOCATIONS = 3;
	static final private Map<Character, Integer> charToLoc = new HashMap<Character, Integer>() {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;

		{
			put('0', 0);
			put('1', 1);
			put('*', 2);
		}
	};

	static final private Map<Integer, Character> locToChar = new HashMap<Integer, Character>() {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;

		{
			put(0, '0');
			put(1, '1');
			put(2, '*');
		}
	};
	static final private Map<ArithValue, Integer> arithValToLoc = new HashMap<ArithValue, Integer>() {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;

		{
			put(DetArithValue.getInstance(0), 0);
			put(DetArithValue.getInstance(1), 1);
			put(NondetArithValue.getInstance(), 2);
		}
	};

	static final private Map<Integer, ArithValue> locToArithVal = new HashMap<Integer, ArithValue>() {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;

		{
			put(0, DetArithValue.getInstance(0));
			put(1, DetArithValue.getInstance(1));
			put(2, NondetArithValue.getInstance());
		}
	};

	protected Trie<TrieObjType>[] m_child;
	protected Trie<TrieObjType> m_father;
	protected TrieObjType m_data; // only for the end nodes.
	private static int m_max_subsume_for_elem_search = -1;
	static protected char[] m_temp_key_buffer;
	static protected int m_temp_key_buffer_lock = -1;

	 @Override
	 public Iterator<TrieObjType> iterator() {
	 // TODO Auto-generated method stub
	
	 return new TrieIterator(this);
	 }

	class TrieIterator implements Iterator<TrieObjType> {
		Trie<TrieObjType> m_current;
		boolean m_at_next = false;

		public TrieIterator(Trie<TrieObjType> tr) {
			m_current = tr;
		}

		@Override
		public boolean hasNext() {
			if (m_current == null) {
				return false;
			}
			if(m_at_next){
				return (m_current.m_data == null);
			}
			TrieObjType obj = findNext();
			m_at_next = true;
			return (obj != null);
		}

	/**
		 * @return
		 */
		private TrieObjType findNextAtSon() {
			if (m_current == null) {
				return null;
			}
			while (m_current.m_data == null) {
				for (int i = 0; i < m_current.m_child.length; ++i) {
					if (m_current.m_child[i] != null) {
						m_current = m_current.m_child[i];
						break;
					}
				}
				if (m_current.isEmpty()) {
					return null;
				}

			}// end while(true)
			return m_current.m_data;
		}

		/**
		 * assuming not atnext
		 * 
		 * @return
		 */
		private TrieObjType findNext() {
			if (m_current == null) {
				return null;
			}
			if (m_current.m_data == null) {
				return findNextAtSon();
			}
			getNextSibling();
			return findNextAtSon();
		}

		/**
		 * 
		 */
		private void getNextSibling() {
			if (m_current == null) {
				return;
			}
			Trie<TrieObjType> father = m_current.m_father;
			if (father == null) {
				m_current = null;return;
			}
			int curr_i;
			for (curr_i = 0; curr_i < father.m_child.length; ++curr_i) {
				if (father.m_child[curr_i] == m_current) {
					break;
				}
			}
			if (curr_i >= father.m_child.length) {
				throw new RuntimeException("discovered adoupted?! :(");
			}
			if (curr_i + 1 >= father.m_child.length) {
				m_current = father;
				getNextSibling();
				return;
			}
			m_current = father.m_child[curr_i + 1];
		}

		@Override
		public TrieObjType next() {
			if (m_current == null) {
				return null;
			}
			if (m_at_next) { // if has next was activated and found next already
				m_at_next = false;
				return m_current.m_data;
			}
			TrieObjType obj = findNext();

			m_at_next = false;

			return obj;
		}

		@Override
		public void remove() {
			throw new RuntimeException("Not Yet Implemented");
		}

		public TrieObjType getCurrent() {
			if (m_current == null) {
				return null;
			}
			return m_current.m_data;
		}

	}

	@Override
	public String toString() {
		StringBuilder ret = new StringBuilder();
		ret.append("<");
		toStringFromSons(ret, "");
		ret.append(">");
		return ret.toString();
	}

	/**
	 * @param ret
	 * @return
	 */
	private void toStringFromSons(StringBuilder ret, String key) {
		if (m_data != null) {
			ret.append("\"").append(key).append("\"->").append(m_data).append("\n");
			return;
		}
		Boolean flag = true;
		for (int i = 0; i < m_child.length; ++i) {
			if (m_child[i] != null) {
				if (!flag) {
					ret.append(", ");
				} else {
					flag = false;
				}
				m_child[i].toStringFromSons(ret,key + locToChar.get(i));
			}
		}

		return;
	}

	public static int getMaxSubsumeForElemSearch() {
		return m_max_subsume_for_elem_search;
	}

	public static void setMaxSubsumeForElemSearch(
			int max_subsume_for_elem_search) {
		Trie.m_max_subsume_for_elem_search = max_subsume_for_elem_search;
	}

	private class Touple<T1, T2> {
		Touple(T1 t1, T2 t2) {
			this.t1 = t1;
			this.t2 = t2;
		}

		public T1 t1;
		public T2 t2;
	}
}
