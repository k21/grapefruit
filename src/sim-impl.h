#ifndef GRAPEFRUIT_SIM_IMPL_H_
#define GRAPEFRUIT_SIM_IMPL_H_

static void sim_mark_active(struct nfa_node* node, bool* active,
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
			sim_mark_active(dest, active, node_count);
		}
	}
}

static void sim_node(struct nfa_node* node, bool* active,
		uintptr_t node_count, uint_fast8_t chr) {
	uintptr_t i;
	for (i = 0; i < node->edge_count; ++i) {
		struct nfa_edge* edge = node->edges.array[i];
		if (chr >= edge->min && chr <= edge->max) {
			struct nfa_node* dest = edge->destination;
			sim_mark_active(dest, active, node_count);
		}
	}
}

static struct dfa_state* sim_compute_dfa(struct sim_state* state,
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
	if (!state->whole_lines) {
		sim_mark_active(nfa->nodes.array[0], active, nfa->node_count);
	}
	bool orig = state->dfa_state->persistent;
	state->dfa_state->persistent = true;
	struct dfa_state* res = cache_get(state->cache, active);
	state->dfa_state->persistent = orig;
	return res;
}

static inline void sim_step(struct sim_state* state, uint_fast8_t chr) {
	struct dfa_state* dfa_state = state->dfa_state;
	if (!state->whole_lines && dfa_state->accept) return;
	struct dfa_state* next = dfa_state->edges[chr];
	if (!next) {
		next = sim_compute_dfa(state, chr);
		dfa_state->edges[chr] = next;
	}
	state->dfa_state = next;
}

static inline bool sim_is_match(struct sim_state* state) {
	return state->dfa_state->accept != state->invert_match;
}

#endif // GRAPEFRUIT_SIM_IMPL_H_
