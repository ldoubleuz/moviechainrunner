package tech.com;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.Map;

public class Check {
	
	static String file = "cleaned";
	
	static String s = "2573,2574,1347,1983,167,1705,1708,2344,1460,2293,404,2082,3006,1042,583,1907,1225,1241,1149,1150,1754,569,1428,633,792,625,1095,3109,608,3206,494,2254,945,1765,1580,1732,62,3182,2314,880,2731,1635,441,424,2086,2004,621,634,1094,769,994,2059,1183,13,3091,984,453,1589,3028,624,2931,2009,2057,2029,2548,890,1151,16,19,3079,1911,273,943,1552,2309,1616,319,1991,1587,879,2185,1134,2242,636,461,1585,1176,752,1303,831,962,661,2166,896,1256,1051,364,2229,893,1532,580,1103,1774,771,773,1010,1809,722,1414,1816,1590,1089,560,2006,815,998,3000,229,252,1895,1680,390,1474,622,1761,623,630,240,640,1767,1692,1762,531,1351,2048,573,1466,256,1908,2355,151,825,23,985,990,988,2334,2117,2076,1131,1124,587,1897,686,348,469,3214,1648,3215,3217,1724,846,718,2094,671,2122,1536,1058,1632,1295,2247,1067,777,3203,1546,1469,854,409,2927,2671,1399,1355,1356,374,1560,1262,457,2244,2123,277,276,1643,1044,2694,2695,2049,2516,1875,546,868,1929,314,750,1528,668,664,3196,197,377,106,3209,819,144,222,1447,2684,394,1467,1301,3230,1825,181,269,2812,2813,3185,2951,551,1305,1853,2637,1319,1489,707,708,704,1070,2986,2251,602,2307,2605,2504,635,638,3113,2354,2147,1586,3211,754,2130";

	public static void main(String args[]) throws IOException {
		Map<Integer,String> lines = importLines();
		
		String[] nums = s.split(",");
		boolean works = true;
		
		for(int i=0;i<nums.length - 1;i++) {
			String cur = lines.get(Integer.parseInt(nums[i]));
			String next = lines.get(Integer.parseInt(nums[i+1]));
			
			if(checkOverlap(cur.split(" "),next.split(" ")) == 0) {
				works = false;
				System.out.println("Oh no! " + cur + " " + next);	
			}
		}
		if(works)
			System.out.println("Whoo!");
		
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
