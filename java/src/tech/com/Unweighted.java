package tech.com;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

public class Unweighted {
		
	static String file = "cleaned";
	static final String nl = System.getProperty("line.separator");

	/*
	 * See Graph_Format.txt
	 */
	
	public static void main(String args[]) throws IOException {

		Map<Integer,List<Integer>> overlaps = unweighted();
		
		EdgeComparator ec = new EdgeComparator(overlaps);
		for(List<Integer> l : overlaps.values()) {
			Collections.sort(l, ec);
		}
		
		for(int i=0;i<30;i++)
			System.out.println(i + " " + overlaps.get(i).size());
										
		writeGraph(overlaps);
	}
	
	public static Map<Integer,List<Integer>> unweighted() throws IOException {
		Map<Integer,String> lines = importLines();
		System.out.println("loaded");
		Map<Integer,Set<Overlap>> overlaps = Overlap.overlap(lines);
		System.out.println("overlaps");
		
		Map<Integer,List<Integer>> g = new LinkedHashMap<Integer,List<Integer>>();
		for(Entry<Integer,Set<Overlap>> e : overlaps.entrySet()) {
			List<Integer> edges = new ArrayList<Integer>();
			for(Overlap o : e.getValue())
				edges.add(o.suf);
			
			g.put(e.getKey(), edges);
		}
		
		return g;


	}
	
	public static void writeGraph(Map<Integer,List<Integer>> g) throws IOException {
		
		String root = new File("").getAbsolutePath();
		root = root.substring(0, root.lastIndexOf(File.separator));
		File f = new File(root + File.separator + "c" + File.separator + file + "_unweighted");
		System.out.println(f.getAbsoluteFile());
		f.createNewFile();
		OutputStream out = new FileOutputStream(f);
		
		System.out.println(g.size());
		out.write(intToTwoBytes(g.size()));
		
		for(Entry<Integer,List<Integer>> e : g.entrySet()) {
			out.write(intToTwoBytes(e.getKey()));
			out.write(intToTwoBytes(e.getValue().size()));
			for(Integer i : e.getValue()) {
				out.write(intToTwoBytes(i));
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
	
	public static Map<Integer,Set<Integer>> flip_set(Map<Integer,Set<Overlap>> g) {
		Map<Integer,Set<Integer>> flipped = new HashMap<Integer,Set<Integer>>();
		for(Integer i : g.keySet())
			flipped.put(i, new HashSet<Integer>());
		for(Entry<Integer,Set<Overlap>> e : g.entrySet())
			for(Overlap o : e.getValue())
				flipped.get(o.suf).add(e.getKey());
		return flipped;
	}
	
	private static int count_reachable(Integer v, Map<Integer, List<Integer>> g, Set<Integer> visited) {
		
		if(visited.contains(v))
			return 0;
		
		visited.add(v);
		int total = 1;
		
		for(Integer n : g.get(v))
			total += count_reachable(n,g,visited);
		
		return total;
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
	
	public static Map<Integer,List<Integer>> flip_list(Map<Integer,List<Integer>> g) {
		Map<Integer,List<Integer>> flipped = new HashMap<Integer,List<Integer>>();
		for(Integer i : g.keySet())
			flipped.put(i, new ArrayList<Integer>());
		for(Entry<Integer,List<Integer>> e : g.entrySet())
			for(Integer o : e.getValue())
				flipped.get(o).add(e.getKey());
		return flipped;
	}
	
	private static class EdgeComparator implements Comparator<Integer> {
		
		int[] reachable;
		int[] reachable_from;
		Map<Integer,List<Integer>> g;
		
		private EdgeComparator(Map<Integer,List<Integer>> g) {
			reachable = new int[g.size()];
			reachable_from = new int[g.size()];
			this.g = g;
			Map<Integer,List<Integer>> flipped = flip_list(g);
			
			for(Entry<Integer,List<Integer>> e : g.entrySet()) {
				reachable[e.getKey()] = count_reachable(e.getKey(),g,(Set<Integer>)new HashSet<Integer>());
				reachable_from[e.getKey()] = count_reachable(e.getKey(),flipped,(Set<Integer>)new HashSet<Integer>()) - 2;
			}					
		}


		@Override
		public int compare(Integer arg0, Integer arg1) {
			int diff = reachable[arg1] - reachable[arg0];
			if(diff == 0) {
				int diff2 = reachable_from[arg0] - reachable_from[arg1];
				if(diff2 == 0)
					return g.get(arg0).size() - g.get(arg1).size();
				else
					return diff2;
			}
			
			return diff;
		}

	}	
}
