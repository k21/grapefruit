#ifndef GRAPEFRUIT_SIM_H_
#define GRAPEFRUIT_SIM_H_

#include <stdbool.h>
#include <stdint.h>

#include "dfa.h"
#include "nfa.h"

// sim_state represents current state of simulation of finite automaton
// it uses nfa to compute unknown states and transitions in dfa
// before_begin and after_begin are cached states before and after the special
//     line begin character
// tmp is used only temporarily in functions to avoid allocating new array
struct sim_state {
	struct nfa* nfa;
	struct dfa_state* dfa_state;
	struct dfa_state* before_begin;
	struct dfa_state* after_begin;
	struct dfa_cache* cache;
	bool* tmp;
	bool count_matches, whole_lines, invert_match;
};

// Allocate and initialize sim_state
struct sim_state* new_sim_state(struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match);

// Initialize already allocated sim_state
void sim_init(struct sim_state* state, struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match);

// Compute next dfa_state based on current state and next character
// It uses nfa to compute the next state
// Next state is automatically saved to cache
struct dfa_state* sim_compute_dfa(struct sim_state* state,
		uint_fast8_t chr);

// Compute next dfa_state
// If the transition is cached, the cache is used
// Otherwise sim_compute_dfa() is used
static inline void sim_step(struct sim_state* state, uint_fast8_t chr);

// Return true iff the state is an exit state
bool sim_is_match(struct sim_state* state);

// Clean up state, free all structures it used except the state itself
void sim_cleanup(struct sim_state* state);

// Clean up and free state
void free_sim_state(struct sim_state* state);

#include "sim-impl.h"

#endif // GRAPEFRUIT_SIM_H_
