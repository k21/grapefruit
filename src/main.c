#include <errno.h>
#include <inttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "build_nfa.h"
#include "common.h"
#include "list.h"
#include "nfa.h"
#include "parse.h"
#include "syntree.h"
#include "sim.h"

static struct buffer buffer;
static struct sim_state state;

int main(int argc, char** argv) {
	bool invert_match = false;
	bool whole_lines = false;
	bool count_matches = false;
	while (1) {
		static struct option long_options[] = {
			{"invert-match", no_argument, 0, 'v'},
			{"line-regexp",  no_argument, 0, 'x'},
			{"count",        no_argument, 0, 'c'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "vxc", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 'v': invert_match = true; break;
			case 'x': whole_lines = true; break;
			case 'c': count_matches = true; break;
			default: break;
		}
	}
	if (optind != argc-1) {
		printf("incorrect usage\n"); //TODO print usage / help
		return 1;
	}
	char* regex = argv[optind];
	struct syntree* tree;
	tree = parse(regex, strlen(regex));
	struct nfa* nfa;
	nfa = build_nfa(tree);
	free_tree(tree);
	sim_init(&state, nfa, count_matches,
			whole_lines, invert_match);
	uintptr_t buffer_size = 65536;
	buffer_init(&buffer, STDIN_FILENO, STDOUT_FILENO, buffer_size);
	uintmax_t match_count = 0;
	bool new_line = true;
	intptr_t res = buffer_next(&buffer);
	while (res == 1) {
		uint_fast8_t ch = buffer_get(&buffer);
		if (ch == '\n') {
			new_line = true;
			if (sim_is_match(&state)) {
				if (count_matches) {
					++match_count;
				} else {
					int_fast8_t res = buffer_print(&buffer, true);
					if (res != 1) die(1, (res == -1) ? errno : 0,
							"Error writing to stdout");
				}
			}
			state.dfa_state = state.start_state;
			res = buffer_next(&buffer);
			if (!count_matches) buffer_mark(&buffer);
		} else {
			new_line = false;
			sim_step(&state, ch);
			res = buffer_next(&buffer);
		}
	}
	if (res == -1) die(1, errno, "Error reading from stdin");
	if (!new_line && sim_is_match(&state)) {
		if (count_matches) {
			++match_count;
		} else {
			int_fast8_t res = buffer_print(&buffer, false);
			puts("");
			if (res != 1) die(1, (res == -1) ? errno : 0,
					"Error writing to stdout");
		}
	}
	if (count_matches) {
		printf("%" PRIuMAX "\n", match_count);
	}
	buffer_cleanup(&buffer);
	sim_cleanup(&state);
	free_nfa(nfa);
	return 0;
}
