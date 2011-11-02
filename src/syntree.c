#include <stdlib.h>

#include "syntree.h"

void free_tree(struct syntree* tree) {
	switch (tree->type) {
		case CONCAT:
			free_tree(tree->concat.part1);
			free_tree(tree->concat.part2);
			free(tree);
			break;
		case ALTER:
			free_tree(tree->alter.option1);
			free_tree(tree->alter.option2);
			free(tree);
			break;
		case REPEAT:
			free_tree(tree->repeat.repeated);
			free(tree);
			break;
		case RANGE:
			free(tree);
			break;
	}
}
