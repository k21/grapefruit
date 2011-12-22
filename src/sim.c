#include <stdlib.h>

#include "common.h"
#include "sim.h"

struct sim_state* new_sim_state(struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match) {
	struct sim_state* res = alloc(sizeof(struct sim_state));
	sim_init(res, nfa, count_matches, whole_lines, invert_match);
	return res;
}

void sim_init(struct sim_state* state, struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match) {
	state->nfa = nfa;
	state->cache = new_cache(nfa->node_count+1, 10*1024*1024);
	//TODO make the cache memory limit customizable
	state->tmp = alloc(sizeof(bool)*(nfa->node_count+1));
	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->tmp[i] = false;
	}
	state->empty_state = cache_get(state->cache, state->tmp);
	state->empty_state->persistent = true;
	sim_mark_active(nfa->nodes.array[0], state->tmp, nfa->node_count);
	state->start_state = cache_get(state->cache, state->tmp);
	state->start_state->persistent = true;
	state->dfa_state = state->start_state;
	state->count_matches = count_matches;
	state->whole_lines = whole_lines;
	state->invert_match = invert_match;
}

void sim_mark_active(struct nfa_node* node, bool* active,
		uintptr_t node_count) {
	if (!node) {
		active[node_count] = true;
		return;
	}
	if (active[node->id]) return;
	active[node->id] = true;
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];
		struct nfa_node* dest = edge->destination;
		if (edge->min == EDGE_SPECIAL_PREFIX && edge->max == EDGE_FREE) {
			sim_mark_active(dest, active, node_count);
		}
	}
}

void sim_node(struct nfa_node* node, bool* active,
		uintptr_t node_count, uint_fast8_t chr) {
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];
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
	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		active[i] = false;
	}
	for (i = 0; i < nfa->node_count; ++i) {
		if (!prev_active[i]) continue;
		struct nfa_node* node = nfa->nodes.array[i];
		sim_node(node, active, nfa->node_count, chr);
	}
	sim_mark_active(nfa->nodes.array[0], active, nfa->node_count);
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
