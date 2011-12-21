#ifndef GRAPEFRUIT_SIM_IMPL_H_
#define GRAPEFRUIT_SIM_IMPL_H_

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

#endif // GRAPEFRUIT_SIM_IMPL_H_
