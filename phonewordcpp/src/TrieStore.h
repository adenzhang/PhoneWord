#ifndef TRIESTORE_H
#define TRIESTORE_H

/* Trie Data Structure
 * 
 * Implemented by Steve Hanov 
 * steve.hanov@mail.com
 *
 * Note: If you are using this in your own project it would
 * be polite to let me know!
 */

typedef struct _TrieEntry {
	char c;
	int end;
	struct _TrieEntry* parent;
	struct _TrieEntry* ptr[50];
} TrieEntry;

/*
 * Create a trie.
 *
 * Parameters:
 *    parent -- NULL, or child to create it from. 
 *       (normally call it with null, trie_insert uses this parameter
 *        to add to the trie.)
 */

TrieEntry* trie_create( TrieEntry* parent );

/* 
 * Insert into the trie.
 *
 * Example:
 *   TrieEntry* trie = trie_create(NULL);
 *   trie_insert(trie, "keyword");
 */
void trie_insert( TrieEntry* root, const char* word);

/*
 * follow a chain in the trie.
 *
 * Returns node in trie representing word so far, or NULL
 * if adding the letter does not result in a word.
 * 
 * Example:
 *   trie_insert(trie, "steve");
 *   assert( NULL != path = trie_follow(root, 's'));
 *   assert( NULL != path = trie_follow(path, 't'));
 *   assert( NULL == path = trie_follow(path, 'W'));
 */
TrieEntry* trie_follow( TrieEntry* root, char c );

/*
 * Retrieve a word from a node obtained from trie_follow().
 *
 * Example:
 *   trie_insert(trie, "the");
 *   trie = trie_follow(trie, 't');
 *   trie = trie_follow(trie, 'h');
 *   trie = trie_follow(trie, 'e');
 *   char buffer[10];
 *   trie_get_word(trie, buffer, 10);
 *   assert(0 == strcmp(buffer, "the"));
 */
int trie_get_word( TrieEntry* node, char* buffer, int buffer_len );


/* 
 * destroy the trie data structure.
 */

void trie_destroy( TrieEntry* root );

#endif

