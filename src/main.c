#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "build_nfa.h"
#include "nfa.h"
#include "parse.h"
#include "syntree.h"
#include "sim.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		puts("no regex");
		return 0;
	}
	struct syntree* tree;
	tree = parse(argv[1], strlen(argv[1]));
	struct nfa* nfa;
	nfa = build_nfa(tree);
	free_tree(tree);
	struct sim_state* state = sim_init(nfa);
	char* data = "abcdefgh";
	int i;
	for (i = 0; i < 8; ++i) {
		sim_step(state, data[i], false);
	}
	printf(sim_is_match(state) ? "match\n" : "no match\n");
	free_sim_state(state);
	free_nfa(nfa);
	return 0;
}
