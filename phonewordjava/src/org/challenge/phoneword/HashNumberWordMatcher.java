package org.challenge.phoneword;

import java.io.PrintStream;
import java.io.Reader;
import java.io.Writer;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

public class HashNumberWordMatcher implements NumberWordMatcher {
	public HashNumberWordMatcher() {
		Map<Character, String> d2a = new HashMap<Character, String>(); // digit to letter
		d2a.put('2', "ABC");
		d2a.put('3', "DEF");
		d2a.put('4', "GHI");
		d2a.put('5', "JKL");
		d2a.put('6', "NMO");
		d2a.put('7', "PQRS");
		d2a.put('8', "TUV");
		d2a.put('9', "WXYZ");
		
		a2d = new HashMap<Character, Character>();
		for(Map.Entry<Character, String> e: d2a.entrySet()) {
			Character c = e.getKey();
			for(Character a : e.getValue().toCharArray()) {
				a2d.put(a, c);
			}
		}
		
		n2w = new HashMap<String, List<String>>();
	}
	// build a map to store all pair of (number, word)
	public void setDictionary(WordSupplier dict) {
		words = dict.getWordList();
		StringBuffer sb = new StringBuffer();
		for(String w :words) {
			sb.setLength(0);
			// get the number in string
			for(Character c : w.toCharArray()) {
				Character d = a2d.get(c);
				if( d != null)
					sb.append(d);
			}
			if( sb.length() >= minWordLen ) { // at least 2 digits
				// put number/word pair in to map
				String num = sb.toString();
				List<String> ws = n2w.get(num);
				if( ws == null ) {
					ws = new LinkedList<String>();
					n2w.put(num, ws);
				}
				ws.add(w);
			}
		}		
	}
	boolean isDelimiter(char ch) {
		return ch > '9' || ch < '2';
	}
	public void findWord(String adigits, PrintStream p) {
		//-- remove non-digit
		StringBuilder sb = new StringBuilder();
		for(int i=0; i<adigits.length(); ++i) {
			char ch = adigits.charAt(i);
			if( Character.isDigit(ch) )
				sb.append(ch);
		}
		String digits = sb.toString();
		
		int N = digits.length();
		List[][] m = new List[N][N];
		for(int i=0; i<N; ++i) 
			for(int j=0; j<N; ++j)
				m[i][j] = new LinkedList();
		
		for(int i=0; i<N-1; ++i) {
			if( isDelimiter(digits.charAt(i)) ) continue;
			if( isDelimiter(digits.charAt(i+1)) ) {
				++i;
				continue;
			}
			
			for(int j=i+minWordLen; j<N; ++j) {
				matchWord(digits.substring(i, j), i,j-i, m);
				if( isDelimiter(digits.charAt(j)) ) break;
			}
		} // for i
		
		printMatrix(m, p);
		
//		constructWords(0, digits, m, "", p);
		
	}
	void matchWord(String digits, int startpos, int length, List[][] m) {
		List<String> w = n2w.get(digits);
		if(w != null)
			m[length][startpos] = w;
	}
	public void printMatrix(List[][] m, PrintStream p) {
		p.println("<startPos, length: matched Strings>");
        for(int i=minWordLen; i<m.length; ++i) {
            for(int j=0; j<m[0].length; ++j) {
                if( m[i][j].size() > 0 ) {
                    p.printf("<%d, %d:", j, i);
                    for(String s: (List<String>) m[i][j]) {
                    	p.print(s + " ");
                    }
                    p.println(">");
                }
            }
        }
	}
	void constructWords(int startpos, String digits, List[][] m, String pre, PrintStream p) {
		int NR = m.length, NC = m[0].length;
		char DEL = '-';
		if( startpos == digits.length() ) { // end of string. print result
			StringBuilder ss = new StringBuilder(); 
			// remove unnecessary DEL
			int i=0;
//			while( pre.charAt(i++) == DEL ) ;  // delete leading DEL
//			--i;
//			for(; i<pre.length(); ++i) {
//				if( pre.charAt(i) == DEL && (pre.charAt(i+1) == DEL 
//						|| (!Character.isLetter(pre.charAt(i-1)) && Character.isLetter(pre.charAt(i+1)))) ){
//					continue;
//				}
//				ss.append(pre.charAt(i));
//			}
			p.println(ss.toString());
		}
        // search for next matched word
        int minStep = 0;
        int minStart = startpos;
        for(int j=startpos; j<NC && minStep==0; ++j) {
            for(int i=minWordLen; i<NR; ++i) {
                if( m[i][j].size() > 0 ) {
                    String w = pre + DEL + digits.substring(startpos, j) + DEL;
                    for(String s : (List<String>) m[i][j] ) {
                        constructWords(j+i, digits, m, w + s, p);
                    }
                    if( minStep == 0 ) {
                        minStep = i;
                        minStart = j;
                    }
                }
            }
        }
        if( minStep == 0 ) {
            constructWords(digits.length(), digits, m, pre + DEL + digits.substring(startpos, digits.length()), p);
        }else{
            // search for next match with starting position before minStep
            for(int j=minStart+1; j<=minStep; ++j) {
                for(int i=minWordLen; i<NR; ++i) {
                    if( m[i][j].size() > 0 ) {
                        constructWords(j, digits, m, pre + DEL + digits.substring(startpos, j), p);
                        return;
                    }
                }
            }
        }	
	}
	
	protected List<String> words;
	protected Map<Character, Character> a2d;  // letter to digit
	protected Map<String, List<String>> n2w;  // number 2 word
	int minWordLen = 2;
}
