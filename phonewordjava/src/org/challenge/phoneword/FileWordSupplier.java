package org.challenge.phoneword;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

public class FileWordSupplier implements WordSupplier {

	public FileWordSupplier() {
		words = new LinkedList();
	}
	// return number of words; -1 for error.
	@Override
	public int load(String filepath) {
		if( filepath.isEmpty() )
			filepath = "/usr/share/dict/words";
		int n = 0;
		try{
			String line;
			BufferedReader br = new BufferedReader(new FileReader(filepath));
			while( (line = br.readLine()) != null ) {
				if( addWord(line) )
					++n;
			}
			br.close();
		}catch(FileNotFoundException e) {
			e.printStackTrace();
			return -1;
		} catch (IOException e) {
			e.printStackTrace();
			return -1;
		}
		
		return n;
	}
	@Override
	public 	List<String> getWordList() {
		return words;
	}
	// @return true if added.
	public boolean addWord(String word) {
		word = word.trim().toUpperCase();
		if( word.matches("^[a-zA-Z]+$") ) {
			words.add(word);
			return true;
		}else{
			return false;
		}
	}
	private List<String> words; // in upper case
}
