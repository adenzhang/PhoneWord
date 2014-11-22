package org.challenge.phoneword;

import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.List;

public interface NumberWordMatcher {
	void setDictionary(WordSupplier dict);
	void findWord(String digits, PrintStream writer);
}
