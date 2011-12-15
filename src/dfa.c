#include <stdint.h>

#include "common.h"
#include "dfa.h"
#include "sim.h"

struct dfa_cache_level {
	union {
		struct dfa_cache_level* child[2];
		struct dfa_state* state;
	} data;
};

struct dfa_cache {
	uintptr_t depth;
	struct dfa_cache_level root;
};

static struct dfa_cache_level* new_level(uintptr_t depth) {
	struct dfa_cache_level* res = alloc(sizeof(struct dfa_cache_level));
	if (depth == 0) {
		res->data.state = 0;
	} else {
		res->data.child[0] = res->data.child[1] = 0;
	}
	return res;
}

static struct dfa_state* new_state(bool* active, uintptr_t active_length) {
	struct dfa_state* res = alloc(sizeof(struct dfa_state));
	uintptr_t i;
	for (i = 0; i < 256; ++i) {
		res->edges[i] = 0;
	}
	res->active = alloc(sizeof(bool)*active_length);
	for (i = 0; i < active_length; ++i) {
		res->active[i] = active[i];
	}
	res->accept = res->active[active_length-1];
	return res;
}

struct dfa_cache* new_cache(uintptr_t depth) {
	struct dfa_cache* res = alloc(sizeof(struct dfa_cache));
	res->depth = depth;
	if (depth == 0) {
		res->root.data.state = 0;
	} else {
		res->root.data.child[0] = res->root.data.child[1] = 0;
	}
	return res;
}

static struct dfa_state* get_impl(struct dfa_cache_level* level, bool* active,
		uintptr_t depth, uintptr_t cache_depth) {
	if (depth == cache_depth) {
		struct dfa_state* dfa_state = level->data.state;
		if (!dfa_state) {
			dfa_state = new_state(active, cache_depth);
			level->data.state = dfa_state;
		}
		return dfa_state;
	}
	bool next_active = active[depth];
	struct dfa_cache_level* next = level->data.child[next_active];
	if (!next) {
		next = new_level(cache_depth - depth);
		level->data.child[next_active] = next;
	}
	return get_impl(next, active, depth+1, cache_depth);
}

struct dfa_state* cache_get(struct dfa_cache* cache, bool* active) {
	return get_impl(&cache->root, active, 0, cache->depth);
}

static void free_cache_level(struct dfa_cache_level* level, uintptr_t depth) {
	if (!level) return;
	if (depth == 0) {
		free(level->data.state->active);
		free(level->data.state);
	} else {
		free_cache_level(level->data.child[0], depth-1);
		free_cache_level(level->data.child[1], depth-1);
	}
	free(level);
}

void free_cache(struct dfa_cache* cache) {
	if (cache->depth == 0) {
		free(cache->root.data.state->active);
		free(cache->root.data.state);
	} else {
		free_cache_level(cache->root.data.child[0], cache->depth-1);
		free_cache_level(cache->root.data.child[1], cache->depth-1);
	}
	free(cache);
}
