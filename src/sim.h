#ifndef GRAPEFRUIT_SIM_H_
#define GRAPEFRUIT_SIM_H_

#include <stdbool.h>
#include <stdint.h>

#include "dfa.h"
#include "nfa.h"

struct sim_state {
	struct nfa* nfa;
	struct dfa_state* dfa_state;
	struct dfa_state* before_begin;
	struct dfa_state* after_begin;
	struct dfa_cache* cache;
	bool* tmp;
	bool count_matches, whole_lines, invert_match;
};

struct sim_state* new_sim_state(struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match);
void sim_init(struct sim_state* state, struct nfa* nfa,
		bool count_matches, bool whole_lines, bool invert_match);
void sim_mark_active(struct nfa_node* node, bool* active,
		uintptr_t node_count);
void sim_node(struct nfa_node* node, bool* active,
		uintptr_t node_count, uint_fast8_t chr);
struct dfa_state* sim_compute_dfa(struct sim_state* state,
		uint_fast8_t chr);
static inline void sim_step(struct sim_state* state, uint_fast8_t chr);
bool sim_is_match(struct sim_state* state);
void sim_cleanup(struct sim_state* state);
void free_sim_state(struct sim_state* state);

#include "sim-impl.h"

#endif // GRAPEFRUIT_SIM_H_
