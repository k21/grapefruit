#include <stdlib.h>

#include "common.h"
#include "sim.h"

struct sim_state* new_sim_state(struct nfa* nfa,
		bool invert_match, uintptr_t cache_mem_limit) {
	struct sim_state* res = alloc(sizeof(struct sim_state));
	sim_init(res, nfa, invert_match, cache_mem_limit);
	return res;
}

// Mark current state and all states accesible through free edges as active
static inline void sim_mark_active(struct nfa_node* node, bool* active,
		uintptr_t node_count) {
	if (!node) {
		// node is the exit node
		active[node_count] = true;
		return;
	}

	// if node is already active, do not continue to avoid loops
	if (active[node->id]) return;

	active[node->id] = true;
	// check all edges, follow them if they are free
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];
		struct nfa_node* dest = edge->destination;
		if (edge->min == EDGE_SPECIAL_PREFIX && edge->max == EDGE_FREE) {
			sim_mark_active(dest, active, node_count);
		}
	}
}

void sim_init(struct sim_state* state, struct nfa* nfa,
		bool invert_match, uintptr_t cache_mem_limit) {
	state->nfa = nfa;
	state->cache = new_cache(nfa->node_count+1, cache_mem_limit);
	state->tmp = alloc(sizeof(bool)*(nfa->node_count+1));
	state->invert_match = invert_match;

	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->tmp[i] = false;
	}

	sim_mark_active(nfa->nodes.array[0], state->tmp, nfa->node_count);
	state->before_begin = cache_get(state->cache, state->tmp);
	state->before_begin->persistent = true;

	state->dfa_state = state->before_begin;
	state->after_begin = sim_compute_dfa(state, CHAR_INPUT_BEGIN);
	state->after_begin->persistent = true;

	state->before_begin->edges[CHAR_INPUT_BEGIN] = state->after_begin;

	state->dfa_state = state->after_begin;
}

// Simulate single nfa node
// This is called once for each active node in every step
static inline void sim_node(struct nfa_node* node, bool* active,
		uintptr_t node_count, uint_fast8_t chr) {
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];

		// find out whether this edge should be followed
		bool can_follow = false;
		if (edge->min == EDGE_SPECIAL_PREFIX) {
			if (edge->max == chr - EDGE_SPECIAL_PREFIX) can_follow = true;
		} else {
			if (chr >= edge->min && chr <= edge->max) can_follow = true;
		}

		if (can_follow) {
			struct nfa_node* dest = edge->destination;
			sim_mark_active(dest, active, node_count);
		}
	}
}

struct dfa_state* sim_compute_dfa(struct sim_state* state,
		uint_fast8_t chr) {
	struct nfa* nfa = state->nfa;
	bool* active = state->tmp;
	bool* prev_active = state->dfa_state->active;
	
	// set all states as unactive
	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		active[i] = false;
	}

	// call sim_node() for each active node
	for (i = 0; i < nfa->node_count; ++i) {
		if (!prev_active[i]) continue;
		struct nfa_node* node = nfa->nodes.array[i];
		sim_node(node, active, nfa->node_count, chr);
	}

	// mark start node as active
	// this allows us to match strings starting anywhere in input
	sim_mark_active(nfa->nodes.array[0], active, nfa->node_count);

	// find out if the new state is in cache, if not, save it
	bool orig = state->dfa_state->persistent;
	state->dfa_state->persistent = true;
	struct dfa_state* res = cache_get(state->cache, active);
	state->dfa_state->persistent = orig;

	return res;
}

bool sim_is_match(struct sim_state* state) {
	return state->dfa_state->accept != state->invert_match;
}

void sim_cleanup(struct sim_state* state) {
	free_cache(state->cache);
	free(state->tmp);
}

void free_sim_state(struct sim_state* state) {
	sim_cleanup(state);
	free(state);
}
