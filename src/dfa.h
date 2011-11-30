#ifndef NEGREP_DFS_H_
#define NEGREP_DFS_H_

#include <stdbool.h>

struct sim_state;
struct nfa;

struct dfa_state {
	bool* active;
	struct dfa_state* edges[128];
};

struct dfa_cache;

struct dfa_cache* cache_init(uintptr_t depth);
struct dfa_state* cache_get(struct dfa_cache* cache, bool* active);
void free_cache(struct dfa_cache* cache);

#endif // NEGREP_DFS_H_
