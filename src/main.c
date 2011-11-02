#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "build_nfa.h"
#include "nfa.h"
#include "parse.h"
#include "syntree.h"

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
	free_nfa(nfa);
	return 0;
}
