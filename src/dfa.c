#include <stdint.h>

#include "common.h"
#include "dfa.h"
#include "sim.h"

struct dfa_cache {
	uintptr_t depth;
	union {
		struct dfa_cache* child[2];
		struct dfa_state* data;
	};
};

static struct dfa_cache* new_cache(uintptr_t depth) {
	struct dfa_cache* res = alloc(sizeof(struct dfa_cache));
	res->depth = depth;
	res->child[0] = res->child[1] = 0;
	return res;
}

static struct dfa_state* new_state(bool* active, uintptr_t active_size) {
	struct dfa_state* res = alloc(sizeof(struct dfa_state));
	res->active = alloc(sizeof(bool)*active_size);
	uintptr_t i;
	for (i = 0; i < active_size; ++i) {
		res->active[i] = active[i];
	}
	for (i = 0; i < 128; ++i) {
		res->edges[i] = 0;
	}
	return res;
}

struct dfa_cache* cache_init(uintptr_t depth) {
	return new_cache(depth);
}

static struct dfa_state* get_impl(struct dfa_cache* cache, bool* active,
		uintptr_t root_depth) {
	if (cache->depth == 0) {
		struct dfa_state* dfa_state = cache->data;
		if (!dfa_state) {
			dfa_state = new_state(active, root_depth);
			cache->data = dfa_state;
		}
		return dfa_state;
	}
	bool next_active = active[root_depth - cache->depth];
	struct dfa_cache* next = cache->child[next_active];
	if (!next) {
		next = new_cache(cache->depth-1);
		if (next->depth == 0) {
			next->data = 0;
		}
		cache->child[next_active] = next;
	}
	return get_impl(next, active, root_depth);
}

struct dfa_state* cache_get(struct dfa_cache* cache, bool* active) {
	return get_impl(cache, active, cache->depth);
}

void free_cache(struct dfa_cache* cache) {
	if (!cache) return;
	if (cache->depth == 0) {
		free(cache->data->active);
		free(cache->data);
	} else {
		free_cache(cache->child[0]);
		free_cache(cache->child[1]);
	}
	free(cache);
}
