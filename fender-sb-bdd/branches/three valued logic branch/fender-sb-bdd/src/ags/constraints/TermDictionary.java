package ags.constraints;

import java.util.HashMap;

public class TermDictionary {
	private HashMap<Formula, Integer> termsMap;
	private HashMap<Integer, Formula> reverseTermsMap;
	private int num = 0;
	
	public TermDictionary()
	{
		termsMap = new HashMap<Formula, Integer>();
		reverseTermsMap  = new HashMap<Integer, Formula>(); 
	}
	
	public boolean isIn(Formula f)
	{
		return termsMap.get(f) != null;
	}
	
	public int add(Formula f)
	{
		termsMap.put(f, num);
		reverseTermsMap.put(num++, f);
		return num - 1;
	}
	
	public int get(Formula f)
	{
		assert f != null;
		assert termsMap != null;
		return termsMap.get(f);
	}

	public Formula reverseGet(int i) {
		return reverseTermsMap.get(i);
	}
}
