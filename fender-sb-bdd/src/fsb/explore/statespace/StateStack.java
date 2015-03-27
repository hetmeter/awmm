package fsb.explore.statespace;

import java.util.Stack;

import fsb.explore.State;

public class StateStack implements StatesToExploreInterface {
private Stack<State> m_global_stack = new Stack<State>();
private Stack<State>[] m_atomics_stack = null;
private int m_process_might_be_in_atomic = -1;

public StateStack(int num_processes) {
	m_atomics_stack = new Stack[num_processes];
	for(int i=0; i<m_atomics_stack.length;i++){
		m_atomics_stack[i] = new Stack<State>();
	}
}

/* (non-Javadoc)
 * @see fsb.explore.statespace.StatesToExploreInterface#isEmpty()
 */
@Override
public boolean isEmpty() {
	if(!m_global_stack.isEmpty()){
		return false;
	}
	if(-1 == m_process_might_be_in_atomic){
		return true;
	}
	for(int i=0; i<m_atomics_stack.length;i++){
		m_process_might_be_in_atomic = (m_process_might_be_in_atomic + 1)
				%  m_atomics_stack.length;
		
		if(!m_atomics_stack[m_process_might_be_in_atomic].isEmpty()){
			return false;
		}
	}
	m_process_might_be_in_atomic = -1;
	throw new RuntimeException("something might be worng m_process_might_be_in_atomic was not -1 and yet the stack was empty");
}

/* (non-Javadoc)
 * @see fsb.explore.statespace.StatesToExploreInterface#pop()
 */
@Override
public State pop() {
	// TODO Auto-generated method stub
	if(-1 == m_process_might_be_in_atomic){
		return m_global_stack.pop();
	}
	
	State ret = m_atomics_stack[m_process_might_be_in_atomic].pop();
	if(m_atomics_stack[m_process_might_be_in_atomic].isEmpty()){
		// the current process has no more States on stack
		for(int i=0; i<m_atomics_stack.length;i++){
			m_process_might_be_in_atomic = (m_process_might_be_in_atomic + 1)
					%  m_atomics_stack.length;
			
			if(!m_atomics_stack[m_process_might_be_in_atomic].isEmpty()){
				return ret;
			}
		}
		m_process_might_be_in_atomic = -1;
	}
	return ret;
}

/* (non-Javadoc)
 * @see fsb.explore.statespace.StatesToExploreInterface#numStateOnStackForProcess(int)
 */
@Override
public int numStateOnStackForProcess(int i){
	return m_atomics_stack[i].size();
}

/* (non-Javadoc)
 * @see fsb.explore.statespace.StatesToExploreInterface#size()
 */
@Override
public int size() {
	int ret = m_global_stack.size();
	for(int i=0; i<m_atomics_stack.length;i++){
		ret += m_atomics_stack[i].size();
	}
	return ret;
}

/* (non-Javadoc)
 * @see fsb.explore.statespace.StatesToExploreInterface#push(fsb.explore.State)
 */
@Override
public void push(State state) {
	int in_atomic_section = state.inAtomicSection();
	if(-1 != in_atomic_section){
		m_atomics_stack[in_atomic_section].push(state);
		if(-1 == m_process_might_be_in_atomic){
			m_process_might_be_in_atomic = in_atomic_section;
		}
		return;
	}
	m_global_stack.push(state);
}

}
