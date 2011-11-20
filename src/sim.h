#ifndef NEGREP_SIM_H_
#define NEGREP_SIM_H_

#include <stdbool.h>
#include <stdint.h>

#include "nfa.h"

struct sim_state {
	struct nfa* nfa;
	bool* active;
	bool* prev_active;
};

struct sim_state* sim_init(struct nfa* nfa);
void sim_step(struct sim_state* state, uint_fast8_t chr, bool start);
bool sim_is_match(struct sim_state* state);
void free_sim_state(struct sim_state* state);

#endif // NEGREP_SIM_H_
