#include <stdint.h>

#include "common.h"
#include "dfa.h"
#include "sim.h"

struct dfa_cache {
	union {
		struct dfa_cache* child[2];
		struct dfa_state* data;
	};
};

static struct dfa_cache* new_cache(void) {
	struct dfa_cache* res = alloc(sizeof(struct dfa_cache));
	res->child[0] = res->child[1] = 0;
	return res;
}

static struct dfa_state* new_state(struct sim_state* state) {
	struct dfa_state* res = alloc(sizeof(struct dfa_state));
	uintptr_t active_size = state->nfa->node_count+1;
	res->active = alloc(sizeof(bool)*active_size);
	uintptr_t i;
	for (i = 0; i < active_size; ++i) {
		res->active[i] = state->active[i];
	}
	for (i = 0; i < 128; ++i) {
		res->edges[i] = 0;
	}
	return res;
}

struct dfa_cache* cache_init(void) {
	return new_cache();
}

static struct dfa_state* get_impl(struct dfa_cache* cache,
		struct sim_state* state, uintptr_t i) {
	if (i == state->nfa->node_count+1) {
		struct dfa_state* dfa_state = cache->data;
		if (!dfa_state) {
			dfa_state = new_state(state);
			cache->data = dfa_state;
		}
		return dfa_state;
	}
	bool active = state->active[i];
	struct dfa_cache* next = cache->child[active];
	if (!next) {
		next = new_cache();
		if (i == state->nfa->node_count) {
			next->data = 0;
		}
		cache->child[active] = next;
	}
	return get_impl(next, state, i+1);
}

struct dfa_state* cache_get(struct dfa_cache* cache, struct sim_state* state) {
	return get_impl(cache, state, 0);
}

static void free_impl(struct dfa_cache* cache, uintptr_t depth) {
	if (!cache) return;
	if (depth == 0) {
		free(cache->data->active);
		free(cache->data);
	} else {
		free_impl(cache->child[0], depth-1);
		free_impl(cache->child[1], depth-1);
	}
	free(cache);
}

void free_cache(struct dfa_cache* cache, struct nfa* nfa) {
	free_impl(cache, nfa->node_count+1);
}
