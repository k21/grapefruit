#include <errno.h>
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

static intptr_t write_all(int fd, char* buffer, uintptr_t size,
		uintptr_t skip) {
	uintptr_t written = skip;
	while (written != size) {
		intptr_t res = write(fd, buffer+written, size-written);
		if (res == -1 && errno == EINTR) continue;
		if (res == -1) return -1;
		if (res == 0) return written;
		written += res;
	}
	return written;
}

static void check_match(struct sim_state* state, struct list* line_parts,
		struct buffer* buffer, int line_start, int i) {
	if (sim_is_match(state)) {
		while (line_parts->head) {
			struct buffer* part = list_pop_front(line_parts);
			intptr_t res = write_all(STDOUT_FILENO, part->data,
				part->full, line_start);
			if (res != part->full) {
				die(1, (res == -1) ? errno : 0, "Error writing to stdout");
			}
			free_buffer(part);
			line_start = 0;
		}
		intptr_t res;
		res = write_all(STDOUT_FILENO, buffer->data, i+1, line_start);
		if (res != i+1) {
			die(1, (res == -1) ? errno : 0, "Error writing to stdout");
		}
	} else {
		while (line_parts->head) {
			struct buffer* part = list_pop_front(line_parts);
			free_buffer(part);
		}
	}
	state->dfa_state = state->start_state;
}

int main(int argc, char** argv) {
	bool invert_match = false;
	bool match_whole_lines = false;
	bool match_count = false;
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
			case 'x': match_whole_lines = true; break;
			case 'c': match_count = true; break;
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
