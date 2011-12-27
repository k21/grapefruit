#include <stdint.h>

#include "common.h"
#include "dfa.h"
#include "sim.h"

// The cache is a binary tree, depth is equal to number of nfa nodes
struct dfa_cache_level {
	union {
		struct dfa_cache_level* child[2];
		struct dfa_state* state;
	} data;
};

struct dfa_cache {
	uintptr_t depth;
	uintptr_t mem_usage;
	uintptr_t mem_limit;
	struct dfa_cache_level* root;
};

static struct dfa_cache_level* new_level(uintptr_t depth,
		struct dfa_cache* cache) {
	struct dfa_cache_level* res = alloc(sizeof(struct dfa_cache_level));
	cache->mem_usage += sizeof(struct dfa_cache_level);
	if (depth == 0) {
		res->data.state = 0;
	} else {
		res->data.child[0] = res->data.child[1] = 0;
	}
	return res;
}

static struct dfa_state* new_state(bool* active, struct dfa_cache* cache) {
	struct dfa_state* res = alloc(sizeof(struct dfa_state));
	cache->mem_usage += sizeof(struct dfa_state);
	// set all next states to unknown
	uintptr_t i;
	for (i = 0; i < EXT_ALPHABET_SIZE; ++i) {
		res->edges[i] = 0;
	}

	// save copy of active to the new state
	res->active = alloc(sizeof(bool)*cache->depth);
	cache->mem_usage += sizeof(bool)*cache->depth;
	for (i = 0; i < cache->depth; ++i) {
		res->active[i] = active[i];
	}

	res->accept = res->active[cache->depth-1];
	res->persistent = false;
	return res;
}

struct dfa_cache* new_cache(uintptr_t depth, uintptr_t mem_limit) {
	struct dfa_cache* res = alloc(sizeof(struct dfa_cache));
	res->depth = depth;
	res->mem_usage = 0;
	res->mem_limit = mem_limit;
	res->root = new_level(depth, res);
	return res;
}

static bool clear_level(struct dfa_cache_level* level, uintptr_t depth,
		struct dfa_cache* cache) {
	if (!level) return true;
	if (depth == cache->depth) {
		if (level->data.state->persistent) {
			uintptr_t i;
			for (i = 0; i < EXT_ALPHABET_SIZE; ++i) {
				level->data.state->edges[i] = 0;
			}
			return false;
		} else {
			free(level->data.state->active);
			free(level->data.state);
			free(level);
			cache->mem_usage -= sizeof(bool)*cache->depth
					+ sizeof(struct dfa_state)
					+ sizeof(struct dfa_cache_level);
			return true;
		}
	} else {
		bool r0, r1;
		r0 = clear_level(level->data.child[0], depth+1, cache);
		r1 = clear_level(level->data.child[1], depth+1, cache);
		// if both subtrees were freed, the whole tree can be freed
		if (r0 && r1) {
			free(level);
			cache->mem_usage -= sizeof(struct dfa_cache_level);
			return true;
		} else if (r0) {
			level->data.child[0] = 0;
		} else if (r1) {
			level->data.child[1] = 0;
		}
		return false;
	}
}

// Called to remove all non-persistent states from cache when its memory usage
//     grows too much
static void clear(struct dfa_cache* cache) {
	bool r = clear_level(cache->root, 0, cache);
	if (r) {
		cache->root = new_level(cache->depth, cache);
	}
}

static struct dfa_state* get_impl(struct dfa_cache_level* level, bool* active,
		uintptr_t depth, struct dfa_cache* cache) {
	// if we are at the bottom level, return the state (optionally create it)
	if (depth == cache->depth) {
		struct dfa_state* dfa_state = level->data.state;
		if (!dfa_state) {
			dfa_state = new_state(active, cache);
			level->data.state = dfa_state;
		}
		return dfa_state;
	}

	// continue recursively
	bool next_active = active[depth];
	struct dfa_cache_level* next = level->data.child[next_active];
	if (!next) {
		next = new_level(cache->depth - depth, cache);
		level->data.child[next_active] = next;
	}
	return get_impl(next, active, depth+1, cache);
}

struct dfa_state* cache_get(struct dfa_cache* cache, bool* active) {
	struct dfa_state* res = get_impl(cache->root, active, 0, cache);
	if (cache->mem_usage > cache->mem_limit) {
		bool orig = res->persistent;
		res->persistent = true;
		clear(cache);
		res->persistent = orig;
	}
	return res;
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
	free_cache_level(cache->root, cache->depth);
	free(cache);
}
