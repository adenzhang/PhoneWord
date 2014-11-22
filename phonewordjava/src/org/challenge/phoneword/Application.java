package org.challenge.phoneword;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Scanner;


public class Application {

	static void print_usage()
	{
		String progname = "program";
	    String s;
	    s = "Usage: " + progname + " [OPTIONS] [numbers]\n";
	    s += "Find words hidden inside phone numbers (separated by comma).\n\n";
	    s += "  If nubmers are read via stdin, two consecutive empty lines terminate input.\n";
	    s += " -d <dictionary> File to use as dictionary (Default: /usr/share/dict/words)\n";
	    s += " -w mininum word length (Default: 2)\n";
	    s += "\nExample:\n";
	    s += " " + progname + " 2255.63,7292650782\n";

	    System.out.println(s);
	}

	public static void run(String[] args) {
	    String dictname="";
	    String number="";
	    for(int i=1; i<args.length; ++i) {
	        if( args[i] == "-n" ) {
	            ++i;
	            dictname = args[i];
	        }else if( args[i] == "-h"
	                  || args[i] == "-?"
	                  || args[i] == "--help") {
	            print_usage();
	        }else{
	            number = args[i];
	        }
	    }
	    if( number.isEmpty() ) {
	        String prev="a";
	        String s;
	        Scanner scanIn = new Scanner(System.in);
	        while( scanIn.hasNext() ) { // exit reading on two consecutive empty lines.
	        	s = scanIn.nextLine();
	        	if(!s.isEmpty() || !prev.isEmpty()) 
	        		break;
	        	number += s;
	        	prev = s;
	        }
	        scanIn.close();
	    }
	    
	    NumberWordMatcher matcher = new HashNumberWordMatcher();
	    WordSupplier ws = new FileWordSupplier();
	    if( ws.load(dictname) <=0 ) {
	    	System.out.println("Failed to load dictionary");
	    	return;
	    }
	    matcher.setDictionary(ws);
	    
	    char DEL=',';
	    for(int currPos = 0; currPos<number.length(); ++currPos) {
	        int pos = number.indexOf(DEL, currPos);
	        if( pos == -1 ) {
	            pos =  number.length();
	        }
	        String num = number.substring(currPos, pos);
	        System.out.println(num);
	        matcher.findWord(num, System.out);

	        currPos = pos;
	    }
	}
	public static void main(String[] args) {
		run(new String[]{"program", "2255.63"});
	}
}
