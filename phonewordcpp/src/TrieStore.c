#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "TrieStore.h"

int trie_node_count = 0;

/*
 * Create a trie node with the given parent. The root node has a 
 * null parent.
 */
TrieEntry* trie_create( TrieEntry* parent )
{
	TrieEntry* root;
	root = (TrieEntry*)malloc(sizeof(TrieEntry));
	memset(root, 0, sizeof(*root));
	root->parent = parent;
	trie_node_count++;

	return root;
}

/*
 * Insert a word into the trie.
 */
void trie_insert( TrieEntry* root, const char* word)
{
	if ( 0 == *word ) {
		root->end = 1;
		return;
	}

	if ( *word < '0' || *word > 'Z' ) {
		root->end = 1;
		return;
	}

	if ( root->ptr[*word - '0'] == 0 ) {
		root->ptr[*word - '0'] = trie_create(root);
		root->ptr[*word - '0']->c = *word;
	}

	trie_insert( root->ptr[*word - '0'], &word[1] );
}

/*
 * Extract a word from the trie.
 */
int 
trie_get_word( TrieEntry* node, char* buffer, int buffer_len )
{
	int length = 0;
	int first_node = 0;
	int i = 0;

	while( node->parent ) {
		if ( length >= buffer_len ) {
			assert(0);
			return 0;
		}

		buffer[length] = node->c;
		node = node->parent;
		length++;
	}

	buffer[length] = 0;


	/* now reverse the word, since we got it backwards */
	for ( i = 0; i < ( length >> 1 ); ++i ) {
		char c;
		c = buffer[i];
		buffer[i] = buffer[length - i - 1];
		buffer[length - i - 1] = c;
	}


	return length;
}

/*
 * Traverses the trie, following it using a single letter.
 * Returns null if adding the letter would not result in a word
 * that exists in the trie.
 */
TrieEntry* trie_follow( TrieEntry* root, char c )
{
	assert(root);

	if ( c >= '0' && c <= 'Z' ) {
		root = root->ptr[c - '0'];
	}

	return root;
}

/*
 * Recursively destroy the trie.
 */
void trie_destroy( TrieEntry* root )
{
	char c;

	if ( 0 == root ) {
		return;
	}

	for( c = '0'; c <= 'Z'; ++c ) {
		trie_destroy( root->ptr[c - '0'] );
	}

	free(root);
	trie_node_count--;
}

