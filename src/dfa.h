#ifndef GRAPEFRUIT_DFS_H_
#define GRAPEFRUIT_DFS_H_

#include <stdbool.h>

struct sim_state;
struct nfa;

struct dfa_state {
	bool* active;
	struct dfa_state* edges[256];
};

struct dfa_cache;

struct dfa_cache* cache_init(uintptr_t depth);
struct dfa_state* cache_get(struct dfa_cache* cache, bool* active);
void free_cache(struct dfa_cache* cache);

#endif // GRAPEFRUIT_DFS_H_
