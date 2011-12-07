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
	state->cache = cache_init(nfa->node_count+1);
	state->tmp = alloc(sizeof(bool)*(nfa->node_count+1));
	uintptr_t i;
	for (i = 0; i < nfa->node_count+1; ++i) {
		state->tmp[i] = false;
	}
	state->empty_state = cache_get(state->cache, state->tmp);
	sim_mark_active(nfa->nodes.array[0], state->tmp, nfa->node_count);
	state->start_state = cache_get(state->cache, state->tmp);
	state->dfa_state = state->start_state;
	state->count_matches = count_matches;
	state->whole_lines = whole_lines;
	state->invert_match = invert_match;
}

void sim_cleanup(struct sim_state* state) {
	free_cache(state->cache);
	free(state->tmp);
}

void free_sim_state(struct sim_state* state) {
	sim_cleanup(state);
	free(state);
}
