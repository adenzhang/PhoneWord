package org.challenge.phoneword.test;

import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.PrintWriter;

import junit.framework.Assert;

import org.challenge.phoneword.FileWordSupplier;
import org.challenge.phoneword.HashNumberWordMatcher;
import org.challenge.phoneword.NumberWordMatcher;
import org.challenge.phoneword.WordSupplier;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class PhoneNumberWordMatchTest {
	NumberWordMatcher matcher;

	@Before
	public void setUp() throws Exception {
		matcher = new HashNumberWordMatcher();
		
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void test() {
//		fail("Not yet implemented");
		WordSupplier ws = new FileWordSupplier();
		assertTrue("Failed to load default dictionary file", ws.load("") > 0);
		

		matcher.setDictionary(ws);
		
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		
//		matcher.findWord("", baos);
//		assertTrue(baos.size() == 0);
		
//		matcher.findWord("2255.63", baos);
//		assertTrue(baos.size() > 0);
	}

}
