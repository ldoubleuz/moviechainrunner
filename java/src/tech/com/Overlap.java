package tech.com;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;


public class Overlap {
	
	int len;
	Integer suf;
	List<Integer> internal;
	
	public Overlap(int l, Integer s) {
		len = l;
		suf = s;
		internal = new ArrayList<Integer>();
	}
	
	public Overlap(Overlap o) {
		len = o.len;
		suf = new Integer(o.suf);
		internal = new ArrayList<Integer>(o.internal);

	}
		
	public String toString() {
		return "(--(" + internal + ")-> " + suf + " :" + len + ")";
	}
	
	public boolean equals(Object o) {
		if(!(o instanceof Overlap))
			return false;
		
		return ((Overlap)o).suf.equals(suf);
	}
	
	@Override
	public int hashCode() {
		return suf.hashCode(); 
	}
	
	/**
	 * Where o is the second in the chain
	 * @param o
	 * @return
	 */
	public Overlap merge(Overlap o) {
		Overlap newO = new Overlap(o);
		newO.len += len;
		
		newO.internal.add(0,suf);
		newO.internal.addAll(0,internal);
		
		return newO;
	}
	
	public static Map<Integer,Set<Overlap>> overlap(Map<Integer,String> s) {
		Map<Integer,Set<Overlap>> overlaps = new LinkedHashMap<Integer,Set<Overlap>>();
		
		Map<Integer,Object> toks = new LinkedHashMap<Integer,Object>();
		
		for(Entry<Integer,String> e : s.entrySet())
			toks.put(e.getKey(), e.getValue().split(" "));

		for(Entry<Integer,Object> e : toks.entrySet()) {
			String[] pref = (String[]) e.getValue();
			Set<Overlap> l = new LinkedHashSet<Overlap>();
			for(Entry<Integer,Object> e2 : toks.entrySet()) {
				if(e.getKey().equals(e2.getKey()))
					continue;
				int i = checkOverlap(pref,(String[]) e2.getValue());
				if(i > 0)
					l.add(new Overlap(((String[])e2.getValue()).length - i,e2.getKey()));
			}
			overlaps.put(e.getKey(), l);
		}
		
		Set<Overlap> all = new LinkedHashSet<Overlap>();
		for(Entry<Integer,Object> e : toks.entrySet()) {
			String[] pref = (String[]) e.getValue();
			all.add(new Overlap(pref.length,e.getKey()));
		}
		overlaps.put(-1,all);
		
		
		return overlaps;
	}
	
	public static int checkOverlap(String[] pref, String[] suf) {
		int numCheck = pref.length < suf.length ? pref.length : suf.length;
		
		for(int i=1;i<=numCheck;i++) {
			boolean overlap = true;
			for(int j=0; overlap && j<i ; j++) {
				if(!pref[(pref.length - i) + j].equals(suf[j]))
					overlap = false;
			}
			if(overlap)
				return i;
		}		
		return 0;		
	}

}
