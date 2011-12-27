#ifndef GRAPEFRUIT_SYNTREE_H_
#define GRAPEFRUIT_SYNTREE_H_

#include <stdint.h>

static const int_least16_t UNLIMITED = -1;

enum syntree_type {
	CONCAT, ALTER, REPEAT, RANGE, EMPTY, INPUT_BEGIN, INPUT_END
};

// concatenations of two subexpressions
struct syntree_concat {
	struct syntree* part1;
	struct syntree* part2;
};

// alternation of two options
struct syntree_alter {
	struct syntree* option1;
	struct syntree* option2;
};

// configurable number of repetitions of subexpression
struct syntree_repeat {
	int_least16_t min;
	int_least16_t max;
	struct syntree* repeated;
};

// matches exactly one character between min and max
struct syntree_range {
	uint_least8_t min;
	uint_least8_t max;
};

// syntree contains abstract syntax tree represenation of regular expressions
// Type says which struct in the union is used
struct syntree {
	enum syntree_type type;
	union {
		struct syntree_concat concat;
		struct syntree_alter alter;
		struct syntree_repeat repeat;
		struct syntree_range range;
	} data;
};

// Free all structures used by the tree
void free_tree(struct syntree* tree);

#endif // GRAPEFRUIT_SYNTREE_H_
