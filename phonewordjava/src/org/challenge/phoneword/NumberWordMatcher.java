package org.challenge.phoneword;

import java.io.PrintWriter;

public interface NumberWordMatcher {
	void setDictionary(WordSupplier dict);
	void findWord(String digits, PrintWriter writer);
}
