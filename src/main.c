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

static char* help_message =
"Usage: %s [OPTION]... PATTERN\n"
"Filter lines on standard input according to PATTERN.\n"
"Pattern is a regular expression as described in user manual.\n"
"\n"
"Recognized options:\n"
"  -x, --invert-match     PATTERN must match whole lines\n"
"  -v, --line-regexp      output lines that did NOT match\n"
"  -c, --count            do not print matched lines, only their count\n"
"      --help             display this help message and exit\n"
"\n"
"If an error occurs, exit status is 2. Otherwise, if at least one line was\n"
"matched, exit status is 0, if no line matched, exit status is 1.\n"
;

static struct buffer buffer;
static struct sim_state state;

bool invert_match = false;
bool whole_lines = false;
bool count_matches = false;
int display_help = 0;

int main(int argc, char** argv) {
	// read all used flags
	while (1) {
		static struct option long_options[] = {
			{"invert-match", no_argument, 0, 'v'},
			{"line-regexp",  no_argument, 0, 'x'},
			{"count",        no_argument, 0, 'c'},
			{"help",         no_argument, &display_help, 1},
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

	if (display_help) {
		printf(help_message, argv[0]);
		return 0;
	}

	// check if there is exactly one regular expression
	if (optind != argc-1) {
		printf("Usage: %s [OPTION]... PATTERN\n", argv[0]);
		printf("Run '%s --help' to get more information\n", argv[0]);
		return 2;
	}

	char* regex = argv[optind];

	// parse the regular expression to abstract syntax tree
	struct syntree* tree;
	tree = parse(regex, strlen(regex));

	// build nfa for the regular expression
	struct nfa* nfa;
	nfa = build_nfa(tree, whole_lines);
	free_tree(tree);

	// initialize simulation state
	sim_init(&state, nfa, invert_match);

	// initialize buffer
	uintptr_t buffer_size = 65536;
	buffer_init(&buffer, STDIN_FILENO, STDOUT_FILENO, buffer_size);

	// main matching loop
	uintmax_t match_count = 0;
	bool new_line = true;
	bool some_match = false;
	intptr_t res = buffer_next(&buffer);
	if (!count_matches) buffer_mark(&buffer);
	while (res == 1) {
		// save next input byte to ch
		uint_fast8_t ch = buffer_get(&buffer);

		if (ch == '\n') {
			// handle the end of line here
			new_line = true;
			// simulate the special lline end character
			if (!state.dfa_state->accept) sim_step(&state, CHAR_INPUT_END);
			if (sim_is_match(&state)) {
				// the last line matched, either print it or count it
				some_match = true;
				if (count_matches) {
					++match_count;
				} else {
					int_fast8_t res = buffer_print(&buffer, true);
					if (res != 1) die(2, (res == -1) ? errno : 0,
							"Error writing to stdout");
				}
			}
			// reset simulation state
			state.dfa_state = state.after_begin;
			// read next character from input
			res = buffer_next(&buffer);
			if (!count_matches) buffer_mark(&buffer);
		} else {
			new_line = false;
			// simulate only if there was no match on the current line
			if (!state.dfa_state->accept) {
				// if the character is not valid ASCII, no transitions will take place
				if (ch > MAX_CHAR) state.dfa_state = state.before_begin;
				// otherwise do the simulation
				else sim_step(&state, ch);
			}
			// read next character from input
			res = buffer_next(&buffer);
		}
	}
	if (res == -1) die(2, errno, "Error reading from stdin");

	// check if there is no \n at the end of the input
	// if it is the case, behave as if it was there
	if (!new_line) {
		if (!state.dfa_state->accept) sim_step(&state, CHAR_INPUT_END);
		if (sim_is_match(&state)) {
			some_match = true;
			if (count_matches) {
				++match_count;
			} else {
				int_fast8_t res = buffer_print(&buffer, false);
				puts("");
				if (res != 1) die(2, (res == -1) ? errno : 0,
						"Error writing to stdout");
			}
		}
	}

	// if -c is used, print the count of matches
	if (count_matches) {
		printf("%" PRIuMAX "\n", match_count);
	}

	// clean up
	buffer_cleanup(&buffer);
	sim_cleanup(&state);
	free_nfa(nfa);

	// return status is based on whether there was a match or not
	return some_match ? 0 : 1;
}
