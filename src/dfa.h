#ifndef GRAPEFRUIT_DFA_H_
#define GRAPEFRUIT_DFA_H_

#include <stdbool.h>

struct sim_state;
struct nfa;

struct dfa_state {
	bool* active;
	struct dfa_state* edges[256];
	bool accept;
	bool persistent;
};

struct dfa_cache;

struct dfa_cache* new_cache(uintptr_t depth, uintptr_t mem_limit);
struct dfa_state* cache_get(struct dfa_cache* cache, bool* active);
void free_cache(struct dfa_cache* cache);

#endif // GRAPEFRUIT_DFA_H_
