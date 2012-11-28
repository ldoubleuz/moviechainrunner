package tech.com; 
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

public class Weighted {
	
	static String file = "titles";
	static final String nl = System.getProperty("line.separator");

	/*
	 * See Graph_Format.txt
	 */
	
	public static void main(String args[]) throws IOException {
		Map<Integer,String> lines = importLines();
		System.out.println("loaded");
		Map<Integer,Set<Overlap>> overlaps = Overlap.overlap(lines);
		System.out.println("overlaps");
		
		
		List<Integer> useless = foundUseless(overlaps,flip(overlaps));
		for(int i : useless) {
			lines.remove(i);
		}
		OutputStreamWriter fout = new OutputStreamWriter(new FileOutputStream(new File("cleaned")));
		for(String s : lines.values()) {
			fout.write(s);
			fout.write(nl);
		}
		fout.flush();
		fout.close();
		

		

	}
	
	public static void writeGraph(Map<Integer,Set<Overlap>> g) throws IOException {
		
		String root = new File("").getAbsolutePath();
		root = root.substring(0, root.lastIndexOf(File.separator));
		File f = new File(root + File.separator + "c" + File.separator + file);
		System.out.println(f.getAbsoluteFile());
		f.createNewFile();
		OutputStream out = new FileOutputStream(f);
		
		out.write(intToTwoBytes(g.size()));
		
		for(Entry<Integer,Set<Overlap>> e : g.entrySet()) {
			out.write(intToTwoBytes(e.getKey()));
			out.write(intToTwoBytes(e.getValue().size()));
			for(Overlap o : e.getValue()) {
				out.write(intToTwoBytes(o.suf));
				out.write(o.len & 0xff);
				out.write(0);
			}
		}
		out.flush();
		out.close();
	}
	
	public static byte[] intToTwoBytes(int i) {
		byte[] res = new byte[2];
		res[0] = (byte)i;
		res[1] = (byte)(i>>8);
		
		return res;
	}
	
	public static Map<Integer,Set<Integer>> flip(Map<Integer,Set<Overlap>> g) {
		Map<Integer,Set<Integer>> flipped = new HashMap<Integer,Set<Integer>>();
		for(Integer i : g.keySet())
			flipped.put(i, new HashSet<Integer>());
		for(Entry<Integer,Set<Overlap>> e : g.entrySet())
			for(Overlap o : e.getValue())
				flipped.get(o.suf).add(e.getKey());
		return flipped;
	}
	
	public static List<Integer> foundUseless(Map<Integer,Set<Overlap>> g, Map<Integer,Set<Integer>> gflip) {
		
		List<Integer> useless = new LinkedList<Integer>();
		Map<Integer,Integer> reach_here = new HashMap<Integer,Integer>();
		Map<Integer,Integer> reached_from_here = new HashMap<Integer,Integer>();
		Map<Integer,Integer> totals = new HashMap<Integer,Integer>();
		
		for(Integer i : g.keySet()) {
			Set<Integer> total = new HashSet<Integer>();
			
			
			AtomicInteger count = new AtomicInteger(0);
			dfs_reached_from_here(i,g,(Set<Integer>)new HashSet<Integer>(),new LinkedList<Integer>(), count,total);
			reached_from_here.put(i,count.get());
			
			count = new AtomicInteger(-2);
			dfs_reach_here(i,gflip,(Set<Integer>)new HashSet<Integer>(),new LinkedList<Integer>(), count,total);
			reach_here.put(i, count.get());
			
			totals.put(i, total.size());
			
			System.out.println(i + " " + reached_from_here.get(i) + " " + reach_here.get(i) + " " + totals.get(i));
			
			if(totals.get(i) < 250)
				useless.add(i);
		}
		
		return useless;
	}
	
	private static void dfs_reach_here(Integer v, Map<Integer, Set<Integer>> g, Set<Integer> visited, LinkedList<Integer> path, AtomicInteger state,Set<Integer> t) {
		
		if(visited.contains(v))
			return;
		
		visited.add(v);
		path.add(v);
		state.incrementAndGet();
		t.add(v);
		
		for(Integer n : g.get(v))
			dfs_reach_here(n,g,visited,path,state,t);
		path.remove(v);		
	}

	private static void dfs_reached_from_here(Integer v, Map<Integer,Set<Overlap>> g, Set<Integer> visited, LinkedList<Integer> path, AtomicInteger state,Set<Integer> t) {
		
		if(visited.contains(v))
			return;
		
		visited.add(v);
		path.add(v);
		state.incrementAndGet();
		t.add(v);
		
		for(Overlap n : g.get(v))
			dfs_reached_from_here(n.suf,g,visited,path,state,t);
		path.remove(v);
	}

	public static Map<Integer,String> importLines() throws IOException {
		Map<Integer,String> lines = new LinkedHashMap<Integer,String>();		
		
		File f = new File(file);
		BufferedReader br = new BufferedReader(new FileReader(f));
		String s;
		int index = 0;
		while((s = br.readLine()) != null)
			lines.put(new Integer(index++),s);
		
		return lines;		
	}

}
