#ifndef NEGREP_SYNTREE_H_
#define NEGREP_SYNTREE_H_

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
	int min;
	int max;
	struct syntree* repeated;
};

struct syntree_range {
	char min;
	char max;
};

struct syntree {
	enum syntree_type type;
	union {
		struct syntree_concat concat;
		struct syntree_alter alter;
		struct syntree_repeat repeat;
		struct syntree_range range;
	};
};

void free_tree(struct syntree* tree);

#endif // NEGREP_SYNTREE_H_
