#include <errno.h>
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

static void check_match(struct sim_state* state, struct list* line_parts,
		struct buffer* buffer, int line_start, int i) {
	if (sim_is_match(state)) {
		while (line_parts->head) {
			struct buffer* part = list_pop_front(line_parts);
			if (line_start < part->full) {
				write(STDOUT_FILENO, part->data+line_start, part->full-line_start);
			}
			//TODO writeall, check
			free_buffer(part);
			line_start = 0;
		}
		write(STDOUT_FILENO, buffer->data+line_start, i+1-line_start);
	} else {
		while (line_parts->head) {
			struct buffer* part = list_pop_front(line_parts);
			free_buffer(part);
		}
	}
	state->dfa_state = state->start_state;
}

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
	uintptr_t buffer_size = 65536;
	struct buffer* buffer = 0;
	struct list* line_parts = new_list();
	uintptr_t line_start = 0;
	while (1) {
		buffer = buffer_fill(STDIN_FILENO, buffer_size);
		if (buffer->full == -1) {
			die(1, errno, "Error reading from stdin");
		}
		if (buffer->full == 0) break;
		uintptr_t i;
		for (i = 0; i < (uintptr_t)buffer->full; ++i) {
			if (buffer->data[i] == '\n') {
				check_match(state, line_parts, buffer, line_start, i);
				line_start = i+1;
			} else {
				sim_step(state, buffer->data[i]);
			}
		}
		list_push_back(line_parts, buffer);
	}
	check_match(state, line_parts, buffer, line_start, buffer->full);
	free(line_parts);
	free(buffer);
	free_sim_state(state);
	free_nfa(nfa);
	return 0;
}
