/****************************************************************************
 * spellophone
 *
 * Algorithm to check phone numbers for words.
 * by steve.hanov@mail.com
 *
 * Usage:
 * 
 *
 *
 * */

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TrieStore.h"

extern int trie_node_count;

/* Minimum word -- the minimum number of characters in a word. */
#define MINIMUM_WORD 2

typedef struct _StringList {
	char* str;
	struct _StringList* next;
} StringList;

/* Represents an entry in the solution. The entry can be put in a list, a 
   tree, or an array. */
typedef struct _Entry {
	char* str;
	int score;
	struct _Entry* lesser;
	struct _Entry* greater;
} Entry;

/* Options for the application from the command line. */
typedef struct _AppOptions {
	int use_default_dict;
	StringList* extra_dict_files;
	int show_scores;
	int lowest_first;
	int show_stats;
	const char* keymap[10];
} AppOptions;

/* 
 * This is passed around -- it contains the root node of all of the solutions.
 */
typedef struct _AppData {
	TrieEntry* root;
	int num_entries;
	Entry* entries;
	char* number;
	AppOptions options;
} AppData;

/* Check for these files in order if none are specified. */
static const char* default_dict_files[] = 
{
  "/etc/dictionaries-common/words",
  "/usr/share/dict/words",
  "/usr/dict/words",
  0
};

#ifdef TIME_IT
long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
#endif

/*
 * Display a usage message.
 */
void
print_usage( const char* progname )
{
	printf("Usage: %s [OPTIONS] digits\n", progname);
	printf("List the words hidden inside a phone number.\n\n");
	printf(" -#=LLL          Remap digit to letters.\n");
	printf(" -d <dictionary> File to use as dictionary (Default: /etc/dictionaries-common/words"); 
	printf("words)\n");
	printf("                 (Can be used multiple times)\n");
	printf(" -n              Do not use the default dictionaries. \n");
	printf(" -r              Show lower scoring entries first \n");
	printf(" -s              Show word scores \n");
	printf(" -v              Show statistics \n\n");
	printf("Example: \n");
	printf("   List all words in 737-8225 using 2 custom dictionaries and no Q or Z.\n\n");
	
	printf("      %s -n -d words.txt -d names.txt -7=PRS -9=WXY 737-8225\n",
			 progname);
}


/*
 * add an entry to a linked list of strings, and returns new head.
 *
 * example: list = stringList_add(list, "hello");
 */
StringList*
stringList_add( StringList* head, const char* str )
{
	StringList* node;

	node = (StringList*)malloc(sizeof(StringList));
	node->str = strdup(str);
	node->next = head;

	return node;
}

/* 
 * Free a linked list of strings.
 */
void
stringList_free( StringList* head )
{
	StringList* current = head;
	StringList* next = 0;

	while ( current ) {
		next = current->next;
		free(current->str);
		free(current);
		current = next;
	}
}

/*
 * Insert an entry into the tree and return the new  root. 
 *
 * root - root of tree.
 * node - node to insert
 * inserted - set to 1 if node was inserted successfully (not a duplicate)
 *
 * Returns new root on success.
 *
 * eg. usage: root = tree_insert( root, new_node, 0 )
 */
Entry* tree_insert( Entry* root, Entry* node, int* inserted )
{
	int comp;

	/* if first entry in tree, */
	if ( 0 == root ) {
	  /* automatically insert it. */
		if ( inserted ) {
			*inserted = 1;
		}
		return node;
	}

	comp = strcmp(root->str, node->str);

	if ( comp == 0 ) {
		/* item already in tree -- do not insert it. */
		free( node->str );
		free( node );
		if ( inserted ) {
			*inserted = 0;
		}
	} else if ( comp == -1 ) {
		root->greater = tree_insert( root->greater, node, inserted );
	} else {
		root->lesser = tree_insert( root->lesser, node, inserted );
	}

	return root;

}

/*
 * Convert a tree to an array.
 * 
 * Parameters:
 *    root: root of tree.
 *    array: pointer to start of array.
 *    max_items: Maximum items to convert
 *
 * Returns:
 *    number of items actually linearized.
 */
int tree_to_array( Entry* root, Entry** array, int max_items )
{
	int items_inserted = 0;

	if ( root == 0 ) {
		return 0;
	}

	if ( root->lesser ) {
		items_inserted += tree_to_array( root->lesser, &array[items_inserted], 
			max_items - items_inserted);
	}

	if ( max_items - items_inserted > 0 ) {
		array[items_inserted++] = root;
	}

	if ( root->greater ) {
		items_inserted += tree_to_array( root->greater, &array[items_inserted], 
			max_items - items_inserted);
	}

	return items_inserted;
}


/*
 * Purpose:
 *      Tries the given list of files in order until it opens one,
 *      then returns the FILE* of the open file.
 */
FILE* open_dict_file( const char* names[] ) 
{
	int i = 0;
	
	while ( names[i] ) {
		FILE* fd = 0;
		fd = fopen( names[i], "r" );
		if ( fd ) {
			return fd;
		}

		++i;
	}

	return 0;
}

/*
 * Read a dictionary file into the trie structure.
 *
 * Parameters:
 *      file: Dictionary file containing words separated by newlines.
 *      root: root of trie, created by trie_create()
 *      num_chars: words greater than this length are skipped.
 */
int read_dict( FILE* file, TrieEntry* root, int num_chars )
{
	char buffer[80];
	int num = 0;
	char* ch;

	while ( 0 != fgets( buffer, 80, file ) ) {
		// filter word
		// disallow words over num_chars
		int len = strlen(buffer);
		if ( len && buffer[len-1] == 10 ) {
			buffer[len - 1] = 0;
			len--;
		}

		if ( len < MINIMUM_WORD || len > num_chars ) {
			continue;
		}

		ch = buffer;
		while ( *ch ) {
			*ch = toupper( *ch );
			++ch;
		}

		trie_insert(root, buffer);
		num++;
	}

	return num;
}

/*
 * qsort function allows sorting by name.
 */
int sort_by_name(const void* first, const void* second) 
{
	return strcmp((*(Entry**)first)->str, (*(Entry**)second)->str);
}

/*
 * qsort function to sort entries by score.
 */
int sort_by_score(const void* first, const void* second)
{
	int score1 = (*(Entry**)first)->score;
	int score2 = (*(Entry**)second)->score;

	if ( score1 > score2 ) {
		return 1;
	} else if ( score1 < score2 ) {
		return -1;
	}
	
	return 0;
}

/*
 * Given an array of entries, it:
 *
 *    1. Removes duplicates.
 *    2. Sorts by score.
 */
int sort_entries(Entry* entries[], int num_entries) 
{
	int i;
	int new_num_entries = num_entries;
	int push_back = 0;

#if 0
	/* First, sort by name. */
	qsort(entries, num_entries, sizeof(Entry*), sort_by_name);

	/* Remove duplicate entries. */
	for ( i = 1; i < num_entries; ++i ) 
	{
		if ( 0 == strcmp(entries[i - 1 - push_back]->str, 
		                 entries[i]->str) ) {
			push_back++;
			new_num_entries--;
			free(entries[i]->str);
			free(entries[i]);
			entries[i] = 0;
		} else if ( push_back > 0 ) {
			entries[i - push_back] = entries[i];
		}
	}

	num_entries = new_num_entries;
#endif
	
	/* Now, sort by score. */
	qsort(entries, num_entries, sizeof(Entry*), sort_by_score);

	return num_entries;
}

/*
 * Think of the trie as a huge finite state machine. The context
 * represents the state of that machine as a word is traversed beginning
 * on a particular letter.
 * 
 * It also lets you join contexts together in a linked list.
 */
typedef struct _Context {
	TrieEntry* node;
	struct _Context* next;
} Context;

/*
 * A PartialSolution is a single square in the dynamic-programming
 * grid. For example, the entry at coordinates (2,4) contains all of
 * the partial words and completed words of length 4 beginning at
 * the second digit of the phone number.
 */
typedef struct _PartialSolution {
	Context* partial_words;
	Context* completed_words;
} PartialSolution;

/*
 * Create a single column of the dynamic programming grid.
 *
 * For example, create_column(appdata,  2, 7) would return a 1-D
 * array of 7 partial solutions containing all of the completed words
 * beginning at digit 2 of the phone nummber.
 */
PartialSolution** 
create_column( AppData* appdata, int starting_pos, int max_length )
{
	/* Create one column of the dynamic programming table described in 
		quick_algorithm(..)
	*/

	PartialSolution** column = 0;
	PartialSolution* previous_solution = 0;
	int length = 0;

	/* 
	 * Allocate space for the column of solutions. There is one square for
	 * each possible length, from 0 to max_length. 
	 * Note that it includes 0, so it is max_length + 1
	*/

	column = 
		(PartialSolution**)malloc( (max_length+1) * sizeof(PartialSolution*) );

	memset(column, 0, sizeof(PartialSolution*)*(max_length+1));
	
	/* for every possible length, */ 
	for ( length = 0; length <= max_length; ++length ) {

		const char** KEYPAD = &appdata->options.keymap[0];

		/* initialize entry */
		column[length] = 
			(PartialSolution*)malloc(sizeof(PartialSolution));
		
		memset(column[length], 0, sizeof(PartialSolution));

		/* if this is the first entry in the row we have nothing to build 
		on, so just initialize some data */
		if (0 == length ) {
			column[length]->partial_words = 
				(Context*)malloc(sizeof(Context));
			column[length]->partial_words->node = appdata->root;
			column[length]->partial_words->next = 0;
		} else {
			/* not first entry -- build on previous. */
			Context* current_context = 0;

			current_context = column[length-1]->partial_words;

			/* For every partially completed word in the previous solution, */
			while ( current_context ) {

				/* Try adding all letters for this digit of the phone number,
				 * and see if it results in a partial or complete word that
				 * we can then add to this solution. 
				 **/
					  
				TrieEntry* new_node = 0;
				const char* keys = 0;
				int key_len = 0;
				int i = 0;
				
				/* Get the letters corresponding to the current digit. */
				keys = KEYPAD[appdata->number[starting_pos + length - 1] - '0'];
				key_len = strlen(keys);

				/* For each letter (eg. "PQRS") */
				for ( i = 0; i < key_len; ++i ) {
					/* Traverse the trie. */
					new_node = trie_follow(current_context->node, 
						keys[i] );

					/* If adding this letter results in a valid word, */
					if ( new_node ) {
						/* 
						 * Create a new context and add it to this partial 
						 * solution. 
						 */
						Context* new_context = 0;
						new_context = (Context*)malloc(sizeof(Context));
						new_context->next = column[length]->partial_words;
						new_context->node = new_node;
						column[length]->partial_words = new_context;

						/* If adding the letter resulted in a completed word, */
						if ( new_context ->node->end ) {
							/* Add it to the list of completed words for this
							 * grid square.
							 */
							Context* completed_context = 0;
							completed_context = (Context*)malloc(sizeof(Context));
							completed_context->next = column[length]->completed_words;
							completed_context->node = new_node;
							column[length]->completed_words = completed_context;
						} /* if ended word */
					} /* if valid word */
				} /* key loop */

				current_context = current_context->next;
			} /* while contexts left to follow */
		} /* if not first entry */
	} /* for each row */

	return column;
}

/*
 * Frees a list of contexts, created in create_column.
 */
void 
free_context_list ( Context* context ) {
	
	Context* cur_context = 0;

	cur_context = context;

	while ( cur_context ) {
		Context* next_context = cur_context->next;
		free(cur_context);
		cur_context = next_context;
	}
}

/*
 * Frees an entire column of the dynamic-programming table, created by
 * create_column.
 */
void
free_column( PartialSolution** column, int length )
{
	int i = 0;

	for ( i = 0; i <= length; ++i ) {
		free_context_list(column[i]->partial_words);
		free_context_list(column[i]->completed_words);
		free(column[i]);
	}
}

/*
 * Given an array of partial solutions (as described above) it will
 * efficiently enumerate all combinations of completed words and add them
 * to the appdata entry tree.
 *
 * Note this is a recursive algorithm.
		
 *
 * Parameters:
 * 	appdata - solution entries are added to the root stored here.
 * 	table - Array of columns of partial solutions.
 * 	starting_pos: First digit of appdata->number to examine.
 * 	length: Maximum number of digits in appdata->number left.
 * 	prepend: String that is prepended to all enumerated solutions. In
 * 	         the initial call, pass "". Used for recursion.
 * 	score: Score that is added to the calculated score of all enumerated
 * 	       solutions. Initially, pass 0. Used for recursion.
 *
 * Returns:
 *    Number of solutions enumerated.
 */
int
enumerate_solutions(AppData* appdata, PartialSolution*** table, 
					int starting_pos, int length, const char* prepend, int score )
{
	int i = 0;
	int maximum_output_length = 0;
	int num_solutions = 0;
	char* buffer = 0;
	int buffer_start_pos = 0;
	int number_length = 0;

	if ( length < MINIMUM_WORD ) {
		/* Stop if we are out of digits. */
		return 0;
	}

	/*
	 * Create a buffer to hold the enumerated solutions. Every letter
	 * could potentially be followed by a '-' so make it twice the size
	 * we need.
	 */
	
	number_length = strlen(appdata->number);
	maximum_output_length = number_length * 2 + 1;
	buffer = (char*)malloc(maximum_output_length);

	/* Prepend the given string, and set buffer_start_pos to the very end
	 * of that. */
	strcpy(buffer, prepend);
	buffer_start_pos = strlen(buffer);
	
	/* If this is not the first character, append a '-'. */
	if ( starting_pos > 0 && buffer_start_pos > 0 && 
	     buffer[buffer_start_pos-1] != '-') {
		buffer[buffer_start_pos++] = '-';
		buffer[buffer_start_pos] = 0;
	}
	
	/* Starting with the longest words, loop through the partial solutions. */
	for ( i = length; i >= MINIMUM_WORD; --i ) {
		PartialSolution** column = 0;
		Context* context = 0;
		column = table[starting_pos];
		context = column[i]->completed_words;

		/* 
		 * context now contains all of the completed words that begin on
		 * starting_pos and are exactly i characters long.
		 */
		
		if ( context ) {
			/* For each word in the list of completed words, */
			do {
				int solutions_added = 0;

				/* 
				 * Retrieve the word from the trie's state and append it
				 * to the buffer. 
				 */
				trie_get_word( context->node, &buffer[buffer_start_pos], 
					maximum_output_length - buffer_start_pos);

				/* 
				 * Recurse, adjusting the starting position and length
				 * to account for the added word. This will enumerate
				 * all solutions that begin with this word. (Plus what
				 * we have from previous recursions)
				 */
				solutions_added += enumerate_solutions(appdata, table, 
					starting_pos + i, length - i, buffer, i*i);

				/* 
				 * If the numbers after this word do not form any more
				 * words,
				 */
				if ( solutions_added == 0 ) {

					/*
					 * Add on the remainder of the phone number as digits,
					 * after a dash if necessary, then output the solution.
					 */
					Entry* entry = 0;
					int inserted = 0;
					if ( 0 != appdata->number[starting_pos + i] ) {
						strcat(buffer, "-");
						strcat(buffer, 	&appdata->number[starting_pos + i]);
					}

					entry = (Entry*)malloc(sizeof(Entry));
					memset(entry, 0, sizeof(Entry));
					entry->score = score + i*i;
					entry->str = strdup(buffer);
					appdata->entries = 
						tree_insert(appdata->entries, entry, &inserted);
					
					if ( inserted ) {
						solutions_added++;
					}
				}
				
				num_solutions += solutions_added;

				/* 
				 * Remove the word we added from the end of the buffer
				 * and go on to the next one.
				 */
				buffer[buffer_start_pos] = 0;
				context = context->next;
			} while ( context );
		}
		
		/*
		 * We have now enumerated all solutions beginning at the
		 * given starting position. We will now recurse and
		 * try all words starting at the next digit.
		 *
		 * If there are more digits to try,
		 */
		if ( starting_pos + 1 + i <= number_length ) {
			/*
			 * But first, we have to do some magic to get the 
			 * dashes right.
			 */
			int my_start_pos = buffer_start_pos;
			int deleted_dash = 0;
			if ( (my_start_pos > 1) && (buffer[my_start_pos - 1] == '-') 
				&& (buffer[my_start_pos - 2] >= '0') 
				&& (buffer[my_start_pos - 2] <= '9')) {
				my_start_pos--;
				deleted_dash = 1;
			}
			
			/*
			 * Add the number corresponding to this starting position,
			 * since we have already enumerated all words where it is 
			 * a letter, add the number.
			 */
			buffer[my_start_pos] = appdata->number[starting_pos];
			buffer[my_start_pos+2] = '-';
			buffer[my_start_pos+1] = 0;
			num_solutions += enumerate_solutions(appdata, table, 
				starting_pos + 1, i, buffer, score);
			buffer[buffer_start_pos] = 0;
			if ( deleted_dash ) {
				buffer[my_start_pos] = '-';
			}
		}

		/* 
		 * buffer is now back to the same state is was at the beginning of 
		 * the loop
		 */
	}

	free(buffer);

	return num_solutions;
}


/* 
 * Dynamic programming algorithm: Build a 2-D array. X values (columns)
 * are the position of the first character of the word. Y values (rows) are the
 * length of the word. Each table entry is a context, which contains all of the
 * partial and finished words with the specified starting letter and length.
 *
 * Example: 
 *     Partial solution at coordinates (2, 5) contains all of the
 *     partially and completed words that begin at the second digit of the
 *     phone number and are exactly five digits long.
 */
int quick_algorithm(AppData* appdata)
{

	PartialSolution*** table = 0;
	int length = 0;
	int starting_pos = 0;
	int num_solutions = 0;

	/* calculate number of rows */
	length = strlen( appdata->number );

	/* Create space for the dynamic programming table. We have one column for
	each possible letter a word can start on (all of them) */
	table = (PartialSolution***)malloc( length * sizeof(PartialSolution**) );
	
	/* for each starting pos */
	for ( starting_pos = 0; starting_pos < length; starting_pos++ ) {

		/* create the row (All words starting on that digit) */
		table[starting_pos] = create_column(appdata, starting_pos, 
		                                    length - starting_pos);
	}

	/* enumerate the solutions into the appdata */
	num_solutions = enumerate_solutions( appdata, table, 0, length, "", 0 );

	/* free memory */
	for ( starting_pos = 0; starting_pos < length; starting_pos++ ) {

		free_column( table[starting_pos], length - starting_pos );
	}

	free( table );

	return num_solutions;
}

void
init_appdata( AppData* appdata )
{
	memset(appdata, 0, sizeof(*appdata));

	appdata->root = trie_create(0);
	appdata->options.use_default_dict = 1;
	appdata->options.keymap[0] = "";
	appdata->options.keymap[1] = "";	
	appdata->options.keymap[2] = "ABC";	
	appdata->options.keymap[3] = "DEF";	
	appdata->options.keymap[4] = "GHI";	
	appdata->options.keymap[5] = "JKL";	
	appdata->options.keymap[6] = "MNO";	
	appdata->options.keymap[7] = "PQRS";	
	appdata->options.keymap[8] = "TUV";	
	appdata->options.keymap[9] = "WXYZ";	

}
int parse_args( AppData* appdata, int argc, char* argv[] ) 
{
	char* retval = 0;
	int i = 0;

	for ( i = 1; i < argc; ++i ) {
		if ( 0 == strncmp( argv[i], "-d", 2 ) ) {
			if ( i + 1 < argc ) {
				++i;
				appdata->options.extra_dict_files = 
					stringList_add( appdata->options.extra_dict_files, argv[i] );
			} else {
				fprintf(stderr, "%s: -d option requires dictionary.\n",
						argv[0]);
				return -1;
			}
		} else if ( 0 == strncmp( argv[i], "-n", 2 ) ) {
			appdata->options.use_default_dict = 0;
		} else if ( 0 == strncmp( argv[i], "-r", 2 ) ) {
			appdata->options.lowest_first = 1;
		} else if ( 0 == strncmp( argv[i], "-s", 2 ) ) {
			appdata->options.show_scores = 1;
		} else if ( 0 == strncmp( argv[i], "-v", 2 ) ) {
			appdata->options.show_stats = 1;
		} else if ( strlen( argv[i] ) >= 3 && argv[i][0] == '-' &&
			argv[i][1] >= '0' && argv[i][1] <= '9' && argv[i][2] == '=' ) {
			char digit = argv[i][1] - '0';
			
			appdata->options.keymap[digit] = &argv[i][3];
		} else if ( strcmp( argv[i], "-help") == 0 || 
		            strcmp( argv[i], "--help" ) == 0 ||
		            strcmp( argv[i], "-?" ) == 0 ||
		            strcmp( argv[i], "-h" ) == 0 ) {
			print_usage(argv[0]);
			return -1;

		} else {
			if ( 0 == appdata->number ) {
				char* str = argv[i];
				char c = 0;
				int i = 0;
				
				/* eliminate illegal characters -- eg '-' */
				appdata->number = strdup(argv[i]);
				while ( c = *str++ ) {
					if ( c >= '0' && c <= '9' ) {
						appdata->number[i++] = c;
					}
				}
				
				appdata->number[i] = 0;

				if ( strlen(appdata->number) < 3 || 
						strlen(appdata->number) > 10 ) {
					fprintf(stderr, "%s: Number must be between 3 and 10 digits.\n",
							argv[0]);
					return -1;
				}
			} else {
				fprintf(stderr, "%s: Too many arguments.\n",argv[0]);
				return -1;
			}
		}
	}

	if ( 0 == appdata->number ) {
        fprintf(stderr, "%s: No phone number specified.\n", argv[0]);
        print_usage(argv[0]);
        return -1;

        char ch = getc(stdin), prev = '\0';
        int SIZE_ALLOC = 1024;
        appdata->number = malloc(SIZE_ALLOC);
        int nread=0, nalloc=SIZE_ALLOC;
        while ((prev !='\n' || ch != '\n') && (ch != EOF)) {
            if( ch != '\n') {
                if(nread > nalloc-2) {
                    appdata->number = realloc(appdata->number, nalloc+=SIZE_ALLOC);
                    appdata->number[nread++] = ch;
                }
            }
            prev = ch;
            ch = getc(stdin);
        }
        appdata->number[nread] = '\n';
	}

	return 0;
}

void
deinit_appdata( AppData* appdata )
{
	free(appdata->number);
	stringList_free(appdata->options.extra_dict_files);
	trie_destroy(appdata->root);
}

int main( int argc, char* argv[] )
{
	int len;
	int i;
	int words = 0;
	FILE* file;
	Entry** entries_array=0;
	Entry* entry = 0;
	AppData appdata;
	StringList* user_dict = 0;
    long long time0, time1, time2;

	init_appdata(&appdata);
	
	if ( 0 != parse_args(&appdata, argc, argv) ) {
		deinit_appdata(&appdata);
		return -1;
	}

	len = strlen(appdata.number);

	if ( appdata.options.use_default_dict ) {
		file = open_dict_file(default_dict_files);
		if ( file == NULL ) {
			fprintf(stderr, "Could not open dictionary file.\n");
			deinit_appdata(&appdata);
			return -1;
		}
		
		words = read_dict(file, appdata.root, len);
		if ( appdata.options.show_stats ) {
			printf("Read %d words into %d nodes.\n", words, trie_node_count);
		}
		fclose(file);
	}

	user_dict = appdata.options.extra_dict_files;
#ifdef TIME_IT
    time0 = current_timestamp();
#endif
	while ( user_dict ) {
		file = fopen(user_dict->str, "r");
		if ( 0 == file ) {
			fprintf(stderr, "%s: Could not read dictionary file %s",
					argv[0], user_dict->str);
			deinit_appdata(&appdata);
		}

		read_dict( file, appdata.root, len );
		fclose( file );
		user_dict = user_dict->next;
	}
#ifdef TIME_IT
    time1 = current_timestamp();
    printf("dict loading time: %lld\n", time1-time0);
#endif

	appdata.num_entries = quick_algorithm(&appdata);
	
	entries_array = (Entry**)malloc(appdata.num_entries * sizeof(Entry));
	memset(entries_array, 0, appdata.num_entries * sizeof(Entry));
	tree_to_array(appdata.entries, entries_array, appdata.num_entries);
	appdata.num_entries = sort_entries(entries_array, appdata.num_entries);

#ifdef TIME_IT
    time2 = current_timestamp();
    printf("process time: %lld\n", time2-time1);
#endif

    for ( i = 0; i < appdata.num_entries; ++i ) {
		int j = i;
		if ( !appdata.options.lowest_first ) {
			j = appdata.num_entries - i - 1;
		}

		if ( appdata.options.show_scores ) {
			printf("%s\t%d\n", entries_array[j]->str, entries_array[j]->score);
		} else {
			printf("%s\n", entries_array[j]->str);
		}
		
		free(entries_array[j]->str);
		free(entries_array[j]);
	}


	free(entries_array);
	deinit_appdata(&appdata);

	return 0;
}
