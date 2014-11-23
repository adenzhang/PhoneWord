#PhonWord

Problem:
Find the hidden words in a phone number.
Input : a list of phone numbers with comma as delimiters. if number is not specified as argument, stdin will be expected to input numbers and two consecutive empty lines terminates the input.
Output: each line prints a combination original digits and matched words. the first line is the original phone numbers.

Solution:
There's an exsiting algorithm emplemented with trie by [Steve Hanov](http://stevehanov.ca/blog/index.php?id=9). But the perfromance is not good. So I designed another hash based algorithm (in java, and in c++ file phonewordcpp/src/main.cpp).
The basic idea is to construct a hash map to store all the possible digit numbers. The hash map is like {"225":["BAL", "CAL], ...} which is constructed from dictionary (Default is /usr/share/dict/words).
Dynamic programming is used in both algorithms to store matched words.


Build-cpp:
cmake is used to build the code. Please refer to cmake docs.


Build-java:
use eclipse to import the poject and add library JUnit4.

