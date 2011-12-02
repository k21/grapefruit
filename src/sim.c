#include <stdlib.h>

#include "common.h"
#include "sim.h"

static struct sim_state* new_state(void) {
	return alloc(sizeof(struct sim_state));
}

static void mark_active(struct nfa_node* node, bool* active,
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
		if (edge->min && !edge->max) {
			mark_active(dest, active, node_count);
		}
	}
}

struct sim_state* sim_init(struct nfa* nfa) {
	struct sim_state* state = new_state();
	state->nfa = nfa;
	state->cache = cache_init(nfa->node_count+1);
	state->tmp = alloc(sizeof(bool)*(nfa->node_count+1));
	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->tmp[i] = false;
	}
	state->empty_state = cache_get(state->cache, state->tmp);
	mark_active(nfa->nodes.array[0], state->tmp, nfa->node_count);
	state->dfa_state = cache_get(state->cache, state->tmp);
	return state;
}

static void sim_node(struct nfa_node* node, bool* active,
		uintptr_t node_count, uint_fast8_t chr) {
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];
		if (chr >= edge->min && chr <= edge->max) {
			struct nfa_node* dest = edge->destination;
			mark_active(dest, active, node_count);
		}
	}
}

static struct dfa_state* compute_dfa(struct sim_state* state,
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
	return cache_get(state->cache, active);
}

void sim_step(struct sim_state* state, uint_fast8_t chr) {
	if (chr > 127) {
		state->dfa_state = state->empty_state;
		return;
	}
	struct dfa_state* dfa_state = state->dfa_state;
	struct dfa_state* next = dfa_state->edges[chr];
	if (!next) {
		next = compute_dfa(state, chr);
		dfa_state->edges[chr] = next;
	}
	state->dfa_state = next;
}

bool sim_is_match(struct sim_state* state) {
	return state->dfa_state->active[state->nfa->node_count];
}

void free_sim_state(struct sim_state* state) {
	free_cache(state->cache);
	free(state->tmp);
	free(state);
}
