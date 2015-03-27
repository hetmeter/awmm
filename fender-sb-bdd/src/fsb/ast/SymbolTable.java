package fsb.ast;

import gnu.trove.map.TMap;
import gnu.trove.map.hash.THashMap;

import java.util.HashMap;
import java.util.Map;

public class SymbolTable {

	private static TMap<String, Integer> m_local_variables = new THashMap<String, Integer>();
	private static TMap<Integer, String> m_local_backward_variables = new THashMap<Integer, String>();
	private static TMap<String, Integer> m_global_variables = new THashMap<String, Integer>();
	private static TMap<Integer, String> m_global_backward_variables = new THashMap<Integer, String>();
	private static int m_localNum = 0;
	private static int m_globalNum = 0;

	public static int getOrCreateLocalVariable(String var) {
		Integer result = m_local_variables.get(var);
		if (result == null) {
			m_local_variables.put(var, m_localNum);
			m_local_backward_variables.put(m_localNum, var);
			return m_localNum++;
		} else {
			return result.intValue();
		}

	}

	public static String getLocalVariableName(int var) {
		return m_local_backward_variables.get(var);

	}
	
	public static int getOrCreateGlobalVariable(String var) {
		Integer result = m_global_variables.get(var);
		if (result == null) {
			m_global_variables.put(var, m_globalNum);
			m_global_backward_variables.put(m_globalNum, var);
			return m_globalNum++;
		} else {
			return result.intValue();
		}

	}

	public static String getGlobalVariableName(int var) {
		return m_global_backward_variables.get(var);

	}
	
	public static void reset(){
		m_local_variables = new THashMap<String, Integer>();
		m_local_backward_variables = new THashMap<Integer, String>();
		m_global_variables = new THashMap<String, Integer>();
		m_global_backward_variables = new THashMap<Integer, String>();
		m_localNum = 0;
		m_globalNum = 0;
		
	}
}
