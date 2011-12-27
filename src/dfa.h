#ifndef GRAPEFRUIT_DFA_H_
#define GRAPEFRUIT_DFA_H_

#include <stdbool.h>

#include "const.h"

struct sim_state;
struct nfa;

// dfa_states are cached in dfa_cache
// unless persistent is true, the state can be freed when cache_get() is called
// accept is true if the state is an exit state
struct dfa_state {
	bool* active;
	struct dfa_state* edges[EXT_ALPHABET_SIZE];
	bool accept;
	bool persistent;
};

// dfa_cache caches dfa_states with their transitions to next states
struct dfa_cache;

// Allocate and initialize new cache
struct dfa_cache* new_cache(uintptr_t depth, uintptr_t mem_limit);

// If state with the same active nodes is in cache, return it
// Otherwise, save it in the cache
struct dfa_state* cache_get(struct dfa_cache* cache, bool* active);

// Clean up and free the cache
void free_cache(struct dfa_cache* cache);

#endif // GRAPEFRUIT_DFA_H_
