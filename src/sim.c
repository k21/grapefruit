#include <stdlib.h>

#include "sim.h"

static struct sim_state* new_state(void) {
	return malloc(sizeof(struct sim_state));
}

struct sim_state* sim_init(struct nfa* nfa) {
	struct sim_state* state = new_state();
	state->nfa = nfa;
	state->active = malloc((nfa->node_count+1)*sizeof(bool));
	state->prev_active = malloc((nfa->node_count+1)*sizeof(bool));
	int i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->active[i] = false;
		state->prev_active[i] = false;
	}
	return state;
}

static void sim_node(struct nfa_node* node, bool* active, int node_count, char chr) {
	int i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges[i];
		struct nfa_node* dest = edge->destination;
		if (edge->min && !edge->max && dest) {
			sim_node(dest, active, node_count, chr);
		}
		else if (chr < edge->min || chr > edge->max) continue;
		if (dest) {
			active[dest->id] = true;
		} else {
			active[node_count] = true;
		}
	}
}

void sim_step(struct sim_state* state, char chr, bool start) {
	struct nfa* nfa = state->nfa;
	int i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->prev_active[i] = false;
	}
	bool* t = state->prev_active;
	state->prev_active = state->active;
	state->active = t;
	for (i = 0; i < nfa->node_count; ++i) {
		if (!state->prev_active[i] && !(start && i == 0)) continue;
		struct nfa_node* node = nfa->nodes[i];
		sim_node(node, state->active, nfa->node_count, chr);
	}
}

bool sim_is_match(struct sim_state* state) {
	return state->active[state->nfa->node_count];
}

void free_sim_state(struct sim_state* state) {
	free(state->active);
	free(state->prev_active);
	free(state);
}
