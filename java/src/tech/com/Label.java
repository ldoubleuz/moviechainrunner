package tech.com;
import java.util.HashSet;
import java.util.Set;

public class Label {

	Set<Integer> ls;
	
	public Label(int l) {
		ls = new HashSet<Integer>();
		ls.add(l);
	}
	
	public Label(Label l) {
		ls = new HashSet<Integer>(l.ls);
	}
	
	public Label(Label l1, Label l2) {
		ls = new HashSet<Integer>();
		ls.addAll(l1.ls);
		ls.addAll(l2.ls);
	}
	
	@Override
	public boolean equals(Object o) {
		if(!(o instanceof Label))
			return false;
		
		return ls.equals(((Label)o).ls);
	}
	
	@Override
	public String toString() {
		return ls.toString();
	}
	
	@Override
	public int hashCode() {
		return ls.hashCode();
	}
}
	