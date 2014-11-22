package org.challenge.phoneword;

import java.util.List;

public interface WordSupplier {
	// return number of feed words
	int load(String a);
	List<String> getWordList();
}
