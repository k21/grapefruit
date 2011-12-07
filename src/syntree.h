#ifndef GRAPEFRUIT_SYNTREE_H_
#define GRAPEFRUIT_SYNTREE_H_

#include <stdint.h>

enum syntree_type {
	CONCAT, ALTER, REPEAT, RANGE, EMPTY
};

struct syntree_concat {
	struct syntree* part1;
	struct syntree* part2;
};

struct syntree_alter {
	struct syntree* option1;
	struct syntree* option2;
};

struct syntree_repeat {
	int_least16_t min;
	int_least16_t max;
	struct syntree* repeated;
};

struct syntree_range {
	uint_least8_t min;
	uint_least8_t max;
};

struct syntree {
	enum syntree_type type;
	union {
		struct syntree_concat concat;
		struct syntree_alter alter;
		struct syntree_repeat repeat;
		struct syntree_range range;
	} data;
};

void free_tree(struct syntree* tree);

#endif // GRAPEFRUIT_SYNTREE_H_
